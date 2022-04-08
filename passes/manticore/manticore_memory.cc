#include "kernel/modtools.h"
#include "kernel/yosys.h"
#include <algorithm>
#include <list>
#include <memory>
USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

/**
 * @brief
 * write a pass that checks the top module has a single clock and all registers
 * and memories have the same edge polarity
 * //call memory_share before this pass
 * 1. Check that every memwr_v2 is synchronous, i.e., has CLK_EN set to 1
 * 2. Check EN ports of memwr_v2 and create a read-modify-write circuit
 *    if the EN is not the same signal for all bits
 * 3. Ensure PRIORITY_MASK is always 0 (error out otherwise)
 *
 *
 */

struct ModuleTransformer {

	Design *m_design;
	Module *m_module;
	int m_name_index;

	IdString fresh(const std::string &prefix)
	{
		int new_index = m_name_index;
		auto name = m_module->uniquify(stringf("$%s", prefix.c_str()), new_index);
		m_name_index = new_index;
		return name;
	}
	// ModIndex *indexer;
	ModWalker *walker;

	std::vector<Cell *> dead_cells;
	std::vector<Cell *> new_cell;

	// a map from MEMID to memory rd cells
	dict<Const, std::vector<Cell *>> memrds;
	// a map from MEMID to the corresponding memory wr cells
	dict<Const, std::vector<Cell *>> memwrs;
	// a map from write cells to read cells that share the same address
	dict<Cell *, Cell *> write_to_read;
	ModuleTransformer(Design *design, Module *module) : m_design(design), m_module(module), m_name_index(0), walker(nullptr) {}
	~ModuleTransformer()
	{

		if (walker != nullptr) {
			delete walker;
		}
	}

	struct Driver {
		Cell *cell;
		IdString port;
		int offset;
		Driver() : cell(nullptr), port(), offset(0){};
		Driver(Cell *cell, IdString port, int offset) : cell(cell), port(port), offset(offset) {}
		Driver(const Driver &other) : cell(other.cell), port(other.port), offset(other.offset) {}
		Driver &operator=(const Driver &other)
		{
			this->cell = other.cell;
			this->port = other.port;
			this->offset = other.offset;
			return *this;
		}
		inline static Driver from(const ModWalker::PortBit &other) { return Driver(other.cell, other.port, other.offset); }
		inline static Driver None() { return Driver(); }
		bool empty() const { return cell == nullptr; }
		bool nonEmpty() const { return cell != nullptr; }
	};


	Driver getDriver(const SigBit &bit) const
	{

		if (walker->signal_drivers.count(bit)) {
			auto drivers = walker->signal_drivers.at(bit);
			log_assert(drivers.size() == 1);
			return Driver::from(drivers.pop());
		} else {
			return Driver::None();
		}
	}

	std::vector<SigBit> walkWriteEnableConditions(const SigBit &enable) const
	{

		SigBit current_bit = walker->sigmap(enable);

		std::vector<SigBit> mux_conditions;

		// helper closure to get the PortBit of a SigBit, note that the
		// lambda returns a pool of PortBits but we always expect the
		// pool to be either empty or a singleton

		auto current_driver = getDriver(current_bit);
		if (current_driver.empty()) {
			// there is no driver, i.e., the enable signal is an input wire?
			log_debug("Found full bit enable\n");
			log_assert(current_bit.is_wire() && current_bit.wire->port_input);
			mux_conditions.push_back(current_bit);
			return mux_conditions;
		}

		do {
			// we only expect a single driver for each bit
			log_assert(current_driver.nonEmpty());
			log_debug("Found driver of type %s and name %s\n", RTLIL::id2cstr(current_driver.cell->type),
				  RTLIL::id2cstr(current_driver.cell->name));
			if (current_driver.cell->type == ID($mux)) {
				// I expect a cascade of MUX nodes with select bits indicating
				// write enable conditions with input A tied to zero and input
				// B tied to 1 or a bit that is connected to another mux
				// the root case the latter

				mux_conditions.push_back(current_driver.cell->getPort(ID::S).as_bit());
				auto port_A = current_driver.cell->getPort(ID::A);
				log_assert(port_A.size() == 1);
				if (port_A.is_fully_ones()) {
					log_error("Expected 0 on port A of cell %s\n", RTLIL::id2cstr(current_driver.cell->name));
				}
				auto port_B = current_driver.cell->getPort(ID::B);
				auto port_B_bit = port_B.as_bit();
				if (port_B_bit.is_wire()) {
					current_bit = port_B_bit;
					current_driver = getDriver(current_bit);
				} else {
					// we have reached the end of the search because a Mux with
					// constant inputs 0 and 1 has been detected. The conjunction of
					// condition bits we have collected on the way now serve
					// as the "inline" enable bit for the write
					log_assert(port_B.is_fully_ones());
					current_driver = Driver::None();
				}

			} else {
				log_error("Unexpected cell %s of type %s in write enable path\n", RTLIL::id2cstr(current_driver.cell->name),
					  RTLIL::id2cstr(current_driver.cell->type));
				current_driver = Driver::None();
			}

		} while (current_driver.nonEmpty());
		log_debug("Found %lu wen conditions\n", mux_conditions.size());
		return mux_conditions;
	}

