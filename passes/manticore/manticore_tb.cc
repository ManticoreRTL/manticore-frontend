#include "kernel/yosys.h"
#include "passes/manticore/manticore_clock.h"
#include <fstream>
USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN
static std::string id(std::string internal_id)
{
	const char *str = internal_id.c_str();
	bool do_escape = false;

	if (*str == '\\')
		str++;

	if ('0' <= *str && *str <= '9')
		do_escape = true;

	for (int i = 0; str[i]; i++) {
		if ('0' <= str[i] && str[i] <= '9')
			continue;
		if ('a' <= str[i] && str[i] <= 'z')
			continue;
		if ('A' <= str[i] && str[i] <= 'Z')
			continue;
		if (str[i] == '_')
			continue;
		do_escape = true;
		break;
	}

	if (do_escape)
		return "\\" + std::string(str) + " ";
	return std::string(str);
}

static std::string idx(std::string str)
{
	if (str[0] == '\\')
		return str.substr(1);
	return str;
}

static std::string idy(std::string str1, std::string str2 = std::string(), std::string str3 = std::string())
{
	str1 = idx(str1);
	if (!str2.empty())
		str1 += "_" + idx(str2);
	if (!str3.empty())
		str1 += "_" + idx(str3);
	return id(str1);
}

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
		log("        seed used for pseudo-random number generation (default = 87901212).\n");
		log("    -drive-clock\n");
		log("        create testbench module with its clock being automatically driven. Use\n");
		log("        this option if you wish to verify the design using iverilog.\n");
		log("\n");
	}

	void execute(std::vector<std::string> args, RTLIL::Design *design) override
	{

		int num_iter = 1000;
		int seed = 87901212;
		bool drive_clock = false;
		log_header(design, "Auto-generating random testbench\n");
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

		auto mod = design->top_module();

		auto found_clock = manticore::checkClock(design, mod, manticore::OptionalClock());

		if (found_clock.nonEmpty()) {
			log("Found clock %s.%s\n", log_id(mod), log_id(found_clock.clk->name));
		}

		// create a reg for every input signal
		// create a wire for every output signal
		// drive the input signals
		// print the output signals

		std::map<std::string, int> signal_in;  // map of input names to their width
		std::map<std::string, int> signal_out; // a map of output names to their width
		std::string mod_clock;
		std::stringstream tb;

		if (drive_clock) {
			tb << stringf("module Main();\n");
			tb << stringf("reg clk = 0;\n");
		} else {
			tb << stringf("module Main(input wire clk);\n");
		}

		tb << stringf("reg [7:0] rst_counter = 0;\n");
		tb << stringf("reg [31:0] iter = 0;\n");
		tb << stringf("reg [31:0] xorshift128_x = 123456789;\n");
		tb << stringf("reg [31:0] xorshift128_y = 362436069;\n");
		tb << stringf("reg [31:0] xorshift128_z = 521288629;\n");
		tb << stringf("reg [31:0] xorshift128_w = %u; // <-- seed value\n", seed);
		tb << stringf("reg [31:0] xorshift128_t;\n\n");
		tb << stringf("task xorshift128;\n");
		tb << stringf("begin\n");
		tb << stringf("\txorshift128_t = xorshift128_x ^ (xorshift128_x << 11);\n");
		tb << stringf("\txorshift128_x = xorshift128_y;\n");
		tb << stringf("\txorshift128_y = xorshift128_z;\n");
		tb << stringf("\txorshift128_z = xorshift128_w;\n");
		tb << stringf("\txorshift128_w = xorshift128_w ^ (xorshift128_w >> 19) ^ xorshift128_t ^ (xorshift128_t >> 8);\n");
		tb << stringf("end\n");
		tb << stringf("endtask\n\n");

		int count_ports = 0;
		for (auto wire : mod->wires()) {

			if (wire->port_output) {

				signal_out[idy("sig", mod->name.str(), wire->name.str())] = wire->width;
				tb << stringf("wire [%d - 1 : 0] %s;\n", wire->width, idy("sig", mod->name.str(), wire->name.str()).c_str());
				count_ports++;
			} else if (wire->port_input) {
				if (wire->width > 128) {
					log_warning("Signal %s.%s is too wide for test bench generation\n", log_id(mod), log_id(wire));
				}
				if (found_clock.nonEmpty() && wire->name == found_clock.clk->name) {
					mod_clock = idy("sig", mod->name.str(), wire->name.str());
					tb << stringf("wire %s;\n", mod_clock.c_str());
					tb << stringf("assign %s = clk;\n", mod_clock.c_str());
				} else {
					signal_in[idy("sig", mod->name.str(), wire->name.str())] = wire->width;
					tb << stringf("reg [%d - 1 : 0] %s;\n", wire->width, idy("sig", mod->name.str(), wire->name.str()).c_str());
				}
				count_ports++;
			}
		}

		// instantiate the module
		tb << stringf("%s %s(\n", id(mod->name.str()).c_str(), idy("uut", mod->name.str()).c_str());

		for (auto wire : mod->wires()) {
			if (wire->port_input || wire->port_output) {
				tb << stringf("\t.%s(%s)%s\n", id(wire->name.str()).c_str(), idy("sig", mod->name.str(), wire->name.str()).c_str(),
					      --count_ports ? "," : "");
			}
		}
		tb << stringf(");\n\n");

		if (drive_clock) {
			tb << stringf("initial begin clk = 0; forever #5 clk = !clk; end\n");
		}

		tb << stringf("task %s;\n", idy(mod->name.str(), "update_data").c_str());
		tb << "begin\n";
		for (auto inp : signal_in) {
			tb << stringf("\txorshift128;\n");
			tb << stringf("\t%s <= { xorshift128_x, xorshift128_y, xorshift128_z, xorshift128_w };\n", inp.first.c_str());
		}
		tb << "end\n";
		tb << "endtask\n";

		tb << stringf("task %s;\n", idy(mod->name.str(), "print_status").c_str());
		tb << stringf("begin\n");
		{

			std::stringstream fmt;
			std::stringstream vargs;
			fmt << "#OUT# ";

			for (const auto &inp : signal_in) {
				fmt << "%b ";
				vargs << inp.first << ", ";
			}
			fmt << "| ";
			for (const auto &outp : signal_out) {
				fmt << "%b ";
				vargs << outp.first << ", ";
			}
			tb << "\t$display(\"" << fmt.rdbuf() << " %d\", " << vargs.rdbuf() << "iter);\n";
		}
		tb << stringf("end\n");
		tb << "endtask\n";

		tb << stringf("task %s;\n", idy(mod->name.str(), "print_header").c_str());
		tb << stringf("begin\n");
		{
			std::stringstream fmt;
			char shortname = 'A';
			fmt << "#OUT# ";

			auto gen = [&](const std::pair<std::string, int> &port) {
				if (port.second == 1) {
					fmt << shortname << " ";
				} else {
					std::string prefix = std::string("-", port.second - 1);
					for (int i = 1; i < port.second; i++)
						fmt << '-';
					fmt << shortname << " ";
				}
				tb << stringf("\t$display(\"#OUT#\t%c\t%s\");\n", shortname, port.first.c_str());
				if (shortname == 'Z') {
					shortname = 'A';
				} else {
					shortname++;
				}
			};

			for (const auto &inp : signal_in) {
				gen(inp);
			}
			fmt << "| ";
			for (const auto &outp : signal_out) {
				gen(outp);
			}
			tb << "\t$display(\"" << fmt.rdbuf() << "\");\n";
		}
		tb << stringf("end\n");
		tb << stringf("endtask\n\n");

		tb << stringf("always @(posedge clk) begin\n");
		tb << stringf("\tif(rst_counter < 10) begin\n");
		tb << stringf("\t\trst_counter = rst_counter + 1;\n");
		tb << stringf("\t\titer = 0;\n");
		for (auto inp : signal_in) {
			tb << stringf("\t\t%s = 0;\n", inp.first.c_str());
		}
		tb << stringf("\tend else if (iter < %d) begin\n", num_iter);
		tb << stringf("\t\tif(iter[5:0] == 0)\n");
		tb << stringf("\t\t\t%s;\n", idy(mod->name.str(), "print_header").c_str());
		tb << stringf("\t\t%s;\n", idy(mod->name.str(), "print_status").c_str());
		tb << stringf("\t\t%s;\n", idy(mod->name.str(), "update_data").c_str());
		tb << stringf("\t\titer <= iter + 1;\n");
		tb << stringf("\tend else begin\n");
		tb << stringf("\t\t$finish;\n");
		tb << stringf("\tend\n");
		tb << stringf("end\n");

		tb << "endmodule\n";

		if (filename.empty()) {
			std::cout << tb.rdbuf() << std::endl;
		} else {
			std::ofstream fout(filename, std::ios::out);
			fout << tb.rdbuf() << std::endl;
			fout.close();
		}
	}

} ManticoreTb;

PRIVATE_NAMESPACE_END
