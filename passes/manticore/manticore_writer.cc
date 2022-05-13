#include "kernel/modtools.h"
#include "kernel/yosys.h"
#include <algorithm>
#include <fstream>
#include <functional>
#include <queue>
USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN
struct NameBuilder {
	const std::string prefix;
	Module *mod;
	int index;
	inline std::string next()
	{
		auto n = stringf("%%%s_%d", prefix.c_str(), index);
		index++;
		log("New name : %s\n", n.c_str());
		return n;
	}
	NameBuilder(const std::string &prefix, Module *m) : prefix(prefix), mod(m), index(0) {}
};
struct DefBuilder {
	std::stringstream defs;
	const std::string type;
	NameBuilder namer;
	dict<Wire *, std::string> wires;
	DefBuilder(const std::string &t, const std::string &p, Module *m) : type(t), namer(p, m) {}
	inline std::string mk(const Const &value)
	{

		auto name = namer.next();
		defs << type << " " << name << " " << value.size() << " 0b" << value.as_string() << std::endl;
		return name;
	}

	inline std::string mk(int width)
	{
		auto name = namer.next();
		defs << type << " " << name << " " << width << std::endl;
		return name;
	}
	inline std::string mk(bool value)
	{
		auto name = namer.next();
		defs << type << " " << name << " 1 " << (value ? "0b1" : "0b0") << std::endl;
		return name;
	}
	inline std::string get(Wire *w)
	{

		if (!wires.count(w)) {
			defs << type << " " << w->name.str() << " " << w->width << std::endl;
			wires.insert(std::make_pair(w, w->name.str()));
		}
		return wires[w];
	}
};

struct InstructionBuilder {

	// enum AluOp
	std::stringstream builder;
	InstructionBuilder() {}

	inline void LOAD(const std::string &rd, const std::string &base) { builder << "LLD " << rd << ", " << base << "[0];" << std::endl; }
	inline void STORE(const std::string &rs, const std::string &base) { builder << "LST " << rs << ", " << base << "[0];" << std::endl; }
	inline void PADZERO(const std::string &rd, const std::string &rs, int width)
	{
		builder << "PADZERO " << rd << ", " << rs << ", " << width << ";" <<std::endl;
	}
	inline void MOV(const std::string &rd, const std::string &rs) { builder << "MOV " << rd << ", " << rs <<  ";" << std::endl; }
	inline void SLICE(const std::string &rd, const std::string &rs, int offset, int len)
	{
		builder << "SLICE " << rd << ", " << rs << "[" << offset + len - 1 << ", " << offset << "];" << std::endl;
	}

	inline void CONCAT(const std::string &rd, const std::string &rs_low, const std::string &rs_high, int offset)
	{
		builder << "CONCAT " << rd << ", " << rs_low << ", " << rs_high << "[" << offset << "];" << std::endl;
	}
	inline void PARMUX(const std::string &rd, const std::vector<std::string> choices, const std::string &def_case,
			   const std::vector<std::string> &conds)
	{
		log_assert(choices.size() == conds.size());
		builder << "PARMUX " << rd;
		for (int i = 0; i < GetSize(choices); i++) {
			builder << choices[i] << "[" << conds[i] << "], ";
		}
		builder << def_case << ";" << std::endl;
	}
#define BINOP_DEF(op)                                                                                                                                \
	inline void op(const std::string &rd, const std::string &rs1, const std::string &rs2)                                                        \
	{                                                                                                                                            \
		builder << #op << " " << rd << ", " << rs1 << ", " << rs2 << ";" << std::endl;                                                       \
	}
	BINOP_DEF(ADD) // First ALU op
	BINOP_DEF(SUB)
	BINOP_DEF(MUL)
	BINOP_DEF(AND)
	BINOP_DEF(OR)
	BINOP_DEF(XOR)
	BINOP_DEF(SLL)
	BINOP_DEF(SRL)
	BINOP_DEF(SRA)
	BINOP_DEF(SEQ)
	BINOP_DEF(SLTS) // Last ALU op

#undef BINOP_DEF

	inline void comment(const std::string &msg) { builder << "// " << msg << std::endl; }
};
struct ManticoreAssemblyWorker {