	/**
	 * @brief Walk along the write-enable conditions over MUX nodes and
	 * get the original SigBit that can be used as the write data bit
	 *
	 * @param conditions
	 * @param data_bit
	 * @return SigBit
	 */
	SigBit walkWriteDataAlongWriteEnableConditions(const std::vector<SigBit> &conditions, const SigBit &data_bit) const
	{

		log_assert(data_bit.is_wire());

		std::list<SigBit> condlist(conditions.begin(), conditions.end());

		SigBit current_data_bit = data_bit;
		Driver current_driver = getDriver(current_data_bit);
		if (conditions.empty()) {
			// we expect the bits to either directly come from a connection
			// walker
			log_assert(current_driver.empty());
			return walker->sigmap(data_bit);
		}
		log_assert(current_driver.nonEmpty());

		while (current_driver.nonEmpty()) {
			if (current_driver.cell->type == ID($mux)) {
				auto port_S = current_driver.cell->getPort(ID::S);
				if (walker->sigmap(port_S.as_bit()) == walker->sigmap(condlist.front())) {
					condlist.pop_front();
					auto port_B = current_driver.cell->getPort(ID::B);
					current_data_bit = port_B[current_driver.offset];
					if (!condlist.empty()) {
						current_driver = getDriver(current_data_bit);
					} else {
						current_driver = Driver::None();
					}
				} else {
					// conditions match don't match
					log_error("Unexpected condition on memwr_v2 DATA in cell %s path!",
						  RTLIL::id2cstr(current_driver.cell->name));
					current_driver = Driver::None();
				}
			} else {
				log_error("Expected MUX on the DATA path of memwr_v2 starting from %s\n", RTLIL::id2cstr(data_bit.wire->name));
				current_driver = Driver::None();
			}
		}

		// the current_data_bits vector contains the "root" data vetors
		// note that the number of bits in t
		return current_data_bit;
	}
	/**
	 * @brief Checks whether all the bits in the SigSpec are the same
	 *
	 * @param sig
	 * @return true
	 * @return false
	 */
	bool isFullBitRepeat(const SigSpec &sig) const
	{

		auto bits = sig.to_sigbit_vector();

		// make sure nobody calls this function with and empty SigSpec if
		// that's even possible
		log_assert(!bits.empty());

		auto bit0 = bits.front();

		return std::all_of(bits.begin() + 1, bits.end(), [&](const SigBit &b) { return walker->sigmap(bit0) == walker->sigmap(b); });
	}

	/**
	 * @brief Disaggregate a SigSpec into repeated bits, for instance the
	 * SigSpec {b0, b0, b1, b1, b1, b2} will be returned as a vector whose elements
	 * are { (b0, 2, 0), (b1, 3, 2), (b2, 1, 5) }
	 * The \c sig argument should not be empty
	 * @param sig
	 * @return std::vector<std::tuple<SigBit, int, int>>
	 */
	std::vector<std::tuple<SigBit, int, int>> disaggregateRepeatedBits(const SigSpec &sig) const
	{

		auto bits = sig.to_sigbit_vector();

		log_assert(!bits.empty());

		struct Builder {
			SigBit current_bit;
			int width;
			int pos;
			std::vector<std::tuple<SigBit, int, int>> res;
			Builder(const SigBit &first) : current_bit(first), width(0), pos(0) {}
			void consume(const SigBit &b)
			{
				if (current_bit == b) {
					width++;
				} else {
					res.emplace_back(current_bit, width, pos);
					pos += width;
					width = 1;
					current_bit = b;
				}
			}
			std::vector<std::tuple<SigBit, int, int>> build()
			{
				if (width) {
					res.emplace_back(current_bit, width, pos);
					return res;
				} else {
					std::vector<std::tuple<SigBit, int, int>> empty;
					return empty;
				}
			}
		};

		Builder builder(walker->sigmap(bits.front()));
		for (const auto &b : bits) {
			builder.consume(walker->sigmap(b));
		}

		return builder.build();
	}

