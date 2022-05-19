#include "kernel/modtools.h"
#include "kernel/yosys.h"
#include "passes/manticore/manticore_utils.h"
#include <algorithm>
#include <fstream>
#include <functional>
#include <queue>
#include <regex>
USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

static int bitLength(const unsigned int value)
{
	unsigned int v = value;
	if (v == 0) {
		return 1;
	} else {

		unsigned int len = 0;
		while (v > 0) {
			len++;
			v >>= 1;
		}
		return len;
	}
}
static inline std::string convertFmt(const std::string &original, std::vector<std::string> sizes)
{

	if (original.empty()) {
		return original;
	}

	auto parts = manticore::split(original, '%');
	if (parts.size() == 1) {
		return parts.front();
	}
	std::stringstream builder;
	int size_ix = 0;
	bool first = true;
	for (const auto &p : parts) {
		if (first) {
			first = false;
			if (!p.empty())
				builder << p;
			continue;
		}

		if (p[0] == '0') {
			builder << "%0";
		} else {
			builder << '%';
		}
		auto spec_type_pos = p.find_first_of("hbdHBD");
		if (spec_type_pos == std::string::npos) {
			log_error("Invalid specifier %s\n", original.c_str());
		}
		log_assert(size_ix < GetSize(sizes));

		auto size_str = sizes[size_ix++];
		auto size_begin = p.find_first_of("123456789");
		if (size_begin != std::string::npos) {
			size_str = p.substr(size_begin, spec_type_pos);
		}
		builder << size_str << p[spec_type_pos];
		if (spec_type_pos + 1 < p.size()) {
			builder << p.substr(spec_type_pos + 1);
		}
	}

	return builder.str();
}
static inline std::string sourceInfo(RTLIL::AttrObject *obj)
{

	if (obj->has_attribute(ID::src)) {
		auto src = obj->get_src_attribute();
		return stringf("@Sourceinfo [ file = \"%s\" ]", src.c_str());
	}
	return "";
}
struct NameBuilder {
	// const std::string prefix;
	Module *mod;
	int index;
	inline std::string next(const std::string &prefix)
	{

		auto n = stringf("%%%s%d", prefix.c_str(), index);
		index++;
		// log("New name : %s\n", n.c_str());
		return n;
	}
	NameBuilder(Module *m) : mod(m), index(0) {}
};

struct WireBuilder {

      public:
	std::stringstream defs;
	std::stringstream mems;
	std::stringstream regs;

      private:
	NameBuilder namer;
	dict<Wire *, std::string> wires;
	dict<Cell *, std::pair<std::string, std::string>> states;
	dict<RTLIL::Memory *, std::string> memories;

      public:
	WireBuilder(Module *m) : namer(m) {}
	inline std::string temp(int width)
	{
		auto name = namer.next("w");
		defs << "\t.wire "
		     << " " << name << " " << width << std::endl;
		return name;
	}
	inline std::string get(Wire *w)
	{
		if (!wires.count(w)) {

			auto name = namer.next("w");

			if (w->has_attribute(ID::hdlname)) {
				auto dotted_name = std::regex_replace(w->get_string_attribute(ID::hdlname), std::regex(" "), "/");
				defs << "\t@DEBUGSYMBOL [ symbol = \"" << dotted_name << "\" ]" << std::endl;
			} else if (w->name.c_str()[0] == '\\') {
				// this is user signal so create a debug symbol
				defs << "\t@DEBUGSYMBOL [ symbol = \"" << w->name.str().substr(1) << "\" ]" << std::endl;
			}
			auto srcinfo = sourceInfo(w);
			if (!srcinfo.empty()) {
				defs << "\t" << srcinfo << std::endl;
			}
			defs << "// " << w->name.str() << std::endl;
			defs << "\t.wire " << name << " " << w->width << std::endl;
			wires.emplace(w, name);
		}
		return wires[w];
	}

