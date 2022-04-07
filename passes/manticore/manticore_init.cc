#include "kernel/yosys.h"
#include <fstream>


USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct Impl {
      public:
	using HierName = std::vector<std::string>;

	Impl(Design *design, const std::vector<HierName> tracked) : m_design(design), m_tracked(tracked)
	{

		log("Will track %lu names\n", m_tracked.size());
	}

	inline IdString asUserId(const std::string &name) { return IdString(std::string("\\") + name); }

	inline std::string flatten(const HierName &hname)
	{
		std::stringstream ss;
		for (const auto &token : hname) {
			ss << token << "/";
		}
		return ss.str();
	}
	void insertTrackAttribute(Module *mod, const size_t level)
	{

		for (const auto &h_name : m_tracked) {
			auto base_name = asUserId(h_name[level]);
			// log("Inspecting %s : %s\n", flatten(h_name).c_str(), h_name[level].c_str());
			if (h_name.size() == level + 1) { // reached the end of hierarchy
				if (auto w = mod->wire(base_name)) {
					// log("Setting attribute for %s\n", w->name.c_str());
					w->set_bool_attribute(ID::keep, true);
					w->set_string_attribute(ID::MANTICORE_TRACK_NAME, flatten(h_name));
				} else {
					log_error("Could not find signal %s", flatten(h_name).c_str());
				}
			} else if (auto found_cell = mod->cell(base_name)) { // still needs to go down
				if (auto next_module = m_design->module(found_cell->type)) {
					insertTrackAttribute(next_module, level + 1);
				} else {
					log_error("Undefined module %s\n", found_cell->type.c_str());
				}
			} else { // oops!
				log_error("Could not find instance %s in module %s\n", base_name.c_str(), mod->name.c_str());
			}
		}
	}
	void transform()
	{

		for (const auto &hname : m_tracked) {
			if (asUserId(hname[0]) != m_design->top_module()->name) {
				log_error("Invalid signal %s\n", flatten(hname).c_str());
			}
		}
		insertTrackAttribute(m_design->top_module(), 1);
	}

      private:
	Design *m_design;
	std::vector<HierName> m_tracked;
};

struct ManticoreInit : public Pass {

	ManticoreInit() : Pass("manticore_init", "Initialize the netlist for manticore code generation and consolidates memory initialization nodes into one") {}

	void help() override
	{
		//   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
		log("\n");
		log("    manticore_init [-track FILE]\n");
		log("\n");
		log("Initialize the design with an optional list of signals to keep track of. The pass\n");
		log("basically appends (* keep *) attributes to the requested signals so that \n");
		log("optimization passes will not delete them. Note that the pass does not flatten the\n");
		log("design to enable faster down-stream optimizations.");
	}

	template <typename T>
	struct Optional {
		T value;
		bool valid = false;
		Optional(const T& value): value(value), valid(true) {}
		Optional(): valid(false) {}
		Optional(const Optional<T>& other) {
			if (other.valid) {
				valid = true;
				value = other.value;
			} else {
				valid = false;
			}
		}
	};

	void execute(std::vector<std::string> args, Design *design) override
	{
		log_header(design, "Executing Manticore Initialization Pass\n");

		if (!(args.size() == 1 || args.size() == 3)) {
			log_error("Invalid number of arguments!");
		}

		std::vector<Impl::HierName> to_track;
		if (args.size() == 3) {
			if (args[1] == "-track") {
				log("Loading track file %s\n", args[2].c_str());

				auto mkHierName = [](const std::string &flat_name) -> Impl::HierName {
					size_t len = 1;
					size_t start = 0UL;
					size_t end = std::string::npos;
					std::vector<std::string> parts;
					do {
						end = flat_name.find('/', start);
						auto token = flat_name.substr(start, end - start);
						parts.push_back(token);
						start = end + len;

					} while (end != std::string::npos);
					return parts;
				};

				std::ifstream ifs(args[2], std::ios::in);
				std::string line;

				if (!ifs.is_open()) {
					log_error("Could not open track file!\n");
				}
				while (std::getline(ifs, line)) {
					to_track.emplace_back(mkHierName(line));
				}

			} else {
				log_error("invalid option %s\n", args[1].c_str());
			}
		}


		Impl worker(design, to_track);

		worker.transform();

	}

} ManticoreInit;

PRIVATE_NAMESPACE_END