	SigSpec createWriteDataSignal(const SigSpec &old_value, const SigSpec &new_value, const std::vector<SigBit> &conditions)
	{
		log_assert(old_value.size() == new_value.size());
		if (conditions.empty()) {
			return new_value;
		} else {
			auto wen = createAndTree(SigSpec(conditions.front()), conditions.cbegin() + 1, conditions.cend());
			auto res_wire = m_module->addWire(fresh("wen_data"), new_value.size());
			auto res_sig = SigSpec(res_wire);
			m_module->addMux(fresh("wen_dpath_mux"), old_value, new_value, wen, res_sig);
			return res_sig;
		}
	}
	template <typename ConstIter> SigSpec createAndTree(const SigSpec &in, const ConstIter beg, const ConstIter end)
	{
		if (beg == end) {
			return in;
		} else {

			const SigSpec other = *beg;
			auto res_wire = m_module->addWire(fresh("wen_wire_and_tree"), 1);
			auto next_in = SigSpec(res_wire);
			m_module->addAnd(fresh("wen_and_tree"), in, other, next_in, false);

			return createAndTree(next_in, beg + 1, end);
		}
	}
	/**
	 * @brief Transforms memory write port
	 * This method handles two cases with a memory write port
	 * 1. A memory write port with identical repeated bits on the EN port:
	 * 		This indicates a conditional, yet full width memory write. We can
	 *      slightly optimize the data path of this write by walking along the
	 *      MUX tree that makes up the write EN and collecting the select bits
	 *      of these muxes. Then we use the collected conditions to walk the
	 *      DATA signal back to the bits that make them, removing the redundant
	 *      multiplexer on the way and directly connecting the bits to the DATA
	 *      port.
	 * 2. A memory write port with multiple repeated bits sequences, i.e.,
	 * 		EN <- b0, b0, b0, b1, b1, b1, 0, 0, 1, 1, 1, 1
	 * 		In this case for each sequence of bits, we perform the same operation
	 *      as in case 1, that is, we walk the conditions and then walk the data
	 *      path to collect the bits that are supposed to be written when the
	 *		EN bit is valid. We then use these conditions to multiplex  new values
	 *      (resulting from our walk) and old values (getting it from a memrd_v2 cell
	 *      sharing the address as the current memwr_v2 cell). Then we set the
	 *      EN signal to all ones. By doing this, we essentially create a
	 * 		a read-modify-write sequence and remove the write-enable (setting to always true).
	 *      Note that without doing so, we will not be able to generate code
	 *      for Manticore-compiler since in the assembly code there is only a
	 *      a word-enable bit, whereas Yosys exposes bit-enable bits! This streamlines
	 *      code generation since any memwr_v2 cell is now guaranteed to have the
	 *      the first EN port bit repeated to the rest of them so the code generator
	 *      code simply take EN bit 0 as the STORE predicate.
	 * @param cell a $memwr_v2 cell
	 */
	void transformMemoryWrite(Cell *cell)
	{

		// allocate a walker if one does not exist
		if (walker == nullptr) {
			walker = new ModWalker(m_design, m_module);
		}
		log_assert(cell->type == ID($memwr_v2));
		auto mid = cell->getParam(ID::MEMID);
		auto en_port = cell->getPort(ID::EN);

		// analyze the enable bit
		auto en_sig = cell->getPort(ID::EN);
		if (en_sig.is_fully_zero()) {
			log_warning("Unused memory write node %s\n", RTLIL::id2cstr(cell->name));
		} else if (!en_sig.is_fully_ones() && isFullBitRepeat(en_sig)) {
			// memwr performs a full write to the memory, we can optimize this
			// write by removing MUXes that are driven by the write enable condition
			auto conditions = walkWriteEnableConditions(en_sig[0]);
			log_assert(conditions.empty() == false);
			auto direct_data_sig = SigSpec();
			for (const auto &wbit : cell->getPort(ID::DATA)) {
				auto b = walkWriteDataAlongWriteEnableConditions(conditions, wbit);
				direct_data_sig.append(b);
			}
			cell->setPort(ID::DATA, direct_data_sig);
			auto wen = createAndTree(SigSpec(conditions.front()), conditions.cbegin() + 1, conditions.cend());
			std::vector<SigBit> wen_sig(cell->getParam(ID::WIDTH).as_int());
			std::fill_n(wen_sig.begin(), wen_sig.size(), wen[0]);
			cell->setPort(ID::EN, SigSpec(wen_sig));

		} else if (!en_sig.is_fully_ones() && !isFullBitRepeat(en_sig)) {
			// the enable signal consists of multiple different bits
			// we need to isolate repeated bit patterns
			auto parts = disaggregateRepeatedBits(en_sig);

			auto getBit = [](const std::tuple<SigBit, int, int> b) -> SigBit { return std::get<0>(b); };
			auto isWire = [&getBit](const std::tuple<SigBit, int, int> b) -> bool { return getBit(b).is_wire(); };
			auto isOne = [&getBit](const std::tuple<SigBit, int, int> b) -> bool {
				return !getBit(b).is_wire() && getBit(b).data == State::S1;
			};
			auto getWidth = [](const std::tuple<SigBit, int, int> b) -> int { return std::get<1>(b); };
			auto getOffset = [](const std::tuple<SigBit, int, int> b) -> int { return std::get<2>(b); };

			// for every contiguous set of enable bits, traverse the netlist to
			// collect the conditions that make this bit. Then using these conditions,
			// traverse the netlist starting from the DATA port to extract the
			// chunk/bits of non-zero data that participate in the partial write
			// operation. Then create memrd_v2 node that takes the same address
			// as the current memwr_v2 node. The DATA port of this memory read
			// is now going to be used to create a read-modify-write circuit.
			// This way we get rid of redundant MUX nodes that are in the way
			// of the DATA port of memwr_v2 nodes (with conditions being identical
			// to the EN condition) and finally create a memwr_v2 node with
			// EN nodes set to all ones.

			/**
			 * @brief
			 * 	Get the current memory data bits read from a memrd_v2 cell
			 */
			auto getOldBits = [this, cell, &getWidth, &getOffset](const std::tuple<SigBit, int, int> b) -> SigSpec {
				Cell *rd_cell;
				try {
					rd_cell = write_to_read.at(cell);
				} catch (const std::exception &e) {
					log_error("Could not find read cell for write cell %s\n", RTLIL::id2cstr(cell->name));
				}
				auto read_bits = rd_cell->getPort(ID::DATA).extract(getOffset(b), getWidth(b));
				return read_bits;
			};
			SigSpec full_resolved_write_data_bits;
			for (const auto &en_bit : parts) {
				if (isWire(en_bit)) {
					log_debug("Found cell %s with %d-bit partial write from position %d\n", RTLIL::id2cstr(cell->name),
						  getWidth(en_bit), getOffset(en_bit));
					// now we need to trace back the EN bit back to
					// the cells that make it. The EN is basically
					// generated by a cascade of MUX cell:
					// EN = MUX(c0, 1'0, 1'1 | MUX(c1, 1'0, 1'1 | MUX(....)))
					// we need to collect all the MUX select bits, then we
					// can simplify the EN bit as a conjunction, but more
					// importantly, we can use these conditions to trace back
					// the DATA input to the chunk or sequence of bits that
					// actually matter for the write (with non-zero EN bits)
					auto conditions = walkWriteEnableConditions(getBit(en_bit));
					log_assert(conditions.empty() == false);

					log_debug("Extracting bits [%d +: %d]\n", getOffset(en_bit), getWidth(en_bit));
					SigSpec used_data_bits = cell->getPort(ID::DATA).extract(getOffset(en_bit), getWidth(en_bit));

					// note that used_data_bits is not necessarily a chunk!

					SigSpec original_bits;
					for (const auto &u : used_data_bits) {
						auto o = walkWriteDataAlongWriteEnableConditions(conditions, u);
						original_bits.append(o);
						log_debug("Found %s[%d] as the write driver of %s in %s[%d]\n", RTLIL::id2cstr(o.wire->name),
							  o.offset, RTLIL::id2cstr(cell->name), RTLIL::id2cstr(u.wire->name), u.offset);
					}
					if (original_bits.is_chunk()) {
						SigChunk write_chunk = original_bits.as_chunk();
						log("Found original wire %s[%d +: %d] starting from %d\n", RTLIL::id2cstr(write_chunk.wire->name),
						    write_chunk.offset, write_chunk.width, getOffset(en_bit));
					}

					auto read_bits = getOldBits(en_bit);
					int offset = 0;
					for (const auto &new_chunk : original_bits.chunks()) {
						log_debug("Creating WDATA mux for chunk %s[%d +: %d]\n", RTLIL::id2cstr(new_chunk.wire->name),
							  new_chunk.offset, new_chunk.width);
						auto resolved_bits =
						  createWriteDataSignal(read_bits.extract(offset, new_chunk.width), SigSpec(new_chunk), conditions);
						offset += new_chunk.width;
						full_resolved_write_data_bits.append(resolved_bits);
					}

				} else if (isOne(en_bit)) {
					// we don't need to find the original bits
					SigSpec write_data = cell->getPort(ID::DATA).extract(getOffset(en_bit), getWidth(en_bit));
					full_resolved_write_data_bits.append(write_data);

				} else {

					// the en bit is zero, so we need to look up the memory read
					// cell that has the same address as this memory write cell
					// and extract the corresponding bits from the read data port
					// signal

					auto read_bits = getOldBits(en_bit);
					full_resolved_write_data_bits.append(read_bits);
				}
			}

			cell->setPort(ID::DATA, full_resolved_write_data_bits);
			cell->setPort(ID::EN, SigSpec(Const(State::S1, cell->getParam(ID::WIDTH).as_int())));
			// if (num_wire_bits.)
		}
		// if (en_port)
	}

