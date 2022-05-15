#include "kernel/modtools.h"
#include "kernel/yosys.h"
#include <algorithm>
#include <fstream>
#include <functional>
#include <queue>
USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN
struct NameBuilder {
	// const std::string prefix;
	Module *mod;
	int index;
	inline std::string next(const std::string &prefix)
	{

		auto n = mod->uniquify(stringf("$%s$", prefix.c_str()), index);
		// log("New name : %s\n", n.c_str());
		return n.str();
	}
	NameBuilder(Module *m) : mod(m), index(0) {}
};

struct WireBuilder {

      public:
	std::stringstream defs;

      private:
	NameBuilder namer;
	dict<Wire *, std::string> wires;
	dict<Cell *, std::pair<std::string, std::string>> states;

      public:
	WireBuilder(Module *m) : namer(m) {}
	inline std::string temp(int width)
	{
		auto name = namer.next("temp");
		defs << ".wire "
		     << " " << name << " " << width << std::endl;
		return name;
	}
	inline std::string get(Wire *w)
	{
		if (!wires.count(w)) {
			defs << ".wire " << w->name.str() << " " << w->width << std::endl;
			wires.emplace(w, w->name.str());
		}
		return wires[w];
	}

      private:
	void tryCreateState(Cell *dff)
	{
		log_assert(dff->type == ID($dff));
		if (!states.count(dff)) {
			int width = dff->getParam(ID::WIDTH).as_int();
			auto q_name = namer.next("current");
			auto d_name = namer.next("next");

			defs << ".reg " << dff->name.str() << " " << width << " .input " << q_name << " ";
			if (dff->has_attribute(ID::INIT)) {
				auto initval = dff->get_const_attribute(ID::INIT);
				std::vector<RTLIL::State> bits;
				for (int ix = 0; ix < initval.size(); ix++) {
					if (initval.bits[ix] == State::S0 || initval.bits[ix] == State::S1) {
						bits.push_back(initval[ix]);
					} else {
						log_warning("Replacing undefined bit %s.init[%d] with zero\n", log_id(dff->name), ix);
						bits.push_back(State::S0);
					}
				}
				defs << "0b" << Const(bits).as_string() << " ";
			}
			defs << ".output " << d_name << std::endl;
			states.emplace(dff, std::make_pair(q_name, d_name));
		}
	}

      public:
	inline std::string getCurrent(Cell *dff)
	{
		tryCreateState(dff);
		return states[dff].first;
	}
	inline std::string getNext(Cell *dff)
	{
		tryCreateState(dff);
		return states[dff].second;
	}
};

struct ConstBuilder {

      public:
	std::stringstream defs;

      private:
	NameBuilder namer;
	dict<Const, std::string> constants;