      private:
	void tryCreateState(Cell *dff)
	{
		log_assert(dff->type == ID($dff));
		if (!states.count(dff)) {
			int width = dff->getParam(ID::WIDTH).as_int();
			auto q_name = namer.next("i");
			auto d_name = namer.next("o");

			regs << "\t.reg " << dff->name.str() << " " << width << " .input " << q_name << " ";
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
				regs << "0b" << Const(bits).as_string() << " ";
			}
			regs << ".output " << d_name << std::endl;
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

	inline std::string getMemory(RTLIL::Memory *mem)
	{

		if (memories.count(mem) == 0) {

			mems << "\t@MEMBLOCK [block = \"" << mem->name.str() << "\", width = " << mem->width << ", capacity = " << mem->size << "]"
			     << std::endl;
			if (mem->has_attribute(ID::MEM_INIT_FILE)) {
				mems << "\t@MEMINIT [ file = \"" << mem->get_string_attribute(ID::MEM_INIT_FILE) << "\", width = " << mem->width
				     << ", count = " << mem->size << "]" << std::endl;
			}

			std::string mem_name = mem->name.str();

			mems << "\t.mem " << mem_name << " " << bitLength(mem->size - 1) << std::endl;

			memories[mem] = mem_name;
		}
		return memories[mem];
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

			defs << "\t.const " << name << " " << value.size();
			if (!value.is_fully_def()) {
				auto defined = std::vector<State>();
				for (const auto &bit : value.bits) {
					if (bit == State::S0 || bit == State::S1) {
						defined.push_back(bit);
					} else {
						defined.push_back(State::S0);
					}
				}
				defs << " 0b" << Const(defined).as_string() << std::endl;
			} else {
				defs << " 0b" << value.as_string() << std::endl;
			}
			constants.emplace(value, name);
		}
		return constants[value];
	}
};

struct InstructionBuilder {

	// enum AluOp
	std::stringstream builder;
	int interrupt_order = 0;
	InstructionBuilder() : interrupt_order(0) {}

	inline void LOAD(const std::string &rd, const std::string &base) { builder << "\tLLD " << rd << ", " << base << "[0];" << std::endl; }
	inline void STORE(const std::string &rs, const std::string &base, const std::string &pred)
	{
		builder << "\tLST " << rs << ", " << base << "[0], " << pred << ";" << std::endl;
	}
	inline void PADZERO(const std::string &rd, const std::string &rs, int width)
	{
		builder << "\tPADZERO " << rd << ", " << rs << ", " << width << ";" << std::endl;
	}
	inline void MOV(const std::string &rd, const std::string &rs) { builder << "\tMOV " << rd << ", " << rs << ";" << std::endl; }
	inline void SLICE(const std::string &rd, const std::string &rs, int offset, int len)
	{
		builder << "\tSLICE " << rd << ", " << rs << ", " << offset << ", " << len << ";" << std::endl;
	}

	// inline void CONCAT(const std::string &rd, const std::string &rs_low, const std::string &rs_high, int offset)
	// {
	// 	builder << "CONCAT " << rd << ", " << rs_low << ", " << rs_high << "[" << offset << "];" << std::endl;
	// }
	inline void PARMUX(const std::string &rd, const std::vector<std::string> choices, const std::string &def_case,
			   const std::vector<std::string> &conds)
	{
		log_assert(choices.size() == conds.size());
		builder << "\tPARMUX " << rd;
		for (int i = 0; i < GetSize(choices); i++) {
			builder << choices[i] << "[" << conds[i] << "], ";
		}
		builder << def_case << ";" << std::endl;
	}
	inline void MUX(const std::string &rd, const std::string &sel, const std::string &rfalse, const std::string &rtrue)
	{
		builder << "\tMUX " << rd << ", " << sel << ", " << rfalse << ", " << rtrue << ";" << std::endl;
	}

	inline void PUT(const std::string &rs, const std::string &pred)
	{
		builder << "\t(0, " << interrupt_order++ << ") PUT " << rs << ", " << pred << ";" << std::endl;
	}

	inline void FLUSH(const std::string &fmt, const std::string &pred)
	{
		builder << "\t(0, " << interrupt_order++ << ") FLUSH \"" << fmt << "\", " << pred << ";" << std::endl;
	}

