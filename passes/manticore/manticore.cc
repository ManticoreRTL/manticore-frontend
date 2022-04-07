#include "kernel/yosys.h"
#include <algorithm>
#include <list>
USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct Manticore : public Pass {

	Manticore() : Pass("manticore", "Run all manticore related pass required before generating code") {}

	void help() override
	{

		//   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
		log("\n");
		log("    manticore_init [-top MODULE] [-track FILE]\n");
		log("\n");
	}

	template <typename T> struct Optional final {
		T value;
		bool vld;
		Optional() : vld(false) {}
		Optional(const T &value) : value(value), vld(true) {}
		inline bool empty() const { return !vld; }
		inline bool nonEmpty() const { return vld; }
	};
	template <typename T> inline Optional<T> Some(const T &value) { return Optional<T>(value); }

	struct Config {
		Optional<std::string> top_name;
		Optional<std::string> track_file;

		inline bool allSet() const { return top_name.nonEmpty() && track_file.nonEmpty(); }
	};

	void parseArgs__(std::list<std::string> &args, Config &cfg)
	{
		if (args.empty() == false) {
			auto front = args.front();

			args.pop_front();
			if (front == "-track") {
				if (!args.empty()) {
					cfg.track_file = Some(args.front());
					args.pop_front();
					parseArgs__(args, cfg);
				} else {
					log_error("Expected file name after -track!");
				}
			} else if (front == "-top") {
				if (!args.empty()) {
					cfg.top_name = Some(args.front());
					args.pop_front();
					parseArgs__(args, cfg);
				}
			}
		}
	}
	Config parseArgs(const std::vector<std::string> &args)
	{
		if (args.size() == 1) {
			log_error("Invalid number of arguments, use help %s to se the list of arguments", args[0].c_str());
		}
		Config cfg;
		std::list<std::string> arg_list(args.begin() + 1, args.end());

		parseArgs__(arg_list, cfg);

		if (cfg.top_name.empty()) {
			log_error("Please provide the top module!");
		}
		return cfg;
	}
	void execute(std::vector<std::string> args, Design *design) override
	{

		log_header(design, "Executing Manticore passes");

		auto cfg = parseArgs(args);
		Pass::call(design, stringf("hierarchy -top %s -check", cfg.top_name.value.c_str()));
		Pass::call(design, "proc");
		if (cfg.track_file.nonEmpty()) {
			Pass::call(design, stringf("manticore_init -track %s", cfg.track_file.value.c_str()));
		} else {
			Pass::call(design, "manticore_init");
		}
        Pass::call(design, "opt");
		Pass::call(design, "manticore_memory");
        Pass::call(design, "flatten");
		Pass::call(design, "manticore_meminit");
		Pass::call(design, "opt");


	}
} Manticore;

PRIVATE_NAMESPACE_END