	const std::string filename;
	Design *design;
	Module *mod;
	SigMap sigmap;
	// helper object to handle constant
	DefBuilder def_const;
	// helper object to handle wires and temp wires
	DefBuilder def_wire;

	//
	DefBuilder def_inp;
	DefBuilder def_out;

	// helper class to create instructions
	InstructionBuilder instr;


	// a dict of signals to their assignmed assembly names
	dict<SigSpec, std::string> sig_names;


	struct SubAssign {
		// a helper class representing
		// wire [from_width - 1 : 0] from_name;
		// connect to[to.offset +: to.width]  from_name[offset +: to.width];

		SigChunk to; // the chunk to assign to
		std::string from_name; // name of the signal on rhs
		int from_offset; // offset of the rhs signal
		int from_width; // width of the rhs signal
		SubAssign(const SigChunk& chunk, const std::string& from_name, int from_offset, int from_width) :
			to(chunk), from_name(from_name), from_offset(from_offset), from_width(from_width) {}

	};
	// a dict of wires with subword assignment
	std::map<Wire*, std::vector<SubAssign>> sub_assigns;

	// dict<IdString, std::function<void(ManticoreAssemblyWorkerCell *)>> converters;

	ManticoreAssemblyWorker(const std::string &filename, Design *design)
	    : filename(filename), design(design), mod(design->top_module()), sigmap(design->top_module()),
	      def_const(".const", "c", design->top_module()), def_wire(".wire", "t", design->top_module()),
	      def_inp(".input", "i", design->top_module()), def_out(".output", "o", design->top_module())
	{
	}

	// recursively create CONCAT instructions from chunks
	inline const std::string convertChunksRecurse__(const std::vector<SigChunk> &chunks, const std::vector<SigChunk>::iterator beg,
							std::pair<std::string, int> prev)
	{

		log_assert(beg != chunks.begin());

		if (beg == chunks.end()) {
			log("Finished\n");
			return prev.first;
		} else {
			auto current = *beg;
			auto current_width = current.width;
			auto accum_width = prev.second;
			auto current_name = convert(current);
			auto next_width = accum_width + current_width;
			auto next_name = def_wire.mk(next_width);
			instr.CONCAT(next_name, prev.first, current_name, accum_width);
			auto next = std::make_pair(next_name, next_width);
			return convertChunksRecurse__(chunks, beg + 1, next);
		}
	}

	// get the Manticore name of a chunk, this may involve creating a SLICE instruction
	inline const std::string convert(const SigChunk &chunk)
	{
		auto found = sig_names.find(SigSpec(chunk));

		if (found != sig_names.end()) {
			return found->second;
		}
		if (chunk.is_wire()) {
			if (chunk.wire->width == chunk.width) {
				log("full chunk %s\n", RTLIL::id2cstr(chunk.wire->name));
				auto n = def_wire.get(chunk.wire);
				sig_names.insert(std::make_pair(SigSpec(chunk), n));
				return def_wire.get(chunk.wire);
			} else {
				log("sliced chunk %s[%d +: %d]\n",
					RTLIL::id2cstr(chunk.wire->name), chunk.offset, chunk.width);
				auto chunk_name = def_wire.mk(chunk.width);
				instr.SLICE(chunk_name, def_wire.get(chunk.wire), chunk.offset, chunk.width);
				sig_names.insert(std::make_pair(SigSpec(chunk), chunk_name));
				return chunk_name;
			}
		} else {
			return def_const.mk(Const(chunk.data));
		}
	}

