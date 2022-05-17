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
		auto freshName = [&](const std::string &prefix) { return mod->uniquify(stringf("$%s$", prefix.c_str()), name_index); };

		dict<Wire *, std::vector<std::pair<SigChunk, SigChunk>>> subwords;
		for (auto cell : mod->cells()) {
			for (auto &con : cell->connections()) {
				auto port_name = con.first;
				auto sig = con.second;
				if (cell->output(port_name) && !sig.is_wire()) {

					auto alias_wire = mod->addWire(freshName("subword_out"), sig.size());
					cell->setPort(port_name, alias_wire);

					log("Replacing %s.%s.%s => %s with %s\n", log_id(mod), log_id(cell), log_id(port_name), log_signal(sig),
					    log_signal(alias_wire));

					auto chunks = sig.chunks();
					int offset = 0;
					for (const auto &chunk : chunks) {

						if (chunk.is_wire()) {
							subwords[chunk.wire].emplace_back(chunk, SigChunk(alias_wire, offset, chunk.width));
						} else {
							log_error("%s.%s.%s => %s assigns to contant\n", log_id(mod), log_id(cell), log_id(port_name), log_signal(sig));
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
				auto alias_wire = mod->addWire(freshName("subword_lhs"), lhs.size());
				log("Replacing %s with %s\n", log_signal(lhs), log_signal(alias_wire));

				auto chunks = lhs.chunks();
				int offset = 0;
				for (const auto &chunk : chunks) {
					if (chunk.is_wire()) {
						subwords[chunk.wire].emplace_back(chunk, SigChunk(alias_wire, offset, chunk.width));
					} else {
						log_error("In %s connect %s %s assigns to contant\n", log_id(mod), log_signal(lhs), log_signal(con.second));
					}
					offset += chunk.width;
				}
				new_connections.emplace_back(SigSpec(alias_wire), con.second);
			}
		}

		// now we have to go through the list all the wires that have subword
		// assignments and create a single word assignment. We should also
		// error out or give a warning if some chunks of a wire are not assigned
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
			auto has_undefined_bit =
			  std::any_of(resolved_sig.begin(), resolved_sig.end(), [](const SigBit &b) { return b == SigBit(State::Sx); });
			if (has_undefined_bit) {
				log_warning("%s.%s has undriven bits\n", log_id(mod), log_id(wire->name));
			}
			new_connections.emplace_back(SigSpec(wire), resolved_sig);
		}

		mod->new_connections(new_connections);
	}
} ManticoreSubword;

PRIVATE_NAMESPACE_END