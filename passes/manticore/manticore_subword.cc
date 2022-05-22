#include "kernel/yosys.h"

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct ManticoreSubword : public Pass {

	ManticoreSubword() : Pass("manticore_subword", "remove subword assignment on cell outputs") {}
	void help() override
	{
		log("\n");
		log("    manticore_subword\n");
		log("\n");
	}
	void execute(std::vector<std::string>, Design *design) override
	{

		log_header(design, "Executing Manticore Subword assignment removal pass\n");

		for (auto mod : design->modules()) {
			transform(mod);
		}
	}

	// struct Subword {
	// 	Wire* wire;
	// 	int offset;
	// 	int width;
	// 	Subword(Wire* wire, int offset, int width) : wire(wire), offset(offset), width(width) {}
	// };
	void transform(Module *mod)
	{
		int name_index = 0;
		// auto freshName = [&](const std::string &prefix) { return mod->uniquify(stringf("$%s$", prefix.c_str()), name_index); };

		dict<Wire *, std::vector<std::pair<SigChunk, SigChunk>>> subwords;
		for (auto cell : mod->cells()) {
			for (auto &con : cell->connections()) {
				auto port_name = con.first;
				auto sig = con.second;
				if (cell->output(port_name) && !sig.is_wire()) {

					auto alias_wire = mod->addWire(NEW_ID_SUFFIX("subword_out"), sig.size());
					cell->setPort(port_name, alias_wire);

					log("Replacing %s.%s.%s => %s with %s\n", log_id(mod), log_id(cell), log_id(port_name), log_signal(sig),
					    log_signal(alias_wire));

					auto chunks = sig.chunks();
					int offset = 0;
					for (const auto &chunk : chunks) {

						if (chunk.is_wire()) {
							subwords[chunk.wire].emplace_back(chunk, SigChunk(alias_wire, offset, chunk.width));
						} else {
							log_error("%s.%s.%s => %s assigns to contant\n", log_id(mod), log_id(cell), log_id(port_name),
								  log_signal(sig));
						}
						offset += chunk.width;
					}
				}
			}
		}

		std::vector<SigSig> new_connections;

		for (auto con : mod->connections()) {

			auto lhs = con.first;
			if (lhs.is_wire()) {
				new_connections.push_back(con);
			} else {
				// connection assign to a subword
				auto alias_wire = mod->addWire(NEW_ID_SUFFIX("subword_lhs"), lhs.size());
				log("Replacing %s with %s\n", log_signal(lhs), log_signal(alias_wire));

				auto chunks = lhs.chunks();
				int offset = 0;
				for (const auto &chunk : chunks) {
					if (chunk.is_wire()) {
						subwords[chunk.wire].emplace_back(chunk, SigChunk(alias_wire, offset, chunk.width));
					} else {
						log_error("In %s connect %s %s assigns to contant\n", log_id(mod), log_signal(lhs),
							  log_signal(con.second));
					}
					offset += chunk.width;
				}
				new_connections.emplace_back(SigSpec(alias_wire), con.second);
			}
		}

		// now we have to go through the list all the wires that have subword
		// assignments and create a SigSpec that fully defines the word. However,
		// we do not create a connection here since this may lead to introducing
		// a cycle if we have something like:
		// connect $swire [0] some_other_wire;
		// connect $swire [1:0] { $swire [2] $swire [2]}

		dict<Wire *, SigSpec> resolved_subwords;
		for (const auto &subword : subwords) {

			auto wire = subword.first;
			auto resolved_sig = SigSpec(State::Sx, wire->width);
			for (const auto &chunk_drivers : subword.second) {
				auto chunk = chunk_drivers.first;
				auto driver = SigSpec(chunk_drivers.second);
				for (int ix = 0; ix < chunk.width; ix++) {
					resolved_sig[ix + chunk.offset] = driver[ix];
				}
			}

			if (!resolved_sig.is_fully_def()) {
				log_warning("%s.%s has undriven bits\n", log_id(mod), log_id(wire->name));
			}
			resolved_subwords.emplace(wire, resolved_sig);
		}
		// At this point we know exactly what drives each wire that have subword
		// assignment. We should go through the whole netlist RHS and input signals
		// and replace them with the new wires we have created so far with the
		// resolved SigSpec for each wire

		auto resolveRhs = [&resolved_subwords](const SigSpec &rhs) -> SigSpec {
			auto chunks = rhs.chunks();
			SigSpec resolved;
			for (const auto &chunk : chunks) {
				if (chunk.is_wire() && resolved_subwords.count(chunk.wire)) {
					auto driver = resolved_subwords[chunk.wire];
					for (int i = chunk.offset; i < chunk.width + chunk.offset; i++) {
						resolved.append(driver[i]);
					}
				} else {
					resolved.append(chunk);
				}
			}
			return resolved;
		};

		for (auto cell : mod->cells()) {
			for (auto &con : cell->connections()) {
				auto port_name = con.first;
				auto sig_in = con.second;
				if (cell->input(port_name)) {
					cell->setPort(port_name, resolveRhs(sig_in));
				}
			}
		}

		std::vector<SigSig> resolved_connections;
		for (const auto &con : new_connections) {
			auto rhs = con.second;
			resolved_connections.emplace_back(con.first, resolveRhs(rhs));
		}
		for(const auto& alias :  resolved_subwords) {

			resolved_connections.emplace_back(SigSpec(alias.first), alias.second);
		}

		mod->new_connections(resolved_connections);
	}
} ManticoreSubword;

PRIVATE_NAMESPACE_END