	// get the manticore name of a SigSpec, may involve creating CONCAT and SLICE
	// instruction
	const std::string convert(const SigSpec &sig)
	{
		auto found = sig_names.find(sig);
		if (found != sig_names.end()) {
			return (*found).second;
		}
		// else do the conversion
		if (sig.is_wire()) {
			auto n = def_wire.get(sig.as_wire());
			sig_names.insert(
				std::make_pair(
					sig,
					n
				)
			);
			return n;
		} else {

			auto chunks = sig.chunks();
			log_assert(chunks.size() != 0);
			log("Chunk size %d\n", GetSize(chunks));

			auto n = convertChunksRecurse__(chunks, chunks.begin() + 1, std::make_pair(convert(chunks.front()), chunks.front().width));
			sig_names.insert(
				std::make_pair(
					sig,
					n
				)
			);
			return n;
		}
	}
	void keepSubAssign(const SubAssign& assign) {

		auto found = sub_assigns.find(assign.to.wire);
		if (found == sub_assigns.end()) {

			sub_assigns.insert(
				std::make_pair(
					assign.to.wire,
					std::vector<SubAssign> { assign }
				)
			);
		} else {
			sub_assigns[assign.to.wire].push_back(assign);
		}

	}
	const std::string outConvert(const SigSpec& sig) {
		if (sig.is_wire()) {
			return convert(sig);
		} else {
			auto temp_name = def_wire.mk(sig.size());
			int offset = 0;
			for(const auto& chunk : sig.chunks()) {

				log_assert(chunk.is_wire()); // chunk can not be a constant,
				// since we are assigning to it!
				log("logging subword-assign %s[%d +: %d] <- %s[%d +: _] (%d)\n",
					RTLIL::id2cstr(chunk.wire->name),
					chunk.offset, chunk.width,
					temp_name.c_str(), offset, sig.size()
				);
				keepSubAssign(
					SubAssign(
						chunk,
						temp_name,
						offset,
						sig.size()
					)
				);
				offset += chunk.width;
			}

			return temp_name;
		}
	}


	struct CellSanityChecker {

		Cell *cell;
		CellSanityChecker(Cell *cell) : cell(cell) {}

		/**
		 * @brief Checks whether both operands have the same width
		 *
		 * @return true
		 * @return false
		 */
		inline bool equalWidthPorts() const { return cell->getParam(ID::A_WIDTH) == cell->getParam(ID::B_WIDTH); }
		/**
		 * @brief Checks whether both operands have the same sign
		 *
		 * @return true
		 * @return false
		 */
		inline bool equalSign() const { return cell->getParam(ID::A_SIGNED) == cell->getParam(ID::B_SIGNED); }

		/**
		 * @brief are both operands unsigned?
		 *
		 * @return true
		 * @return false
		 */
		inline bool bothUnsigned() const
		{
			return (cell->getParam(ID::A_SIGNED).as_bool() == false) && (cell->getParam(ID::B_SIGNED) == false);
		}

		/**
		 * @brief A signed operand A implies that both operands should have
		 * the same width
		 *
		 * @return true  if A is signed and A_WIDTH == B_WIDTH or A is not signed
		 * @return false  if A is signed and A_WIDTH != B_WIDTH
		 */

		inline bool signedOperandsRequireEqualWidth() const
		{
			bool a_signed = cell->getParam(ID::A_SIGNED).as_bool();
			if (a_signed) {
				return equalWidthPorts();
			} else {
				return true;
			}
		}

		inline bool outputWidthIsMax() const {
			auto w = std::max(
				cell->getParam(ID::A_WIDTH).as_int(),
				cell->getParam(ID::B_WIDTH).as_int()
			);
			return w == cell->getParam(ID::Y_WIDTH).as_int();
		}

		inline bool boolOutput() const {
			return cell->getParam(ID::Y_WIDTH).as_int() == 1;
		}

		inline void assertEqualWidth() const
		{
			if (!equalWidthPorts()) {
				log_error("Expected equal width operands in cell %s of type %s\n", RTLIL::id2cstr(cell->name),
					  RTLIL::id2cstr(cell->type));
			}
		}
		inline void assertEqualSigns() const
		{
			if (!equalSign()) {
				log_error("Expected equal signs for operands in cell %s of type %s\n", RTLIL::id2cstr(cell->name),
					  RTLIL::id2cstr(cell->type));
			}
		}
		inline void assertSignedOperandsRequireEqualWidth() const
		{
			if (!signedOperandsRequireEqualWidth()) {
				log_error("Expected signed operand A to imply equal width for both ports in cell %s of type %s\n",
					  RTLIL::id2cstr(cell->name), RTLIL::id2cstr(cell->type));
			}
		}

		inline void assertBothUnsigned() const
		{
			if (!bothUnsigned()) {
				log_error("Expected unsigned operands in cell %s of type %s\n", RTLIL::id2cstr(cell->name),
					  RTLIL::id2cstr(cell->type));
			}
		}

