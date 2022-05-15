#include "kernel/yosys.h"
#include "passes/manticore/manticore_clock.h"
#include <fstream>
USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct ManticoreTb : public Pass {

	ManticoreTb() : Pass("manticore_tb", "generate a simple test bench for manticore") {}

	void help() override
	{

		//   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
		log("\n");
		log("    test_autotb [options] [filename]\n");
		log("\n");
		log("Generate a testbench that wraps a verilog module.\n");
		log("\n");
		log("    -n <int>\n");
		log("        number of iterations the test bench should run (default = 1000)\n");
		log("\n");
		log("    -seed <int>\n");
		log("        seed used for pseudo-random number generation (default = 12345678).\n");
		log("    -drive-clock\n");
		log("        create testbench module with its clock being automatically driven. Use\n");
		log("        this option if you wish to verify the design using iverilog.\n");
		log("\n");
	}

	void execute(std::vector<std::string> args, RTLIL::Design *design) override
	{

		int num_iter = 1000;
		int seed = 12345678;
		bool drive_clock = false;
		log_header(design, "Auto-generating random testbench");
		int argidx;
		std::string filename;
		for (argidx = 1; argidx < GetSize(args); argidx++) {
			if (args[argidx] == "-n" && argidx + 1 < GetSize(args)) {
				num_iter = atoi(args[++argidx].c_str());
				continue;
			}
			if (args[argidx] == "-seed" && argidx + 1 < GetSize(args)) {
				seed = atoi(args[++argidx].c_str());
				continue;
			}
			if (args[argidx] == "-drive-clock") {
				drive_clock = true;
				continue;
			}
			if (args[argidx].compare(0, 1, "-") == 0 && args[argidx] != "-") {
				cmd_error(args, argidx, "Invalid option!");
			}
			if (args[argidx].compare(0, 1, "-") != 0) {
				filename = args[argidx];
				continue;
			}
			break;
		}

		auto found_clock = manticore::checkClock(design, design->top_module(), manticore::OptionalClock());

		auto mod = design->top_module();

		// create a reg for every input signal
		// create a wire for every output signal
		// drive the input signals
		// print the output signals


	}

} ManticoreTb;

PRIVATE_NAMESPACE_END
