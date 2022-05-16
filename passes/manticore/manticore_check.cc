#include "kernel/modtools.h"
#include "kernel/yosys.h"
#include "passes/manticore/manticore_clock.h"
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

	void execute(std::vector<std::string>, Design *design) override
	{
		log_header(design, "Executing Manticore Checker Pass\n");

		auto top = design->top_module();
		if (top == nullptr) {
			log_error("Expected a top module!");
		}

		auto found_clk = manticore::checkClock(design, top, manticore::OptionalClock());

		if (found_clk.empty()) {
			log_error("Could not detect the clock!");
		}

		auto sigmap = SigMap(top);
		log("Inferred %s as the clock\n", log_id(found_clk.clk));
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
		if (!has_input) {
			log_error("manticore Main module requires a single input as the clock!");
		}
	}
} ManticoreCheck;

PRIVATE_NAMESPACE_END