		inline void assertAll() const
		{
			assertEqualWidth();
			assertEqualWidth();
			assertSignedOperandsRequireEqualWidth();
		}

		inline void assertBoolOutput() const {
			if (!boolOutput()) {
				log_error("Expected boolean output in cell %s of type %s\n", RTLIL::id2cstr(cell->name),
					  RTLIL::id2cstr(cell->type));
			}
		}
		inline void assertOutputIsMaxWidth() const {
			if (!outputWidthIsMax()) {
				log_error("Expected maximum input width on output in cell %s of type %s\n",
					RTLIL::id2cstr(cell->name),
					  RTLIL::id2cstr(cell->type));
			}
		}
	};


	std::string padConvert(const SigSpec &sig, int width)
	{
		log_assert(sig.size() <= width);
		if (sig.size() < width) {
			auto padded = def_wire.mk(width);
			instr.PADZERO(padded, convert(sig), width);
			return padded;
		} else {
			return convert(sig);
		}
	}

	// converts a cell (most likely a $eq or $neq) into SEQ
	inline void mkSeq(Cell *cell, const std::string &res)
	{

		auto checker = CellSanityChecker(cell);
		checker.assertEqualSigns();
		checker.assertSignedOperandsRequireEqualWidth();
		auto y_name = res;
		if (checker.equalWidthPorts()) {
			// note that the A and B might be negative, but that is OK
			instr.SEQ(y_name, convert(cell->getPort(ID::A)), convert(cell->getPort(ID::B)));
		} else {
			// ensure both operands are unsigned
			checker.assertBothUnsigned();
			auto max_width = std::max(cell->getParam(ID::A_WIDTH).as_int(), cell->getParam(ID::B_WIDTH).as_int());
			instr.SEQ(y_name, padConvert(cell->getPort(ID::A), max_width), padConvert(cell->getPort(ID::B), max_width));
		}
	}

	inline void mkSlts(const std::string &res, int a_width, int b_width, bool a_signed, bool b_signed, const SigSpec &a, const SigSpec &b, Cell* cell)
	{

		// We implement less than as a subtraction and a test because we can
		// later use the same logic when lowering MASM to 16-bit operands.
		// Lowering SUB is trivial, and the SLTS does not change (it is already
		// single-bit < 16).
		// We use SLTS instead of SLTU because we have to check if the result is
		// negative.
		// IMPORTANT: We need to sign zero extend the operands if they are not signed.
		// 1. Suppose 32 bit unsigned numbers with a = (1 << 31) + 1 and b = 1, obviously,
		// a - b = 1 << 31 is a negative number from the perspective of SLTS but
		// here infact we would like to have a < b to be false, but if we simply
		// subtract and then check for a negative result, we will have the wrong
		// conclusion. Therefore, if we look at the operands as 32-bit signed numbers
		// with the sign bit hardcoded to zero, we can use SLTS and conclude
		// a < b to false.
		// 2. Suppose 32 bit signed numbers. Here because the sign is already encoded
		// in the operands, we need not to zero/sign extend them.
		auto mkZero = [this](int width) { return def_const.mk(Const(State::S0, width)); };
		if (a_signed != b_signed) {
			log_error("Expected equal signs in cell %s of type %s\n", RTLIL::id2cstr(cell->name), RTLIL::id2cstr(cell->type));
		}
		if (a_signed) {
			if (a_width != b_width) {
				log_error("Expected equal width in cell %s of type %s\n", RTLIL::id2cstr(cell->name), RTLIL::id2cstr(cell->type));
			}

			// easier case, we do not need to sign extend the operands
			int width = a_width;
			auto temp_name = def_wire.mk(width);
			instr.SUB(temp_name, convert(a), convert(b));
			instr.SLTS(res, temp_name, mkZero(width));
		} else {
			// operands are unsigned, therefore we need to zero extends them to
			// ensure the sign bit is always zero
			int extended_width = std::max(a_width, b_width) + 1;
			auto temp_name = def_wire.mk(extended_width);
			instr.SUB(temp_name, padConvert(a, extended_width), padConvert(b, extended_width));
			instr.SLTS(res, temp_name, mkZero(extended_width));
		}
	}

