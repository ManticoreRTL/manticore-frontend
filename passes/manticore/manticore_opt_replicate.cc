#include "kernel/modtools.h"
#include "kernel/yosys.h"
#include "passes/manticore/manticore_utils.h"
#include <algorithm>
#include <fstream>
#include <queue>
#include <sstream>
#include <vector>
USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct MantcoreOptimizeBitReplication : public Pass {

	MantcoreOptimizeBitReplication()
	    : Pass("manticore_opt_replicate", "Replace SigSpecs that have replicated bit patterns with multiplexers driven by constants"){};
	void help() override
	{
		log("\n");
		log("    manticore_opt_replicate\n");
		log("\n");
	}

	void execute(std::vector<std::string>, Design *design) override
	{
		log_header(design, "Executing Manticore Bit Replication Optimization\n");

		for (auto mod : design->modules()) {
			transform(mod);
		}
	}

	void transform(Module *mod)
	{

		auto sigmap = SigMap(mod);

		int name_index = 0;
		auto freshName = [&](const std::string &prefix) { return mod->uniquify(stringf("$%s$", prefix.c_str()), name_index); };
		auto convertRhs = [&](const SigSpec &rhs) -> SigSpec {
			auto patterns = manticore::getRepeats(rhs, sigmap);
			SigSpec new_rhs;
			for (const auto &pat : patterns) {
				if (pat.width > 1) {
					if (pat.bit.is_wire()) {
						auto new_wire = mod->addWire(freshName("rep_wire"), pat.width);
						mod->addMux(freshName("rep_mux"), SigSpec(State::S0, pat.width), SigSpec(State::S1, pat.width),
							    SigSpec(pat.bit), SigSpec(new_wire));
						new_rhs.append(new_wire);
					} else {
						// bit is constant
						new_rhs.append(SigSpec(pat.bit.data, pat.width));
					}
				} else {
					// single-bit not repeated
					new_rhs.append(pat.bit);
				}
			}
			return new_rhs;
		};

		// create a copy of all the cells and then iterate over it
		// we can not iterate over the original ones because we'll be adding
		// cells midway and we do not want to invalidate the iterator
		std::vector<Cell *> cells = mod->cells();
		for (auto cell : cells) {
			for (auto &con : cell->connections()) {
				auto port_name = con.first;
				if (cell->type == ID($memwr_v2) && port_name == ID::EN) {
					// we want to keep the memory write enables as they are
					// because we deal with them at code generation time
					continue;
				}
				if (cell->input(port_name)) {
					log("checking port %s of %s\n", log_id(port_name), log_id(cell->name));
					auto rhs = con.second;
					auto new_rhs = convertRhs(rhs);
					cell->setPort(port_name, new_rhs);
				}
			}
		}

		std::vector<SigSig> new_connections;
		// go through all the connections and replace repeated bit patterns with
		// multiplexer with two constant inputs. This will lead to better manticore
		// assembly code because we can do any bit-repeat pattern with a single
		// mux. If we don't do this we will end up with a tree of contatenations
		// which are usually implemented using shift and or operations.
		for (const auto &con : mod->connections()) {
			auto rhs = con.second;
			auto new_rhs = convertRhs(rhs);
			new_connections.emplace_back(con.first, new_rhs);
		}
		mod->new_connections(new_connections);
	};

} MantcoreOptimizeBitReplication;

PRIVATE_NAMESPACE_END