      public:
	ConstBuilder(Module *m) : namer(m) {}
	inline std::string get(const Const &value)
	{
		if (!constants.count(value)) {
			auto name = namer.next("c");
			defs << ".const " << name << " " << value.size() << "0b" << value.as_string() << std::endl;
			constants.emplace(value, name);
		}
		return constants[value];
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
		builder << "PADZERO " << rd << ", " << rs << ", " << width << ";" << std::endl;
	}
	inline void MOV(const std::string &rd, const std::string &rs) { builder << "MOV " << rd << ", " << rs << ";" << std::endl; }
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
	inline void MUX(const std::string &rd, const std::string &sel, const std::string &rfalse, const std::string &rtrue)
	{
		builder << "MUX " << rd << ", " << sel << ", " << rfalse << ", " << rtrue << std::endl;
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
	ConstBuilder def_const;
	// helper object to handle wires and temp wires
	WireBuilder def_wire;

	// helper class to create instructions
	InstructionBuilder instr;

	// a dict of already converted sigs that are singleton chunks,
	// we can not use SigChunk as the key because they are not hashable
	dict<SigSpec, std::string> available_sigs;

	ManticoreAssemblyWorker(const std::string &filename, Design *design)
	    : filename(filename), design(design), mod(design->top_module()), sigmap(design->top_module()), def_const(design->top_module()),
	      def_wire(design->top_module())
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
			auto next_name = def_wire.temp(next_width);
			instr.CONCAT(next_name, prev.first, current_name, accum_width);

			auto next = std::make_pair(next_name, next_width);
			return convertChunksRecurse__(chunks, beg + 1, next);
		}
	}

	// get the Manticore name of a chunk, this may involve creating a SLICE instruction
	inline const std::string convert(const SigChunk &chunk)
	{
		auto found = available_sigs.find(SigSpec(chunk));

		if (found != available_sigs.end()) {
			return found->second;
		}
		if (chunk.is_wire()) {
			if (chunk.wire->width == chunk.width) {
				log("full chunk %s\n", RTLIL::id2cstr(chunk.wire->name));
				auto n = def_wire.get(chunk.wire);
				available_sigs.emplace(SigSpec(chunk), n);
				return n;
			} else {
				log("sliced chunk %s[%d +: %d]\n", log_id(chunk.wire->name), chunk.offset, chunk.width);
				auto chunk_name = def_wire.temp(chunk.width);
				instr.SLICE(chunk_name, def_wire.get(chunk.wire), chunk.offset, chunk.width);
				available_sigs.emplace(SigSpec(chunk), chunk_name);
				return chunk_name;
			}
		} else {
			return def_const.get(Const(chunk.data));
		}
	}

	// get the manticore name of a SigSpec, may involve creating CONCAT and SLICE
	// instruction
	const std::string convert(const SigSpec &sig)
	{
		auto found = available_sigs.find(sig);
		if (found != available_sigs.end()) {
			return (*found).second;
		}
		// else do the conversion
		if (sig.is_wire()) {
			auto n = def_wire.get(sig.as_wire());
			return n;
		} else {

			auto chunks = sig.chunks();
			log_assert(chunks.size() != 0);
			log("Chunk size %d\n", GetSize(chunks));

			std::string prev_res = convert(chunks.front());
			int pos = chunks.front().width;
			SigSpec concat_sig = SigSpec(chunks.front());
			// create concatenation
			for (int cix = 1; cix < GetSize(chunks); cix++) {
				concat_sig.append(chunks[cix]);
				int concat_width = concat_sig.size();
				auto op1 = def_wire.temp(concat_width);
				auto op2 = def_wire.temp(concat_width);

				instr.PADZERO(op1, prev_res, concat_width);
				instr.PADZERO(op2, convert(chunks[cix]), concat_width);
				auto shifted = def_wire.temp(concat_width);
				instr.SLL(shifted, op2, def_const.get(Const(pos, 32)));
				auto this_res = def_wire.temp(concat_width);
				instr.OR(this_res, shifted, op1);
				const SigSpec this_sig = concat_sig;
				available_sigs.emplace(this_sig, this_res);
				prev_res = this_res;
				pos = concat_width;
			}

			return prev_res;
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

		inline bool outputWidthIsMax() const
		{
			auto w = std::max(cell->getParam(ID::A_WIDTH).as_int(), cell->getParam(ID::B_WIDTH).as_int());
			return w == cell->getParam(ID::Y_WIDTH).as_int();
		}

		inline bool boolOutput() const { return cell->getParam(ID::Y_WIDTH).as_int() == 1; }

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

		inline void assertBoolOutput() const
		{
			if (!boolOutput()) {
				log_error("Expected boolean output in cell %s of type %s\n", RTLIL::id2cstr(cell->name), RTLIL::id2cstr(cell->type));
			}
		}
		inline void assertOutputIsMaxWidth() const
		{
			if (!outputWidthIsMax()) {
				log_error("Expected maximum input width on output in cell %s of type %s\n", RTLIL::id2cstr(cell->name),
					  RTLIL::id2cstr(cell->type));
			}
		}
	};

	std::string padConvert(const SigSpec &sig, int width)
	{
		log_assert(sig.size() <= width);
		if (sig.size() < width) {
			auto padded = def_wire.temp(width);
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

	inline void mkSlts(const std::string &res, int a_width, int b_width, bool a_signed, bool b_signed, const SigSpec &a, const SigSpec &b,
			   Cell *cell)
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
		auto mkZero = [this](int width) { return def_const.get(Const(State::S0, width)); };
		if (a_signed != b_signed) {
			log_error("Expected equal signs in cell %s of type %s\n", RTLIL::id2cstr(cell->name), RTLIL::id2cstr(cell->type));
		}
		if (a_signed) {
			if (a_width != b_width) {
				log_error("Expected equal width in cell %s of type %s\n", RTLIL::id2cstr(cell->name), RTLIL::id2cstr(cell->type));
			}

			// easier case, we do not need to sign extend the operands
			int width = a_width;
			auto temp_name = def_wire.temp(width);
			instr.SUB(temp_name, convert(a), convert(b));
			instr.SLTS(res, temp_name, mkZero(width));
		} else {
			// operands are unsigned, therefore we need to zero extends them to
			// ensure the sign bit is always zero
			int extended_width = std::max(a_width, b_width) + 1;
			auto temp_name = def_wire.temp(extended_width);
			instr.SUB(temp_name, padConvert(a, extended_width), padConvert(b, extended_width));
			instr.SLTS(res, temp_name, mkZero(extended_width));
		}
	}

#define UNARY_OP_HEADER                                                                                                                              \
	auto a_width = cell->getParam(ID::A_WIDTH).as_int();                                                                                         \
	auto y_width = cell->getParam(ID::Y_WIDTH).as_int();                                                                                         \
	auto a_signed = cell->getParam(ID::A_SIGNED).as_bool();                                                                                      \
	auto a_name = convert(cell->getPort(ID::A));                                                                                                 \
	auto y_name = convert(cell->getPort(ID::Y));

	// convert a cell into instructions
	void convert(Cell *cell)
	{

		auto checker = CellSanityChecker(cell);
		auto maxWidth = [cell]() { return std::max(cell->getParam(ID::A_WIDTH).as_int(), cell->getParam(ID::B_WIDTH).as_int()); };

		if (cell->type == ID($not)) {
			auto width = cell->getParam(ID::A_WIDTH).as_int();
			log_assert(width == cell->getParam(ID::Y_WIDTH).as_int());
			instr.XOR(convert(cell->getPort(ID::Y)), convert(cell->getPort(ID::A)), def_const.get(Const(State::S1, width)));
		} else if (cell->type == ID($pos)) {
			UNARY_OP_HEADER
			if (a_signed && a_width < y_width) {
				// sign extend A
				auto a_sign = def_wire.temp(1);
				instr.SLICE(a_sign, a_name, 0, 1);
				auto extension = def_wire.temp(y_width);
				std::vector<State> extension_mask;
				for (int i = 0; i < a_width; i++)
					extension_mask.push_back(State::S0);
				for (int i = 0; i < (y_width - a_width); i++)
					extension_mask.push_back(State::S1);

				instr.MUX(extension, a_sign, def_const.get(Const(State::S0, y_width)), def_const.get(Const(extension_mask)));
				auto padded = def_wire.temp(y_width);
				instr.PADZERO(padded, a_name, y_width);
				instr.OR(y_name, padded, extension);
			} else if (a_width > y_width) {
				instr.SLICE(y_name, a_name, 0, y_width);
			} else {
				instr.MOV(y_name, a_name);
			}

		} else if (cell->type == ID($neg)) {
			UNARY_OP_HEADER
			log_assert(a_width == y_width);
			auto bits_flipped = def_wire.temp(a_width);
			instr.XOR(bits_flipped, a_name, def_const.get(Const(State::S1, a_width)));
			inst.ADD(y_name, bits_flipped, def_const.get(Const(1, a_width)));

		} else if (cell->type == ID($reduce_and)) {
			UNARY_OP_HEADER
			auto all_ones = def_const.get(Const(State::S1, a_width));
			log_assert(!a_signed);
			if (y_width == 1) {
				instr.SEQ(y_name, a_name, all_ones);
			} else {
				auto b0 = def_wire.temp(1);
				instr.SEQ(b0, a_name, all_ones);
				instr.PADZERO(y_name, b0, y_width);
			}
		} else if (cell->type == ID($reduce_or) || cell->type == ID($reduce_bool)) {
			UNARY_OP_HEADER
			log_assert(!a_signed);
			auto b0 = def_wire.temp(1);
			instr.SEQ(b0, a_name, def_const.get(State(State::S0, a_width)));
			auto const_true = def_const.get(Const(State::S1, 1));

			if (y_width == 1) {
				instr.XOR(y_name, b0, const_true);
			} else {
				auto notb0 = def_wire.temp(1);
				instr.XOR(notb0, b0, const_true);
				instr.PADZERO(y_name, notb0, y_width);
			}

		} else if (cell->type == ID($reduce_xor)) {

			UNARY_OP_HEADER

			log_assert(!a_signed);
			auto prevres = def_wire.temp(1);
			instr.SLICE(prevres, a_name, 0, 1);

			for (int i = 1; i < a_width; i++) {

				auto thisbit = def_wire.temp(1);
				instr.SLICE(thisbit, a_name, i, 1);
				auto thisres = def_wire.temp(1);
				instr.XOR(thisres, prevres, thisbit);
				prevres = thisres;
			}
			if (y_width == 1)
				instr.MOV(y_name, prevres);
			else
				instr.PADZERO(y_name, prevres, y_width);

		} else if (cell->type == ID($reduce_xnor)) {

			UNARY_OP_HEADER
			auto prevbit = def_wire.temp(1);
			auto prevres = def_wire.temp(1);
			auto const_true = def_const.get(Const(State::S1, 1));
			instr.SLICE(prevbit, a_name, 0, 1);
			instr.XOR(prevres, prevbit, const_true);

			for (int i = 0; i < a_width; i++) {
				auto thisbit = def_wire.temp(1);
				instr.SLICE(thisbit, a_name, i, 1);
				auto temp = def_wire.temp(1);
				instr.XOR(temp, thisbit, prevres);
				auto thisres = def_wire.temp(1);
				instr.XOR(thisres, temp, const_true);
				prevres = thisres;
			}
			if (y_width == 1) {
				instr.MOV(y_name, prevres);
			} else {
				instr.PADZERO(y_name, prevres, y_width);
			}

		} else if (cell->type == ID($logic_not)) {
			UNARY_OP_HEADER
			log_assert(!a_signed);
			log_assert(y_width == 1);
			instr.SEQ(y_name, a_name, def_const.get(Const(State::S0, a_width)));
		} // end of unary ops
		  // begin binary ops
		else if (cell->type == ID($and)) {
			checker.assertSignedOperandsRequireEqualWidth();
			checker.assertEqualSigns();
			auto w = maxWidth();
			instr.AND(convert(cell->getPort(ID::Y)), padConvert(cell->getPort(ID::A), w), padConvert(cell->getPort(ID::B), w));
		} else if (cell->type == ID($or)) {
			checker.assertSignedOperandsRequireEqualWidth();
			checker.assertEqualSigns();
			auto w = maxWidth();
			instr.OR(convert(cell->getPort(ID::Y)), padConvert(cell->getPort(ID::A), w), padConvert(cell->getPort(ID::B), w));
		} else if (cell->type == ID($xor)) {
			checker.assertSignedOperandsRequireEqualWidth();
			checker.assertEqualSigns();
			auto w = maxWidth();
			instr.XOR(convert(cell->getPort(ID::Y)), padConvert(cell->getPort(ID::A), w), padConvert(cell->getPort(ID::B), w));
		} else if (cell->type == ID($add)) {
			checker.assertSignedOperandsRequireEqualWidth();
			// checker.assertEqualWidth(); need not to hold if one operand is a constant
			checker.assertEqualSigns();
			instr.ADD(convert(cell->getPort(ID::Y)), convert(cell->getPort(ID::A)), convert(cell->getPort(ID::B)));
		} else if (cell->type == ID($sub)) {
			checker.assertEqualWidth();
			checker.assertEqualSigns();

			instr.SUB(convert(cell->getPort(ID::Y)), convert(cell->getPort(ID::A)), convert(cell->getPort(ID::B)));
		} else if (cell->type == ID($eq)) {
			mkSeq(cell, convert(cell->getPort(ID::Y)));
		} else if (cell->type == ID($ne)) {
			auto temp_name = def_wire.temp(1);
			mkSeq(cell, temp_name);
			instr.XOR(convert(cell->getPort(ID::Y)), temp_name, def_const.get(Const(State::S0)));
			// same as $eq, except with negate the results (XOR with false is basically not)
		} else if (cell->type == ID($lt)) {

			mkSlts(convert(cell->getPort(ID::Y)), cell->getParam(ID::A_WIDTH).as_int(), cell->getParam(ID::B_WIDTH).as_int(),
			       cell->getParam(ID::A_SIGNED).as_bool(), cell->getParam(ID::B_SIGNED).as_bool(), cell->getPort(ID::A),
			       cell->getPort(ID::B), cell);
		} else if (cell->type == ID($gt)) {
			// just like $lt, with operands swapped
			mkSlts(convert(cell->getPort(ID::Y)), cell->getParam(ID::B_WIDTH).as_int(), cell->getParam(ID::A_WIDTH).as_int(),
			       cell->getParam(ID::B_SIGNED).as_bool(), cell->getParam(ID::A_SIGNED).as_bool(), cell->getPort(ID::B),
			       cell->getPort(ID::A), cell);
		} else if (cell->type == ID($le)) {

			// like $le but ord with $eq
			auto less = def_wire.temp(1);
			auto equal = def_wire.temp(1);
			mkSeq(cell, equal);
			mkSlts(less, cell->getParam(ID::A_WIDTH).as_int(), cell->getParam(ID::B_WIDTH).as_int(),
			       cell->getParam(ID::A_SIGNED).as_bool(), cell->getParam(ID::B_SIGNED).as_bool(), cell->getPort(ID::A),
			       cell->getPort(ID::B), cell);
			checker.assertBoolOutput();
			instr.OR(convert(cell->getPort(ID::Y)), less, equal);

		} else if (cell->type == ID($ge)) {
			// like $gt ored with $eq
			auto greater = def_wire.temp(1);
			auto equal = def_wire.temp(1);
			mkSlts(greater, cell->getParam(ID::B_WIDTH).as_int(), cell->getParam(ID::A_WIDTH).as_int(),
			       cell->getParam(ID::B_SIGNED).as_bool(), cell->getParam(ID::A_SIGNED).as_bool(), cell->getPort(ID::B),
			       cell->getPort(ID::A), cell);
			mkSeq(cell, equal);
			checker.assertBoolOutput();
			instr.OR(convert(cell->getPort(ID::Y)), greater, equal);
		}

		// Unary operators
		else
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

		for (const auto &con : mod->connections()) {
			instr.comment("Connection");
			instr.MOV(convert(con.first), convert(con.second));
		}
	}

	void emit()
	{

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