	pool<Module *> initialize()
	{

		pool<Module *> module_instances;
		// initialize maps and check for basic stuff
		for (const auto &cell : m_module->cells()) {

			if (cell->type == ID($mem_v2)) {
				log_error("Can not handle $mem_v2! Did you run memory_collect before manticore_memory?");
			} else if (cell->type == ID($memrd)) {
				log_error("Can not handle $memrd cell %s\n", RTLIL::id2cstr(cell->name));
			} else if (cell->type == ID($memrd_v2)) {
				// ensure memory read nodes are always asynchronous
				if (cell->getParam(ID::CLK_ENABLE).as_bool()) {
					log_error("Can not handle synchronous memrd_v2 nodes, someone needs to implement the code to handle it:D");
				}
				if (cell->getParam(ID::TRANSPARENCY_MASK).as_bool()) {
					// I don't know what this parameter actually means
					// perhaps it means the writes directly propagate to the
					// read pots, in that case we can not handle it in Manticore (it's like a latch with EN high
					// therfore, does not fit in cycle-accurate simulation domain)
					log_error("Unhandled $memrd TRANSPARENCY_MASK parameter value in %s\n", RTLIL::id2cstr(cell->name));
				}
				auto mid = cell->getParam(ID::MEMID);
				memrds[mid].push_back(cell);

			} else if (cell->type == ID($memwr)) {
				log_error("Can not handle $memwr cell %s\n", RTLIL::id2cstr(cell->name));
			} else if (cell->type == ID($memwr_v2)) {
				auto is_sync_write = cell->getParam(ID::CLK_ENABLE).as_bool();
				if (!is_sync_write) {
					log_error("Can not handle asynchronous memory writes in %s\n", RTLIL::id2cstr(cell->name));
				}
				auto mid = cell->getParam(ID::MEMID);
				memwrs[mid].push_back(cell);
			} else {
				// if the cell is user module, then keep it to inform
				// the caller about it
				auto other_module = m_design->module(cell->type);
				if (other_module != NULL) {
					module_instances.insert(other_module);
				}
			}
		}
		return module_instances;
	}

