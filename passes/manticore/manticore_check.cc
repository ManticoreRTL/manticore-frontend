#include "kernel/modtools.h"
#include "kernel/yosys.h"
USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct ManticoreCheck : public Pass {

	ManticoreCheck() : Pass("manticore_check", "Check for basic interfaces in t") {}

	void help() override
	{
		//   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
		log("\n");
		log("    manticore_check");
		log("Check the manticore workload for validity, for instance the existence of a unique clock");
		log("and lack of asynchronous reset register or latches");
		log("\n");
	}

	struct OptionalClock {
		Wire *clk;
		bool polarity;
		OptionalClock() : clk(nullptr), polarity(false) {}
		OptionalClock(Wire *clk, bool p) : clk(clk), polarity(p) {}

		inline bool operator==(const OptionalClock &other) const { return polarity == other.polarity && clk == other.clk; }
		inline bool operator!=(const OptionalClock &other) const { return !operator==(other); }
		inline bool nonEmpty() const { return clk != nullptr; }
		inline bool empty() const { return clk == nullptr; }
	};
	OptionalClock checkClock(Design *design, Module *mod, const OptionalClock &assumed_clock) const
	{

		SigMap sigmap(mod);
		OptionalClock found_clk;
		if (assumed_clock.nonEmpty()) {
			found_clk = OptionalClock(sigmap(SigSpec(assumed_clock.clk)).as_wire(), assumed_clock.polarity);
		}
		auto cellClock = [&sigmap](const Cell *cell) -> OptionalClock {
			auto s = sigmap(cell->getPort(ID::CLK));
			auto p = cell->getParam(ID::CLK_POLARITY).as_bool();
			return OptionalClock(s.as_wire(), p);
		};
		for (const auto &cell : mod->cells()) {
			if (cell->type == ID($dff)) {
				auto cell_clk = cellClock(cell);
				if (found_clk.empty()) {
					found_clk = cell_clk;
				} else if (found_clk != cell_clk) {
					log_error("The found clock %s is different from the used clock %s in cell %s in module %s\n",
						  RTLIL::id2cstr(found_clk.clk->name), RTLIL::id2cstr(cell_clk.clk->name), RTLIL::id2cstr(cell->name),
						  RTLIL::id2cstr(mod->name));
				}
			} else if (cell->type.in(ID($adff), ID($adffe), ID($dffsr), ID($dffsre), ID($dffs))) {
				log_error("asynchronous cell %s in module %s not supported!", RTLIL::id2cstr(cell->name), RTLIL::id2cstr(mod->name));
			} else if (cell->type.in(ID($dffe), ID($dffsr), ID($dffsre))) {
				log_error("built-in reset/enable dff %s cell in module %s need to be converted to $dff!", RTLIL::id2cstr(cell->name),
					  RTLIL::id2cstr(mod->name));

			} else if (cell->type.in(ID($dlatch), ID($adlatch), ID($dlatchsr), ID($adffe), ID($aldff), ID($aldffe))) {
				log_error("latch cell %s in module %s is not supported!", RTLIL::id2cstr(cell->name), RTLIL::id2cstr(mod->name));
			} else {
				auto user_mod_found = design->module(cell->type);
				if (user_mod_found != NULL) {
					auto other_found_clock = checkClock(design, user_mod_found, found_clk);
					if (found_clk.nonEmpty() && other_found_clock != found_clk) {
						log_error("detected a different clock in instance %s of module %s", RTLIL::id2cstr(cell->name),
							  RTLIL::id2cstr(mod->name));
					}
				}
			}
		}

		return found_clk;
	}
	void execute(std::vector<std::string>, Design *design) override
	{
		log_header(design, "Executing Manticore Checker Pass\n");

		auto top = design->top_module();
		if (top == nullptr) {
			log_error("Expected a top module!");
		}

		auto found_clk = checkClock(design, top, OptionalClock());

		if (found_clk.empty()) {
			log_error("Could not detect the clock!");
		}

		auto sigmap = SigMap(top);
		log("Inferred %s as the clock\n", RTLIL::id2cstr(sigmap(found_clk.clk).as_wire()->name));
		auto has_input = false;
		for (const auto &w : top->wires()) {
			if (w->port_output) {
				log_error("Top module can not have output, only a single clock input!\n");
			} else if (w->port_input) {
				if (has_input) {
					log_error("Top module can only a have single single-bit input as the clock!");
				} else {
					has_input = true;
					if (w->width == 1) {
						if (sigmap(SigSpec(found_clk.clk)) != sigmap(SigSpec(w))) {
							log_error("Top module input clock is not the same as the inferred clock!");
						}
					} else {
						log_error("Top module can not have multiple inputs, only a single bit clock inputs is allowed\n");
					}
				}
			}
		}
	}
} ManticoreCheck;

PRIVATE_NAMESPACE_END