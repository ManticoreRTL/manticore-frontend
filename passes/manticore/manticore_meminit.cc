#include "kernel/mem.h"
#include "kernel/yosys.h"
#include <algorithm>
#include <fstream>
#include <queue>
#include <sstream>
#include <vector>
#include <stdlib.h>
USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct ManticoreMemoryInit : public Pass {

	ManticoreMemoryInit() : Pass("manticore_meminit", "Create memory initialization files and remove meminit nodes"){};

	void help() override
	{
		//   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
		log("\n");
		log("    manticore_meminit [PREFIX]\n");
		log("\n");
	}

	void execute(std::vector<std::string> args, Design *design) override
	{

		log_header(design, "Executing Manticore Memory Initialization Pass\n");
		// std::string prefix_path = ".";
		// if (args.size() == 2) {
		// 	prefix_path = args[1];
		// }

		auto MemOrder = [](const Cell *m1, const Cell *m2) -> bool {
			// inverse priority order
			return m1->getParam(ID::PRIORITY).as_int() > m2->getParam(ID::PRIORITY).as_int();
		};

		using PriorityQueue = std::priority_queue<Cell *, std::vector<Cell *>, decltype(MemOrder)>;

		for (auto module : design->selected_modules()) {
			dict<IdString, PriorityQueue *> mem_inits;
			for (const auto &m : module->memories) {
				mem_inits.insert(std::make_pair(m.first, new PriorityQueue(MemOrder)));
			}
			// collect all the init nodes and put them in a priority queue
			for (const auto &cell : module->cells()) {
				if (cell->type == ID($meminit_v2) || cell->type == ID($meminit)) {
					auto mid = IdString(cell->getParam(ID::MEMID).decode_string());
					try {
						auto q = mem_inits.at(mid);
						// log("Found init cell %s\n", cell->name.c_str());
						q->push(cell);
						// log("Size = %lu\n", q->size());
					} catch (const std::exception &e) {
						log_error("Caught exception while creating memory init for cell %s wit mid %s map:\n %s",
							  cell->name.c_str(), mid.c_str(), e.what());
					}
				}
				//  else if (cell->type == ID($meminit)) {
				// 	log_error("Can not support cell %s of type %s\n", cell->name.c_str(), cell->type.c_str());
				// }
			}

			// now we have map from every MEMID to the MEMINIT_V2 cells that initialize it
			// we should now consolidate all the init nodes into a vector of constants

			for (const auto &p : mem_inits) {
				IdString mid = p.first;
				log("Processing memory %s\n", mid.c_str());
				RTLIL::Memory *mem = nullptr;
				try {
					mem = module->memories.at(mid);
				} catch (const std::exception &e) {
					log_error("Could not find memory id %s\n", mid.c_str());
				}

				auto queue = p.second;

				auto constPort = [](const Cell *cell, const IdString &port) -> Const {
					Const res;
					try {
						res = cell->getPort(port).as_const();
					} catch (const std::exception &e) {
						log_error("Caught exception, expected a constant signal on port %s of cell %s: %s", port.c_str(),
							  cell->name.c_str(), e.what());
					}
					return res;
				};

				log("Number of initial memory cells to resolve = %lu\n", queue->size());
				std::vector<Const> resolved(mem->size);
				std::fill(resolved.begin(), resolved.end(), Const(State::S0, mem->width));
				while (!queue->empty()) {

					auto init_cell = queue->top();

					log_assert(init_cell->getParam(ID::WIDTH).as_int() == mem->width);

					Const init_data = constPort(init_cell, ID::DATA);

					int init_addr = constPort(init_cell, ID::ADDR).as_int();

					Const en_mask;
					if (init_cell->hasPort(ID::EN)) {
						en_mask = constPort(init_cell, ID::EN);
					} else {
						// sometimes $meminit_v2 does not have an EN port, which
						// means we have to write the full DATA
						en_mask = Const(State::S1, mem->width);
					}
					Const neg_mask = const_not(en_mask, Const(), false, false, mem->width);
					auto num_words = init_cell->getParam(ID::WORDS).as_int();
					log("Processing cell %s with %d words at address %d\n", init_cell->name.c_str(), num_words, init_addr);
					log_assert(num_words + init_addr - mem->start_offset < mem->size);
					// split the init_data to the num_words, each containing mem->width bits
					for (int offset = 0; offset < num_words; offset++) {
						auto word = init_data.extract(offset * mem->width, mem->width);
						auto masked = const_and(word, en_mask, false, false, mem->width);
						int addr = offset + init_addr - mem->start_offset;
						auto original = resolved[addr];
						auto orig_masked = const_and(original, neg_mask, false, false, mem->width);
						resolved[addr] = const_or(masked, orig_masked, false, false, mem->width);
					}
					log_assert(init_data.size() % mem->width == 0);

					// log("Removing %s\n", init_cell->name.c_str());
					module->remove(init_cell);
					queue->pop();
				}
				delete queue;

				// now that we resolved the memory initialization based on the priority
				// we can write it to a file

				auto directory = realpath(".", NULL);
				if (directory == NULL) {
					log_error("Could not resolve path %s.%s.hex\n", log_id(module), log_id(mid));
				}
				auto file_path = stringf("%s/%s.%s.hex", directory, log_id(module), log_id(mid));
				std::ofstream ofs(file_path, std::ios::out);

				if (!ofs.is_open()) {
					log_error("Could not open file %s\n", file_path.c_str());
				}
				auto constToHexString = [](const Const &value) -> std::string {
					int num_parts = (value.size() - 1) / 32 + 1;
					std::stringstream builder;
					for (int i = num_parts - 1; i >= 0; i--) {

						int int_value = value.extract(i * 32, 32).as_int();
						builder << stringf("%08x", int_value);
					}
					return builder.str();
				};
				for (const auto &word : resolved) {

					ofs << constToHexString(word) << std::endl;
				}
				ofs.close();

				module->memories.at(mid)->set_string_attribute(ID::MEM_INIT_FILE, file_path);
			}
		}
	}

} ManticoreMemoryInit;

PRIVATE_NAMESPACE_END