	void addReadCells()
	{

		SigMap sigmap(m_module);

		for (const auto &entry : memwrs) {
			auto mid = entry.first;
			for (const auto &wr_cell : entry.second) {
				// look up read cells in to the same memory block
				// and check if there exists a memory read with the
				// same address
				auto waddr = wr_cell->getPort(ID::ADDR);
				auto &rd_cells = memrds[mid];

				const auto found = std::find_if(rd_cells.begin(), rd_cells.end(), [&waddr, &sigmap](const Cell *rdc) {
					auto raddr = rdc->getPort(ID::ADDR);
					return sigmap(raddr) == sigmap(waddr);
				});
				if (found == rd_cells.end()) {

					Cell *new_rd_cell = m_module->addCell(fresh("memrd_rd_mod_wr"), ID($memrd_v2));
					new_rd_cell->setParam(ID::ABITS, wr_cell->getParam(ID::ABITS));
					new_rd_cell->setParam(ID::ARST_VALUE, Const(State::Sx, wr_cell->getParam(ID::WIDTH).as_int()));
					new_rd_cell->setParam(ID::CE_OVER_SRST, Const(State::S0, 1));
					new_rd_cell->setParam(ID::CLK_ENABLE, Const(State::S0, 1));
					new_rd_cell->setParam(ID::CLK_POLARITY, wr_cell->getParam(ID::CLK_POLARITY));
					new_rd_cell->setParam(ID::COLLISION_X_MASK, Const(State::S0, 1));
					new_rd_cell->setParam(ID::INIT_VALUE, Const(State::Sx, wr_cell->getParam(ID::WIDTH).as_int()));
					new_rd_cell->setParam(ID::MEMID, mid);
					new_rd_cell->setParam(ID::SRST_VALUE, Const(State::Sx, wr_cell->getParam(ID::WIDTH).as_int()));
					new_rd_cell->setParam(ID::TRANSPARENCY_MASK, Const(State::S0, 1));
					new_rd_cell->setParam(ID::WIDTH, wr_cell->getParam(ID::WIDTH));

					// connect the address from the write cell to this read cell

					new_rd_cell->setPort(ID::ADDR, waddr);
					new_rd_cell->setPort(ID::ARST, SigSpec(Const(State::S0, 1)));
					new_rd_cell->setPort(ID::CLK, SigSpec(Const(State::Sx, 1)));
					// create a new wire for the write data
					Wire *rdata = m_module->addWire(fresh("data_memrd_rd_mod_wr"), wr_cell->getParam(ID::WIDTH).as_int());
					new_rd_cell->setPort(ID::DATA, SigSpec(rdata));

					new_rd_cell->setPort(ID::EN, SigSpec(Const(State::S1, 1)));
					new_rd_cell->setPort(ID::SRST, SigSpec(Const(State::S0, 1)));

					memrds[mid].push_back(new_rd_cell);
					write_to_read[wr_cell] = new_rd_cell;
					log_debug("Created $memrd_v2 %s cell for $memwr_v2 cell %s\n", RTLIL::id2cstr(new_rd_cell->name),
						  RTLIL::id2cstr(wr_cell->name));
				} else {
					write_to_read[wr_cell] = *found;
				}
			}
		}
	}
	pool<Module *> transform()
	{

		log_assert(m_module != NULL);

		/// do basic checks and create rd/wr memory mappings
		auto module_instances = initialize();
		/// create read cells if a write cell does not have one with the
		// same address signal
		addReadCells();
		// note that we can not iterate over cells because we are going possibly
		// add cells when we visit memwr_v2 cells.

		for (const auto &entry : memwrs) {
			for (const auto &wr_cell : entry.second) {
				transformMemoryWrite(wr_cell);
			}
		}

		return module_instances;
	}
};
struct ManticoreMemoryWorker {

