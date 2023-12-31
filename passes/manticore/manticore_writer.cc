#include "kernel/mem.h"
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
		if (size_begin != std::string::npos && size_begin < spec_type_pos) {
			size_str = p.substr(size_begin, spec_type_pos);
			log_error("Can not handle format specifier %s\n", p.c_str());
			log_flush();
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
		return stringf("@SOURCEINFO [ file = \"%s\" ]", src.c_str());
	}
	return "";
}
struct NameBuilder {
	// const std::string prefix;
	Module *mod;
	int index;
	inline std::string next(const std::string &prefix)
	{

		auto n = stringf("ys_%s%d", prefix.c_str(), index);
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
				auto dotted_name = std::regex_replace(w->get_string_attribute(ID::hdlname), std::regex(" "), ".");
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
		log_assert(dff->type == ID($dff) || dff->type == ID($memrd_v2));
		if (!states.count(dff)) {
			int width = dff->getParam(ID::WIDTH).as_int();
			auto r_name = namer.next("r");
			auto q_name = namer.next("i");
			auto d_name = namer.next("o");
			auto srcinfo = sourceInfo(dff);
			if (!srcinfo.empty()) {
				defs << "\t" << srcinfo << std::endl;
			}
			regs << "\t.reg " << r_name << " " << width << " .input " << q_name << " ";
			auto resolveInitBits = [](const Const& initval) {
				std::vector<RTLIL::State> bits;
				for (int ix = 0; ix < initval.size(); ix++) {
					if (initval.bits[ix] == State::S0 || initval.bits[ix] == State::S1) {
						bits.push_back(initval[ix]);
					} else {
						// log_warning("Replacing undefined bit %s.init[%d] with zero\n", log_id(dff->name), ix);
						bits.push_back(State::S0);
					}
				}
				return Const(bits);
			};
			if (dff->has_attribute(ID::INIT)) {
				auto initval = dff->get_const_attribute(ID::INIT);
				regs << "0b" << resolveInitBits(initval).as_string() << " ";
			} else if (dff->hasParam(ID::INIT_VALUE)) {
				auto initval = dff->getParam(ID::INIT_VALUE);
				regs << "0b" << resolveInitBits(initval).as_string() << " ";
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

			std::string mem_name = namer.next("m");
			auto srcinfo = sourceInfo(mem);
			if (!srcinfo.empty()) {
				defs << "\t" << srcinfo << std::endl;
			}
			mems << "\t.mem " << mem_name << " " << mem->width << " " << mem->size << std::endl;

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

	inline void LOAD(const std::string &rd, const std::string &addr, const std::string &mem, int order)
	{
		builder << "\t(" << mem << ", " << order << ") LLD " << rd << ", " << mem << "[ " << addr << " ];" << std::endl;
	}
	inline void STORE(const std::string &rs, const std::string &addr, const std::string &pred, const std::string &mem, int order)
	{
		builder << "\t(" << mem << ", " << order << ") LST " << rs << ", " << mem << "[ " << addr << " ], " << pred << ";" << std::endl;
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
		builder << "\tPARMUX " << rd << ", ";
		for (int i = 0; i < GetSize(choices); i++) {
			builder << conds[i] << " ? " << choices[i] << ", ";
		}
		builder << def_case << ";" << std::endl;
	}
	inline void MUX(const std::string &rd, const std::string &sel, const std::string &rfalse, const std::string &rtrue)
	{
		builder << "\tMUX " << rd << ", " << sel << ", " << rfalse << ", " << rtrue << ";" << std::endl;
	}

	inline void PUT(const std::string &rs, const std::string &pred)
	{
		builder << "\t(" << interrupt_order++ << ") PUT " << rs << ", " << pred << ";" << std::endl;
	}

	inline void FLUSH(const std::string &fmt, const std::string &pred)
	{
		builder << "\t(" << interrupt_order++ << ") FLUSH \"" << fmt << "\", " << pred << ";" << std::endl;
	}

	inline void FINISH(const std::string &pred) { builder << "\t(" << interrupt_order++ << ") FINISH " << pred << ";" << std::endl; }

	inline void ASSERT(const std::string &pred) { builder << "\t(" << interrupt_order++ << ") ASSERT " << pred << ";" << std::endl; }

	inline void STOP(const std::string &pred) { builder << "\t(" << interrupt_order++ << ") STOP " << pred << ";" << std::endl; }

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
	// a place holder for memory load/store nodes

	dict<IdString, std::vector<Cell *>> memory_writes;
	dict<IdString, std::vector<Cell *>> memory_reads;
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
		inline bool equalOperandWidth() const { return cell->getParam(ID::A_WIDTH) == cell->getParam(ID::B_WIDTH); }
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
				return equalOperandWidth();
			} else {
				return true;
			}
		}

		inline bool outputWidthIsMax() const
		{
			auto w = std::max(cell->getParam(ID::A_WIDTH).as_int(), cell->getParam(ID::B_WIDTH).as_int());
			return w <= cell->getParam(ID::Y_WIDTH).as_int();
		}

		inline bool boolOutput() const { return cell->getParam(ID::Y_WIDTH).as_int() == 1; }

		inline void assertEqualOperandWidth() const
		{
			if (!equalOperandWidth()) {
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
			assertEqualOperandWidth();
			assertEqualSigns();
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
				log_error("Expected maximum width on output in cell %s of type %s\n", RTLIL::id2cstr(cell->name), log_id(cell->type));
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

	std::string sextConvert(const SigSpec &sig, int width)
	{
		log_assert(sig.size() <= width);
		if (sig.size() < width) {
			auto sign = def_wire.temp(1);
			auto orig = convert(sig);
			instr.SRL(sign, orig, def_const.get(Const(sig.size() - 1)));

			auto pos_mask = Const(State::S0, width);
			auto neg_mask = Const(State::S0, width);
			for (int i = sig.size(); i < width; i++) {
				neg_mask.bits[i] = State::S1;
			}

			auto resolved_mask = def_wire.temp(width);
			instr.MUX(resolved_mask, sign, def_const.get(pos_mask), def_const.get(neg_mask));
			auto sextended = def_wire.temp(width);
			auto zextended = def_wire.temp(width);
			instr.PADZERO(zextended, orig, width);
			instr.OR(sextended, zextended, resolved_mask);
			return sextended;
		} else {
			return convert(sig);
		}
	}
	// converts a cell (most likely a $eq or $neq) into SEQ
	inline void mkSeq(Cell *cell, const std::string &res)
	{

		auto checker = CellSanityChecker(cell);
		checker.assertEqualSigns();

		auto a_signed = cell->getParam(ID::A_SIGNED).as_bool();
		auto a_width = cell->getParam(ID::A_WIDTH).as_int();
		auto b_width = cell->getParam(ID::B_WIDTH).as_int();
		auto w = std::max(a_width, b_width);
		if (a_signed) {
			instr.SEQ(res, sextConvert(cell->getPort(ID::A), w), sextConvert(cell->getPort(ID::B), w));
		} else {
			instr.SEQ(res, padConvert(cell->getPort(ID::A), w), padConvert(cell->getPort(ID::B), w));
		}
	}

	inline void mkCompare(Cell *cell, const std::string &res, bool less = true)
	{

		auto checker = CellSanityChecker(cell);

		checker.assertEqualSigns();

		auto a_signed = cell->getParam(ID::A_SIGNED).as_bool();

		auto op1_port = less ? ID::A : ID::B;
		auto op2_port = less ? ID::B : ID::A;

		auto w = std::max(cell->getParam(ID::A_WIDTH).as_int(), cell->getParam(ID::B_WIDTH).as_int());

		if (a_signed) {

			instr.SLTS(res, sextConvert(cell->getPort(op1_port), w), sextConvert(cell->getPort(op2_port), w));
		} else {
			instr.SLT(res, padConvert(cell->getPort(op1_port), w), padConvert(cell->getPort(op2_port), w));
		}
	}

#define UNARY_OP_HEADER                                                                                                                              \
	auto a_width = cell->getParam(ID::A_WIDTH).as_int();                                                                                         \
	auto y_width = cell->getParam(ID::Y_WIDTH).as_int();                                                                                         \
	auto a_name = convert(cell->getPort(ID::A));                                                                                                 \
	auto y_name = convert(cell->getPort(ID::Y));

	std::string extendOrSlice(const SigSpec &sig, int width, bool is_signed)
	{
		auto sig_size = sig.size();
		if (sig_size <= width && is_signed) {
			return sextConvert(sig, width);
		} else if (sig_size <= width && !is_signed) {
			return padConvert(sig, width);
		} else {
			return convert(sig.extract(0, width));
		}
	}
	std::pair<std::string, std::string> convertBinaryOperands(Cell *cell)
	{

		auto checker = CellSanityChecker(cell);
		auto y_width = cell->getParam(ID::Y_WIDTH).as_int();
		auto a_signed = cell->getParam(ID::A_SIGNED).as_bool();
		checker.assertEqualSigns();
		return std::make_pair(extendOrSlice(cell->getPort(ID::A), y_width, a_signed), extendOrSlice(cell->getPort(ID::B), y_width, a_signed));
	}
	// convert a cell into instructions
	void convert(Cell *cell)
	{

		instr.comment(cell);
		if (cell->type == ID($not)) {
			auto width = cell->getParam(ID::A_WIDTH).as_int();
			log_assert(width == cell->getParam(ID::Y_WIDTH).as_int());
			instr.XOR(convert(cell->getPort(ID::Y)), convert(cell->getPort(ID::A)), def_const.get(Const(State::S1, width)));
		} else if (cell->type == ID($pos)) {
			UNARY_OP_HEADER
			auto a_signed = cell->getParam(ID::A_SIGNED).as_bool();
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

			auto y_width = cell->getParam(ID::Y_WIDTH).as_int();
			auto a_width = cell->getParam(ID::A_WIDTH).as_int();
			log_assert(a_width <= y_width);
			auto a_name = sextConvert(cell->getPort(ID::A), y_width);
			auto y_name = convert(cell->getPort(ID::Y));
			auto bits_flipped = def_wire.temp(y_width);
			instr.XOR(bits_flipped, a_name, def_const.get(Const(State::S1, y_width)));
			instr.ADD(y_name, bits_flipped, def_const.get(Const(1, y_width)));

		} else if (cell->type == ID($reduce_and)) {
			UNARY_OP_HEADER
			auto all_ones = def_const.get(Const(State::S1, a_width));
			// log_assert(!a_signed);
			if (y_width == 1) {
				instr.SEQ(y_name, a_name, all_ones);
			} else {
				auto b0 = def_wire.temp(1);
				instr.SEQ(b0, a_name, all_ones);
				instr.PADZERO(y_name, b0, y_width);
			}
		} else if (cell->type == ID($reduce_or) || cell->type == ID($reduce_bool)) {
			UNARY_OP_HEADER
			// log_assert(!a_signed);
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

			// log_assert(!a_signed);
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
			// log_assert(!a_signed);

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

			if (y_width == 1) {
				instr.SEQ(y_name, a_name, def_const.get(Const(State::S0, a_width)));
			} else {
				auto temp = def_wire.temp(1);
				instr.SEQ(temp, a_name, def_const.get(Const(State::S0, a_width)));
				instr.PADZERO(y_name, temp, y_width);
			}
		} // end of unary ops
		  // begin binary ops
		else if (cell->type == ID($and)) {
			auto operands = convertBinaryOperands(cell);
			instr.AND(convert(cell->getPort(ID::Y)), operands.first, operands.second);
		} else if (cell->type == ID($or)) {
			auto operands = convertBinaryOperands(cell);
			instr.OR(convert(cell->getPort(ID::Y)), operands.first, operands.second);
		} else if (cell->type == ID($xor)) {
			auto operands = convertBinaryOperands(cell);
			instr.XOR(convert(cell->getPort(ID::Y)), operands.first, operands.second);
		} else if (cell->type == ID($xnor)) {
			auto operands = convertBinaryOperands(cell);
			auto y_width = cell->getParam(ID::Y_WIDTH).as_int();
			auto temp = def_wire.temp(y_width);
			instr.XOR(temp, operands.first, operands.second);
			instr.XOR(convert(cell->getPort(ID::Y)), temp, def_const.get(Const(State::S1, y_width)));
		} else if (cell->type == ID($add)) {
			auto operands = convertBinaryOperands(cell);
			instr.ADD(convert(cell->getPort(ID::Y)), operands.first, operands.second);
		} else if (cell->type == ID($sub)) {
			auto operands = convertBinaryOperands(cell);
			instr.SUB(convert(cell->getPort(ID::Y)), operands.first, operands.second);
		} else if (cell->type == ID($mul)) {
			auto operands = convertBinaryOperands(cell);
			instr.MUL(convert(cell->getPort(ID::Y)), operands.first, operands.second);
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

		} else if (cell->type == ID($logic_and) || cell->type == ID($logic_or)) {

			// checker.assertBothUnsigned();

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
		} else if (cell->type.in(ID($sshl), ID($shl))) {

			log_assert(cell->getParam(ID::B_SIGNED).as_bool() == false);
			auto a_signed = cell->getParam(ID::A_SIGNED).as_bool();
			auto a_width = cell->getParam(ID::A_WIDTH).as_int();
			auto y_width = cell->getParam(ID::Y_WIDTH).as_int();
			auto w = std::max(a_width, y_width);
			if (a_signed) {
				instr.SLL(convert(cell->getPort(ID::Y)), sextConvert(cell->getPort(ID::A), w), convert(cell->getPort(ID::B)));
			} else {
				instr.SLL(convert(cell->getPort(ID::Y)), padConvert(cell->getPort(ID::A), w), convert(cell->getPort(ID::B)));
			}

		} else if (cell->type == ID($shr)) {

			log_assert(cell->getParam(ID::B_SIGNED).as_bool() == false);
			auto a_signed = cell->getParam(ID::A_SIGNED).as_bool();
			auto a_width = cell->getParam(ID::A_WIDTH).as_int();
			auto y_width = cell->getParam(ID::Y_WIDTH).as_int();
			auto w = std::max(a_width, y_width);
			if (a_signed) {

				instr.SRL(convert(cell->getPort(ID::Y)), sextConvert(cell->getPort(ID::A), w), convert(cell->getPort(ID::B)));

			} else {
				instr.SRL(convert(cell->getPort(ID::Y)), padConvert(cell->getPort(ID::A), w), convert(cell->getPort(ID::B)));
			}

		} else if (cell->type == ID($sshr)) {

			log_assert(cell->getParam(ID::B_SIGNED).as_bool() == false);
			auto a_signed = cell->getParam(ID::A_SIGNED).as_bool();
			auto a_width = cell->getParam(ID::A_WIDTH).as_int();
			auto y_width = cell->getParam(ID::Y_WIDTH).as_int();
			auto y_name = convert(cell->getPort(ID::Y));
			auto w = std::max(a_width, y_width);
			if (a_signed) {
				if (y_width < w) {
					auto widened = def_wire.temp(w);
					instr.SRA(widened, convert(cell->getPort(ID::A)), convert(cell->getPort(ID::B)));
					instr.SLICE(y_name, widened, 0, y_width);
				} else {
					instr.SRA(y_name, sextConvert(cell->getPort(ID::A), w), convert(cell->getPort(ID::B)));
				}
			} else {
				instr.SRL(y_name, padConvert(cell->getPort(ID::A), w), convert(cell->getPort(ID::B)));
			}

		} else if (cell->type == ID($shift) || cell->type == ID($shiftx)) {

			// although the following implementation maybe correct, I don't have
			// have a way of testing this so I am going to throw an error so that
			// anybody you finds a circuit that creates this cell has to deal
			// with validating the implementation
			log_warning("Can not generate code for %s.%s of type %s\n", log_id(mod), log_id(cell), log_id(cell->type));
			log_error("Our Yosys' code generator may have a valid translation for $shift and $shiftx but up to know I did not know of "
				  "any circuit that results in these cell. Please submit your Verilog files as a git issue so we can patch and test "
				  "our compiler.");
			auto b_signed = cell->getParam(ID::B_SIGNED).as_bool();

			auto a_name = convert(cell->getPort(ID::A));
			auto y_name = convert(cell->getPort(ID::Y));
			auto b_name = convert(cell->getPort(ID::B));

			auto b_width = cell->getParam(ID::B_WIDTH).as_int();
			auto y_width = cell->getParam(ID::Y_WIDTH).as_int();

			if (!b_signed) {
				instr.SRL(y_name, a_name, b_name);
			} else {
				// get the sign bit
				auto shift_bits = bitLength(y_width);
				auto b_sig = cell->getPort(ID::B);
				auto shift_sig = b_sig.extract(0, shift_bits);
				auto shift_wire = convert(shift_sig);

				auto sel = def_wire.temp(1);
				instr.SRL(sel, b_name, def_const.get(Const(b_width - 1, 32)));
				auto r1 = def_wire.temp(y_width);
				auto r2 = def_wire.temp(y_width);
				// perform a right logical shift if B is positive
				instr.SRL(r1, a_name, b_name);
				// or do a left logical shift if B is negative but
				// use the positive representation of B
				auto b_not = def_wire.temp(b_width);
				instr.XOR(b_not, b_name, def_const.get(Const(State::S1, b_width)));
				auto b_neg = def_wire.temp(b_width);
				instr.ADD(b_neg, b_not, def_const.get(Const(1, b_width)));
				instr.SLL(r2, a_name, b_neg);
				instr.MUX(y_name, sel, r1, r2);
			}

		} else if (cell->type == ID($memwr_v2)) {

			memory_writes[IdString(cell->getParam(ID::MEMID).decode_string())].push_back(cell);

		} else if (cell->type == ID($memrd_v2)) {

			memory_reads[IdString(cell->getParam(ID::MEMID).decode_string())].push_back(cell);

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

	void convertMemoryCell(const IdString &memid)
	{

		auto mem = mod->memories[memid];
		auto mem_addr_bits = bitLength(mem->size + mem->start_offset - 1);

		bool has_lower_bound_check = (mem->start_offset != 0);

		auto hasUpperBoundCheck = [&](Cell *mcell) -> bool {
			int max_address = (1 << mcell->getPort(ID::ADDR).size());
			int max_mem_addr = (1 << mem_addr_bits);
			bool has_upper_bound_check = (mem->size != (1 << mem_addr_bits)) || (max_address > max_mem_addr);
			return has_upper_bound_check;

		};
		auto mem_name = def_wire.getMemory(mem);

		auto addrFromLowerBound = [&](Cell *mcell) -> std::string {
			int sig_addr_bits = mcell->getPort(ID::ADDR).size();
			auto cell_addr = convert(mcell->getPort(ID::ADDR));
			if (has_lower_bound_check) {
				auto real_addr = def_wire.temp(sig_addr_bits);
				instr.SUB(real_addr, cell_addr, def_const.get(Const(mem->start_offset, sig_addr_bits)));
				return real_addr;
			} else {
				return cell_addr;
			}
		};
		auto createBoundOk = [&](const std::string &real_addr, int sig_addr_bits) -> std::string {
			instr.comment("Bound check");
			auto max_width = std::max(sig_addr_bits, mem_addr_bits);
			auto addr_padded = def_wire.temp(max_width + 1);
			auto bound_ok = def_wire.temp(1);
			instr.PADZERO(addr_padded, real_addr, max_width + 1);
			instr.SLT(bound_ok, addr_padded, def_const.get(Const(mem->size, max_width + 1)));
			return bound_ok;
		};

		auto convertReadData = [this](Cell *cell) {
			instr.comment("memory internal dff");
			bool clocked = cell->getParam(ID::CLK_ENABLE).as_bool();
			const auto data_name = convert(cell->getPort(ID::DATA));
			if (clocked) {

				const auto current = def_wire.getCurrent(cell);
				const auto next = def_wire.getNext(cell);
				// instr.MOV(data_name, current);
				auto has_arst = !cell->getPort(ID::ARST).is_fully_zero();
				auto has_rst = !cell->getPort(ID::SRST).is_fully_zero();
				auto has_dyn_en = !cell->getPort(ID::EN).is_fully_ones();
				log_assert(!(has_arst && has_rst)); // can have only one of them!
				// here we may be creating dead wires, but that's fine for now...
				std::string loaded_value = next;
				auto port_width = cell->getParam(ID::WIDTH).as_int();
				if (has_arst) {
					loaded_value = def_wire.temp(port_width);
					// we need to MUX the next and current value
					auto arst_name = convert(cell->getPort(ID::ARST));
					instr.MUX(data_name, arst_name, current, def_const.get(cell->getParam(ID::ARST_VALUE)));
					if (has_dyn_en) {
						instr.comment("adffe");
						auto loaded_en = def_wire.temp(port_width);
						instr.MUX(loaded_en, convert(cell->getPort(ID::EN)), current, loaded_value);
						instr.MUX(next, convert(cell->getPort(ID::ARST)), loaded_en,
							  def_const.get(cell->getParam(ID::ARST_VALUE)));

					} else {
						instr.comment("adff");
						instr.MUX(next, convert(cell->getPort(ID::ARST)), loaded_value,
							  def_const.get(cell->getParam(ID::ARST_VALUE)));
					}
					return loaded_value;
				} else if (has_rst) {
					instr.MOV(data_name, current);
					loaded_value = def_wire.temp(port_width);
					auto ce_override = cell->getParam(ID::CE_OVER_SRST).as_bool();
					auto srst_name = convert(cell->getPort(ID::SRST));
					auto srst_value = def_const.get(cell->getParam(ID::SRST_VALUE));
					if (has_dyn_en && ce_override) {
						// SRST only works if EN
						instr.comment("sdffce");
						auto loaded_rst = def_wire.temp(port_width);
						instr.MUX(loaded_rst, srst_name, loaded_value, srst_value);
						instr.MUX(next, convert(cell->getPort(ID::EN)), current, loaded_rst);
					} else if (has_dyn_en && !ce_override) {
						instr.comment("sdffe");
						auto loaded_en = def_wire.temp(port_width);
						instr.MUX(loaded_en, convert(cell->getPort(ID::EN)), current, loaded_value);
						instr.MUX(next, srst_name, loaded_en, srst_value);
					} else {
						instr.comment("sdff");
						log_assert(!has_dyn_en);
						instr.MUX(next, srst_name, loaded_value, srst_value);
					}
				} else if (has_dyn_en) {
					instr.comment("dffe");
					instr.MOV(data_name, current);
					loaded_value = def_wire.temp(port_width);
					instr.MUX(next, convert(cell->getPort(ID::EN)), current, loaded_value);

				} else {
					instr.MOV(data_name, current);
					loaded_value = next;
				}
				instr.comment(stringf("// uses %s", loaded_value.c_str()));
				return loaded_value;
			} else {
				// easy case; none of the nonsense of dealing with a register
				log_assert(cell->getPort(ID::EN).is_fully_ones());
				log_assert(cell->getPort(ID::SRST).is_fully_zero());
				log_assert(cell->getPort(ID::ARST).is_fully_zero());

				return data_name;
			}
		};

		auto write_operations = memory_writes[memid];
		// handle write operatations first and record their order
		// sort the write by the port id
		std::sort(write_operations.begin(), write_operations.end(),
			  [](Cell *cell1, Cell *cell2) { return cell1->getParam(ID::PORTID).as_int() < cell2->getParam(ID::PORTID).as_int(); });
		std::vector<int> write_port_order;

		int order = 1;
		for (auto wr_cell : write_operations) {
			log("Handling STORE instructions for %s (port %d)\n", log_id(wr_cell), wr_cell->getParam(ID::PORTID).as_int());

			log_assert(wr_cell->getParam(ID::CLK_ENABLE).as_bool());
			log_assert(wr_cell->getParam(ID::WIDTH).as_int() == mem->width);
			// log_assert(wr_cell->getParam(ID::ABITS).as_int() == addr_bits);
			auto sig_addr_bits = wr_cell->getParam(ID::ABITS).as_int();
			// check the EN signal, if the EN signal is not a fully repeated bit
			// pattern then create sequences of loads followed by stores to
			// emulate "bit-strobes".

			auto en_sig = wr_cell->getPort(ID::EN);
			auto pattern = manticore::getRepeats(en_sig, sigmap);

			bool is_full_repeat = pattern.size() == 1;

			auto addr_name = addrFromLowerBound(wr_cell);

			if (is_full_repeat) {

				instr.emit(sourceInfo(wr_cell));
				const std::string predicate = convert(SigSpec(pattern.front().bit));
				auto wdata_name = convert(wr_cell->getPort(ID::DATA));
				// easy case
				auto checked_predicate = predicate;
				if (hasUpperBoundCheck(wr_cell)) {
					// has bound check
					auto bound_ok = createBoundOk(addr_name, sig_addr_bits);
					checked_predicate = def_wire.temp(1);
					instr.AND(checked_predicate, predicate, bound_ok);
				}

				instr.STORE(wdata_name, addr_name, checked_predicate, mem_name, order++);

			} else if (!en_sig.is_fully_ones() && !is_full_repeat) {
				// load the original value
				auto original_value = def_wire.temp(mem->width);
				instr.LOAD(original_value, addr_name, mem_name, order++);
				auto write_word = convert(wr_cell->getPort(ID::DATA));
				auto createMask = [&](const manticore::RepeatPattern &p, bool write_data_mask = false) -> Const {
					auto mask = Const(write_data_mask ? State::S0 : State::S1, mem->width);
					for (int i = p.offset; i < p.offset + p.width; i++) {
						mask.bits[i] = write_data_mask ? State::S1 : State::S0;
					}
					return mask;
				};
				for (const auto &en_part : pattern) {

					if (en_part.bit.is_wire() || en_part.bit.data == State::S1) {
						auto load_mask = createMask(en_part, false);
						auto store_mask = createMask(en_part, true);
						auto wdata = def_wire.temp(mem->width);
						auto rdata = def_wire.temp(mem->width);
						instr.AND(wdata, write_word, def_const.get(store_mask));
						instr.AND(rdata, original_value, def_const.get(load_mask));
						auto combined_word = def_wire.temp(mem->width);
						instr.OR(combined_word, wdata, rdata);
						instr.emit(sourceInfo(wr_cell));
						const std::string pred = convert(SigSpec(en_part.bit));
						auto checked_pred = pred;
						if (hasUpperBoundCheck(wr_cell)) {
							// has bound check
							auto bound_ok = createBoundOk(addr_name, sig_addr_bits);
							checked_pred = def_wire.temp(1);
							instr.AND(checked_pred, pred, bound_ok);
						}
						instr.STORE(combined_word, addr_name, checked_pred, mem_name, order++);

					} else {

						// either we have en_part.bit.data == State::S0 or State::Sx;
						// so we don't do anything, i.e., we don't store anything
					}
				}
			} else {
				log_error("Unhandled cell %s!\n", log_id(wr_cell));
			}
			log_assert(GetSize(write_port_order) == wr_cell->getParam(ID::PORTID).as_int());
			write_port_order.push_back(order);
		}

		auto read_operations = memory_reads[memid];

		for (auto rd_cell : read_operations) {

			auto transparency_mask = rd_cell->getParam(ID::TRANSPARENCY_MASK).bits;
			order = 0;
			for (int port_ix = 0; port_ix < GetSize(transparency_mask); port_ix++) {
				if (transparency_mask[port_ix] == State::S1) {
					order = std::max(order, write_port_order[port_ix]);
				}
			}

			if (order > 0) {
				log_assert(!rd_cell->getParam(ID::TRANSPARENCY_MASK).is_fully_zero());
				log_assert(rd_cell->getParam(ID::COLLISION_X_MASK).is_fully_zero());
			} else {
				// we may have write collision and we model the behavior as
				// read-then-write (i.e., order loads before stores)
			}

			log_assert(rd_cell->getParam(ID::WIDTH).as_int() == mem->width);
			auto sig_addr_bits = rd_cell->getParam(ID::ABITS).as_int();



			auto addr_name = addrFromLowerBound(rd_cell);
			auto rdata_name = convertReadData(rd_cell); // handle internal registers
			instr.emit(sourceInfo(rd_cell));
			if (!hasUpperBoundCheck(rd_cell)) {
				instr.LOAD(rdata_name, addr_name, mem_name, order);
				continue;
			}
			// most memories don't require bound checking since their address is guaranteed to be within bounds
			// but now that we have memory that has bound checks we need to explicitly handle it. Note that
			// Yosys' memory_memx pass may do the job, but it adds the check even if it is unnecessary! So we have
			// to handle it ourselves
			auto rd_width = rd_cell->getParam(ID::WIDTH).as_int();
			auto raw_rd_name = def_wire.temp(rd_width);
			instr.LOAD(raw_rd_name, addr_name, mem_name, order);
			// note that we still use the potentially out-of-range address so that
			// we allow Manticore to emit a warning if it wants but we set the final
			// loaded value to zero if the bound check fails. Though manticore
			// could only issue a warning for upper bound errors not lower bound
			// ones since we are subtracting the offset from the address... also
			// for store we don't do the same...
			auto bound_ok = createBoundOk(addr_name, sig_addr_bits);
			instr.MUX(rdata_name, bound_ok, def_const.get(Const(0, rd_width)), raw_rd_name);
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

		for (const auto &m : mod->memories) {
			convertMemoryCell(m.first);
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