	// convert a cell into instructions
	void convert(Cell *cell)
	{

		auto checker = CellSanityChecker(cell);
		auto maxWidth = [cell]() {
			return std::max(
				cell->getParam(ID::A_WIDTH).as_int(),
				cell->getParam(ID::B_WIDTH).as_int()
			);
		};
		if (cell->type == ID($add)) {
			checker.assertSignedOperandsRequireEqualWidth();
			// checker.assertEqualWidth(); need not to hold if one operand is a constant
			checker.assertEqualSigns();
			instr.ADD(convert(cell->getPort(ID::Y)), convert(cell->getPort(ID::A)), convert(cell->getPort(ID::B)));
		} else if (cell->type == ID($sub)) {
			checker.assertEqualWidth();
			checker.assertEqualSigns();

			instr.SUB(outConvert(cell->getPort(ID::Y)), convert(cell->getPort(ID::A)), convert(cell->getPort(ID::B)));
		} else if (cell->type == ID($eq)) {
			mkSeq(cell, convert(cell->getPort(ID::Y)));
		} else if (cell->type == ID($ne)) {
			auto temp_name = def_wire.mk(1);
			mkSeq(cell, temp_name);
			instr.XOR(convert(cell->getPort(ID::Y)), temp_name, def_const.mk(false));
			// same as $eq, except with negate the results (XOR with false is basically not)
		} else if (cell->type == ID($lt)) {

			mkSlts(
				outConvert(cell->getPort(ID::Y)),
				cell->getParam(ID::A_WIDTH).as_int(),
				cell->getParam(ID::B_WIDTH).as_int(),
				cell->getParam(ID::A_SIGNED).as_bool(),
				cell->getParam(ID::B_SIGNED).as_bool(),
				cell->getPort(ID::A),
				cell->getPort(ID::B),
				cell
			);
		} else if (cell->type == ID($gt)) {
			// just like $lt, with operands swapped
			mkSlts(
				outConvert(cell->getPort(ID::Y)),
				cell->getParam(ID::B_WIDTH).as_int(),
				cell->getParam(ID::A_WIDTH).as_int(),
				cell->getParam(ID::B_SIGNED).as_bool(),
				cell->getParam(ID::A_SIGNED).as_bool(),
				cell->getPort(ID::B),
				cell->getPort(ID::A),
				cell
			);
		} else if (cell->type == ID($le)) {

			// like $le but ord with $eq
			auto less = def_wire.mk(1);
			auto equal = def_wire.mk(1);
			mkSeq(cell, equal);
			mkSlts(
				less,
				cell->getParam(ID::A_WIDTH).as_int(),
				cell->getParam(ID::B_WIDTH).as_int(),
				cell->getParam(ID::A_SIGNED).as_bool(),
				cell->getParam(ID::B_SIGNED).as_bool(),
				cell->getPort(ID::A),
				cell->getPort(ID::B),
				cell
			);
			checker.assertBoolOutput();
			instr.OR(outConvert(cell->getPort(ID::Y)), less, equal);

		} else if (cell->type == ID($ge)) {
			// like $gt ored with $eq
			auto greater = def_wire.mk(1);
			auto equal = def_wire.mk(1);
			mkSlts(
				greater,
				cell->getParam(ID::B_WIDTH).as_int(),
				cell->getParam(ID::A_WIDTH).as_int(),
				cell->getParam(ID::B_SIGNED).as_bool(),
				cell->getParam(ID::A_SIGNED).as_bool(),
				cell->getPort(ID::B),
				cell->getPort(ID::A),
				cell
			);
			mkSeq( cell, equal);
			checker.assertBoolOutput();
			instr.OR(outConvert(cell->getPort(ID::Y)), greater, equal);
		} else if (cell->type == ID($or)) {
			checker.assertOutputIsMaxWidth();
			auto w = maxWidth();
			instr.OR(
				outConvert(cell->getPort(ID::Y)),
				padConvert(cell->getPort(ID::A), maxWidth()),
				padConvert(cell->getPort(ID::B), maxWidth())
			);
		} else if (cell->type == ID($and)) {
			checker.assertOutputIsMaxWidth();
			auto w = maxWidth();
			instr.AND(
				outConvert(cell->getPort(ID::Y)),
				padConvert(cell->getPort(ID::A), maxWidth()),
				padConvert(cell->getPort(ID::B), maxWidth())
			);
		} else if (cell->type == ID($xor)) {
			checker.assertOutputIsMaxWidth();
			auto w = maxWidth();
			instr.XOR(
				outConvert(cell->getPort(ID::Y)),
				padConvert(cell->getPort(ID::A), maxWidth()),
				padConvert(cell->getPort(ID::B), maxWidth())
			);
		} else if (cell->type == ID($not)) {
			auto width = cell->getParam(ID::A_WIDTH);
			log_assert(width == cell->getParam(ID::Y_WIDTH).as_int());
			// instr.XOR(
			// 	outConvert(cell->getPort(ID::Y)),
			// 	convert(cell->getPort(ID::A)),
			// 	// def_const.mk(Const(State::S1, width))
			// );
		}
	}
	void generate()
	{

		for (const auto &cell : mod->cells()) {
			try {
				convert(cell);
			} catch (const std::exception &e) {
				log_error("Do not know how translate cell %s of type %s\n", RTLIL::id2cstr(cell->name), RTLIL::id2cstr(cell->type));
			}
		}

		for (const auto& con : mod->connections()) {
			instr.comment("Connection");
			instr.MOV(
				outConvert(con.first),
				convert(con.second)
			);
		}

		for(auto& sub_assign : sub_assigns) {
			convert(sub_assign.second);
		}

	}