	Design *m_design;

	pool<Module *> modules_to_transform;
	pool<Module *> transformed;

	ManticoreMemoryWorker(Design *design) : m_design(design) {}
	void transform()
	{
		modules_to_transform.insert(m_design->top_module());
		while (!modules_to_transform.empty()) {

			auto current_module = modules_to_transform.pop();
			std::unique_ptr<ModuleTransformer> worker(new ModuleTransformer(m_design, current_module));
			auto pending_modules = worker->transform();
			transformed.insert(current_module);
			for (const auto &n : pending_modules) {
				modules_to_transform.insert(n);
			}
		}
	}
};
struct ManticoreMemory : public Pass {

	ManticoreMemory() : Pass("manticore_memory", "replace bit-wise write strobes with write enables") {}

	void execute(std::vector<std::string>, Design *design) override
	{

		// find all the memwr_v2 nodes that do not have a full EN input
		// and
		log_header(design, "Executing Manticore Memory Normalization and Optimization Pass\n");
		Pass::call(design, "memory_share");
		Pass::call(design, "write_rtlil pre.rtlil");

		std::unique_ptr<ManticoreMemoryWorker> worker(new ManticoreMemoryWorker(design));
		worker->transform();
	}
} ManticoreMemory;

PRIVATE_NAMESPACE_END