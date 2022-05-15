
#include "passes/manticore/manticore_clock.h"


USING_YOSYS_NAMESPACE

namespace manticore
{



OptionalClock checkClock(Design *design, Module *mod, const OptionalClock &assumed_clock)
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
				if (found_clk.nonEmpty() && other_found_clock.nonEmpty()) {
                    if (other_found_clock.clk->port_output) {
                        log_error("conflicting clock %s.%s.%s", log_id(mod), log_id(cell), log_id(other_found_clock.clk->name));
                    }
                    if(other_found_clock.clk->port_input && cell->getPort(other_found_clock.clk->name).as_wire() != found_clk.clk) {
                        log_error("different clock %s.%s.%s", log_id(mod), log_id(cell), log_id(other_found_clock.clk->name));
                    }
                    if (!other_found_clock.clk->port_input) {
                        log_error("stranded clock %s.%s.%s", log_id(mod), log_id(cell), log_id(other_found_clock.clk->name));
                    }
				}
                if (found_clk.empty()) {
                    found_clk = other_found_clock;
                }
			}
		}
	}

	return found_clk;
}

}; // namespace manticore