	inline void FINISH(const std::string &pred) { builder << "\t(0, " << interrupt_order++ << ") FINISH " << pred << ";" << std::endl; }

	inline void ASSERT(const std::string &pred) { builder << "\t(0, " << interrupt_order++ << ") ASSERT " << pred << ";" << std::endl; }

	inline void STOP(const std::string &pred) { builder << "\t(0, " << interrupt_order++ << ") STOP " << pred << ";" << std::endl; }

#define BINOP_DEF(op)                                                                                                                                \
	inline void op(const std::string &rd, const std::string &rs1, const std::string &rs2)                                                        \
	{                                                                                                                                            \
		builder << "\t" << #op << " " << rd << ", " << rs1 << ", " << rs2 << ";" << std::endl;                                               \
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
	BINOP_DEF(SLT)
	BINOP_DEF(SLTS) // Last ALU op

#undef BINOP_DEF
	inline void emit(const std::string &str)
	{
		if (str.empty() == false)
			builder << "\t" << str << std::endl;
	}
	inline void comment(const std::string &msg) { builder << "// " << msg << std::endl; }
	inline void comment(Cell *cell) { builder << "// " << RTLIL::id2cstr(cell->name) << " : " << RTLIL::id2cstr(cell->type) << std::endl; }
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

	// a place holder for systemcall. We need to first emit any other instruction
	// and then emit system calls in correct order.
	std::vector<Cell *> syscalls;

	ManticoreAssemblyWorker(const std::string &filename, Design *design)
	    : filename(filename), design(design), mod(design->top_module()), sigmap(design->top_module()), def_const(design->top_module()),
	      def_wire(design->top_module())
	{
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
				// log("full chunk %s\n", RTLIL::id2cstr(chunk.wire->name));
				auto n = def_wire.get(chunk.wire);
				available_sigs.emplace(SigSpec(chunk), n);
				return n;
			} else {
				// log("sliced chunk %s[%d +: %d]\n", log_id(chunk.wire->name), chunk.offset, chunk.width);
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
			// log("Chunk size %d\n", GetSize(chunks));
			std::string prev_res = convert(chunks.front());
			if (chunks.size() == 1) {
				return prev_res;
			}

			int pos = chunks.front().width;
			SigSpec concat_sig = SigSpec(chunks.front());
			// create concatenation
			instr.comment(stringf("Concat gen %s", log_signal(sig)));
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

		if (checker.equalWidthPorts()) {
			// note that the A and B might be negative, but that is OK
			instr.SEQ(res, convert(cell->getPort(ID::A)), convert(cell->getPort(ID::B)));
		} else {
			// ensure both operands are unsigned
			checker.assertBothUnsigned();
			auto max_width = std::max(cell->getParam(ID::A_WIDTH).as_int(), cell->getParam(ID::B_WIDTH).as_int());
			instr.SEQ(res, padConvert(cell->getPort(ID::A), max_width), padConvert(cell->getPort(ID::B), max_width));
		}
	}

	inline void mkCompare(Cell *cell, const std::string &res, bool less = true)
	{

		auto checker = CellSanityChecker(cell);
		checker.assertSignedOperandsRequireEqualWidth();
		checker.assertEqualSigns();


		auto a_signed = cell->getParam(ID::A_SIGNED).as_bool();
		auto b_signed = cell->getParam(ID::B_SIGNED).as_bool();

		auto op1_port = less ? ID::A : ID::B;
		auto op2_port = less ? ID::B : ID::A;

		if (a_signed) {
			checker.assertEqualWidth();
			instr.SLTS(res, convert(cell->getPort(op1_port)), convert(cell->getPort(op2_port)));
		} else {
			auto w = std::max(cell->getParam(ID::A_WIDTH).as_int(),
									 cell->getPort(ID::B_WIDTH).as_int());
			instr.SLT(res, padConvert(cell->getPort(op1_port), w), padConvert(cell->getPort(op2_port), w));
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
		instr.comment(cell);
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
			instr.ADD(y_name, bits_flipped, def_const.get(Const(1, a_width)));

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
			instr.SEQ(b0, a_name, def_const.get(Const(State::S0, a_width)));
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
			log_assert(!a_signed);

			auto prevres = def_wire.temp(1);
			auto const_true = def_const.get(Const(State::S1, 1));
			if ((a_width & 0x00000001) == 1) { // if a has odd width
				auto prevbit = def_wire.temp(1);
				instr.SLICE(prevbit, a_name, 0, 1);
				instr.XOR(prevres, prevbit, const_true);
			} else {
				instr.SLICE(prevres, a_name, 0, 1);
			}

			for (int i = 1; i < a_width; i++) {
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

			auto y_width = cell->getParam(ID::Y_WIDTH).as_int();
			auto y_name = convert(cell->getPort(ID::Y));
			if (y_width == 1) {
				mkSeq(cell, y_name);
			} else {
				auto tmp = def_wire.temp(1);
				mkSeq(cell, tmp);
				instr.PADZERO(y_name, tmp, y_width);
			}

		} else if (cell->type == ID($ne)) {
			// same as $eq, except with negate the results (XOR with true is basically not)
			auto y_width = cell->getParam(ID::Y_WIDTH).as_int();
			auto y_name = convert(cell->getPort(ID::Y));
			auto seq_res = def_wire.temp(1);
			mkSeq(cell, seq_res);
			auto const_true = def_const.get(Const(State::S1, 1));
			if (y_width == 1) {
				instr.XOR(y_name, seq_res, const_true);
			} else {
				auto ne_res = def_wire.temp(1);
				instr.XOR(ne_res, seq_res, const_true);
				instr.PADZERO(y_name, ne_res, y_width);
			}
		} else if (cell->type.in(ID($lt), ID($gt))) {

			auto y_name = convert(cell->getPort(ID::Y));
			auto y_width = cell->getParam(ID::Y_WIDTH).as_int();
			if (y_width == 1) {
				mkCompare(cell, y_name, cell->type == ID($lt));
			} else {
				auto tmp = def_wire.temp(1);
				mkCompare(cell, tmp, cell->type == ID($lt));
				instr.PADZERO(y_name, tmp, y_width);
			}

		} else if (cell->type.in(ID($le), ID($ge))) {

			auto y_name = convert(cell->getPort(ID::Y));
			auto y_width = cell->getParam(ID::Y_WIDTH).as_int();

			auto equal = def_wire.temp(1);
			auto comp = def_wire.temp(1);

			mkSeq(cell, equal);
			mkCompare(cell, comp, cell->type == ID($le));

			if (y_width == 1) {
				instr.OR(y_name, comp, equal);
			} else {
				auto tmp = def_wire.temp(1);
				instr.OR(tmp, comp, equal);
				instr.PADZERO(y_name, tmp, y_width);
			}

		}  else if (cell->type == ID($logic_and) || cell->type == ID($logic_or)) {

			checker.assertBothUnsigned();

			auto a_zero = def_wire.temp(1);
			auto b_zero = def_wire.temp(1);
			auto a_nonzero = def_wire.temp(1);
			auto b_nonzero = def_wire.temp(1);
			auto a_name = convert(cell->getPort(ID::A));
			auto b_name = convert(cell->getPort(ID::B));
			auto a_width = cell->getParam(ID::A_WIDTH).as_int();
			auto b_width = cell->getParam(ID::B_WIDTH).as_int();
			instr.SEQ(a_zero, a_name, def_const.get(Const(State::S0, a_width)));
			instr.SEQ(b_zero, b_name, def_const.get(Const(State::S0, b_width)));
			auto const_true = def_const.get(Const(State::S1, 1));
			instr.XOR(a_nonzero, a_zero, const_true);
			instr.XOR(b_nonzero, b_zero, const_true);

			auto y_width = cell->getParam(ID::Y_WIDTH).as_int();
			auto y_name = convert(cell->getPort(ID::Y));
			instr.emit(sourceInfo(cell));
			if (cell->type == ID($logic_and)) {
				if (y_width == 1) {
					instr.AND(y_name, a_nonzero, b_nonzero);
				} else {
					auto res = def_wire.temp(1);
					instr.AND(res, a_nonzero, b_nonzero);
					instr.PADZERO(y_name, res, y_width);
				}
			} else {
				if (y_width == 1) {
					instr.OR(y_name, a_nonzero, b_nonzero);
				} else {
					auto res = def_wire.temp(1);

					instr.OR(res, a_nonzero, b_nonzero);
					instr.PADZERO(y_name, res, y_width);
				}
			}

		} else if (cell->type == ID($mux)) {

			auto a_name = convert(cell->getPort(ID::A));
			auto b_name = convert(cell->getPort(ID::B));
			auto y_name = convert(cell->getPort(ID::Y));
			auto s_name = convert(cell->getPort(ID::S));
			instr.emit(sourceInfo(cell));
			instr.MUX(y_name, s_name, a_name, b_name);

		} else if (cell->type == ID($pmux)) {
			auto a_name = convert(cell->getPort(ID::A));
			auto width = cell->getParam(ID::WIDTH).as_int();
			auto num_cases = cell->getParam(ID::S_WIDTH).as_int();
			std::vector<std::string> cases;
			std::vector<std::string> conditions;
			auto data_bits = cell->getPort(ID::B).to_sigbit_vector();
			auto cond_bits = cell->getPort(ID::S).to_sigbit_vector();
			for (int case_ix = 0; case_ix < num_cases; case_ix++) {
				std::vector<SigBit> case_bits;
				for (int i = 0; i < width; i++) {
					case_bits.push_back(data_bits[case_ix * width + i]);
				}
				cases.push_back(convert(SigSpec(case_bits)));
				conditions.push_back(convert(SigSpec(cond_bits[case_ix])));
			}
			instr.emit(sourceInfo(cell));
			instr.PARMUX(convert(cell->getPort(ID::Y)), cases, a_name, conditions);
		} else if (cell->type == ID($memrd_v2)) {

			log_assert(cell->getParam(ID::CLK_ENABLE).is_fully_zero());
			log_assert(cell->getParam(ID::TRANSPARENCY_MASK).is_fully_zero());
			log_assert(cell->getParam(ID::COLLISION_X_MASK).is_fully_zero());
			log_assert(cell->getParam(ID::INIT_VALUE).is_fully_undef());
			log_assert(cell->getParam(ID::ARST_VALUE).is_fully_undef());
			log_assert(cell->getParam(ID::SRST_VALUE).is_fully_undef());
			log_assert(cell->getParam(ID::CE_OVER_SRST).is_fully_zero());

			auto data_name = convert(cell->getPort(ID::DATA));
			log_assert(cell->getPort(ID::EN).is_fully_ones());
			auto memid = cell->getParam(ID::MEMID).decode_string();
			auto mem = mod->memories[memid];

			auto addr_width = std::max(cell->getParam(ID::ABITS).as_int(), bitLength(mem->size - 1));

			auto addr_name = convert(cell->getPort(ID::ADDR));

			auto load_addr = def_wire.temp(addr_width);
			instr.ADD(load_addr, def_wire.getMemory(mem), addr_name);
			instr.emit(sourceInfo(cell));
			instr.LOAD(data_name, load_addr);

		} else if (cell->type == ID($memwr_v2)) {

			// because of the manticore_memory pass we have the guarantee
			// that the we can use teh enable bit 0 as the enable, i.e., there
			// are no "bit-" or "byte-" enables rather, there is a memory line
			// enable. In other words the EN signal is a fully repeated bit pattern

			auto mem = mod->memories[cell->getParam(ID::MEMID).decode_string()];
			auto addr_bits = std::max(cell->getParam(ID::ABITS).as_int(), bitLength(mem->size - 1));
			auto store_addr = def_wire.temp(addr_bits);
			instr.ADD(store_addr, def_wire.getMemory(mem), convert(cell->getPort(ID::ADDR)));
			auto en_bit = cell->getPort(ID::EN)[0];
			auto pred_name = convert(SigSpec(en_bit));
			instr.emit(sourceInfo(cell));
			instr.STORE(convert(cell->getPort(ID::DATA)), store_addr, pred_name);

		} else if (cell->type == ID($dff)) {

			auto current_state = def_wire.getCurrent(cell);
			auto next_state = def_wire.getNext(cell);

			instr.MOV(convert(cell->getPort(ID::Q)), current_state);
			instr.MOV(next_state, convert(cell->getPort(ID::D)));

		} else if (cell->type == ID($manticore)) {
			syscalls.push_back(cell);
		} else {
			log_error("Can not handle cell type in %s.%s : %s\n", log_id(mod), log_id(cell), log_id(cell->type));
		}
	}

	void convertSyscall(Cell *cell)
	{

		std::string call_type = cell->getParam(ID::TYPE).decode_string();
		instr.comment(cell);
		log("Handling systemcall %s.%s of type %s\n", log_id(mod), log_id(cell), call_type.c_str());
		if (call_type == "$display") {

			auto fmt = cell->getParam(ID::FMT).decode_string();
			auto sizes_str = cell->getParam(ID::VAR_ARG_SIZE).decode_string();
			auto arg_size = manticore::split(sizes_str, ',');

			auto data_bits = cell->getPort(ID::B).to_sigbit_vector();
			auto en_sig = cell->getPort(ID::EN);
			auto en_name = convert(en_sig);
			log_assert(en_sig.size() == 1);

			auto bit_iter = data_bits.begin();
			for (const auto &sz_str : arg_size) {

				auto sz = std::stoi(sz_str);
				auto arg_bits = std::vector<SigBit>(bit_iter, bit_iter + sz);
				auto arg_sig = SigSpec(arg_bits);
				auto arg_name = convert(arg_sig);

				instr.emit(sourceInfo(cell));
				instr.PUT(arg_name, en_name);
				bit_iter += sz;
			}
			auto srcinfo = sourceInfo(cell);
			instr.emit(sourceInfo(cell));
			instr.FLUSH(convertFmt(fmt, arg_size), en_name);

		} else if (call_type == "$stop" || call_type == "$finish") {

			auto en_sig = cell->getPort(ID::EN);
			auto en_name = convert(en_sig);
			auto srcinfo = sourceInfo(cell);
			instr.emit(sourceInfo(cell));
			if (call_type == "$stop") {
				instr.STOP(en_name);
			} else {
				instr.FINISH(en_name);
			}
		} else if (call_type == "$assert") {

			auto en_name = convert(cell->getPort(ID::EN));
			auto cond_name = convert(cell->getPort(ID::A));
			// if (en_name) assert(cond_name) is equivalent to assert(~en_name | cond_name)
			auto dis_name = def_wire.temp(1);
			instr.XOR(dis_name, en_name, def_const.get(Const(State::S1, 1)));
			auto implication = def_wire.temp(1);
			instr.OR(implication, dis_name, cond_name);
			auto srcinfo = sourceInfo(cell);
			instr.emit(srcinfo);
			instr.ASSERT(implication);

		} else {
			log_error("%s.%s : %s not implemented yet!\n", log_id(mod), log_id(cell), call_type.c_str());
		}
	}
	void generate()
	{

		for (auto cell : mod->cells()) {
			convert(cell);
		}

		// sort the syscall instructions

		auto cellOrdering = [](const Cell *cell1, const Cell *cell2) -> bool {
			auto order1 = cell1->getParam(ID::ORDER).as_int();
			auto order2 = cell2->getParam(ID::ORDER).as_int();
			return order1 < order2;
		};

		std::sort(syscalls.begin(), syscalls.end(), cellOrdering);

		for (auto cell : syscalls) {
			convertSyscall(cell);
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

		auto append = [&](const std::string &cmt, std::stringstream &s) {
			ofs << "// " << cmt << std::endl;
			if (s.tellp() > 0) {
				ofs << s.rdbuf() << std::endl;
			}
		};
		ofs << ".prog:" << std::endl;
		ofs << ".proc main:" << std::endl;

		append("wires", def_wire.defs);

		append("states", def_wire.regs);

		append("memories", def_wire.mems);

		append("constants", def_const.defs);

		append("instructions", instr.builder);

		ofs.flush();
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

		log_header(design, "Emitting Manticore Assembly\n");
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