	void convert(std::vector<SubAssign>& parts) {

		auto compare = [](const SubAssign& a, const SubAssign& b) {
			return a.to.offset < b.to.offset;
		};
		std::sort(parts.begin(), parts.end(), compare);
		// note that the slicing can create potential aliases
		auto getSliced = [this](const SubAssign& s) {
			if (s.from_offset == 0 && s.from_width == s.to.width) {
				// this part of subword assignment has a full word on the rhs
				// no need to slice the full word into a full word!
				// this happens in a connection like:
				// connect $sig$ [5:0] \d
				// where \d is also 6 bits
				return s.from_name;
			} else {
				auto sliced = def_wire.mk(s.to.width);
				instr.SLICE(sliced, s.from_name, s.from_offset, s.to.width);
				return sliced;
			}
		};
		auto prev_assign = parts.front();
		auto prev_sliced = getSliced(prev_assign);
		std::string res = prev_sliced;
		int concat_offset = prev_assign.to.width;
		for(auto it = parts.begin() + 1; it != parts.end(); it ++) {

			SubAssign current_assign = *it;
			auto current_sliced = getSliced(current_assign);
			res = def_wire.mk(concat_offset + current_assign.to.width);
			instr.CONCAT(res, prev_sliced, current_sliced, concat_offset);
			concat_offset += current_assign.to.width;
			prev_sliced = current_sliced;
		}
		// although this is an alias, we leave it as is for the manticore compiler
		// to remove it
		instr.MOV(def_wire.get(parts.front().to.wire), res);


	}

	void emit() {

		std::ofstream ofs(filename, std::ios::out);

		if (ofs.is_open() == false) {
			log_error("Could not open %s\n", filename.c_str());
		}

		ofs << ".prog:" << std::endl;
		ofs << ".proc main:" << std::endl;
		ofs << "// wires " << std::endl;
		ofs << def_wire.defs.rdbuf() << std::endl;
		ofs << "// constants" << std::endl;
		ofs << def_const.defs.rdbuf() << std::endl;
		ofs << "//instructions" << std::endl;
		ofs << instr.builder.rdbuf() << std::endl;

		ofs.close();
		log("Finished writing to %s\n", filename.c_str());

	}
};

struct ManticoreAssemblyWriter : public Pass {

	ManticoreAssemblyWriter() : Pass("manticore_writer", "write the design as manticore assembly") {}

	void help() override
	{
		//   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
		log("\n");
		log("    manticore_write filename\n");
		log("\n");
	}

	void execute(std::vector<std::string> args, Design *design) override
	{

		if (args.size() != 2) {
			log_error("Invalid call to %s\n", pass_name.c_str());
		}

		auto worker = new ManticoreAssemblyWorker(args[1], design);

		worker->generate();
		worker->emit();

		// std::ofstream &
	}

} ManticoreAssemblyWriter;

PRIVATE_NAMESPACE_END