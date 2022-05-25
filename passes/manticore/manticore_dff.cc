#include "kernel/ffinit.h"
#include "kernel/modtools.h"
#include "kernel/yosys.h"

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct ManticoreDff : public Pass {

	ManticoreDff() : Pass("manticore_dff", "Replace all valid state elements with DFF cells, removing built-in reset and enables"){};
	void help() override
	{ //   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
		log("\n");
		log("    manticore_dff\n");
		log("\n");
	}

	struct Builder {
		int index;
		Module *m_module;
		Builder(Module *m) : index(0), m_module(m) {}
		IdString fresh(const std::string &prefix)
		{
			int new_ix = index;
			auto name = m_module->uniquify(stringf("$%s$", prefix.c_str()), new_ix);
			index = new_ix;
			return name;
		}
		bool isConvertible(Cell *cell) const
		{
			return cell->type.in(ID($dffe),	  // dff with EN
					     ID($sdff),	  // dff with sync reset
					     ID($sdffe),  // dff with sync reset over EN
					     ID($sdffce), // diff with (chip)-EN and sync reset (reset only applies if enabled)
					     ID($adff),	  // dff with async reset
					     ID($adffe)  // dff with async reset and enable
			);
		}
		bool isUnsupported(Cell *cell) const
		{

			return cell->type.in(ID($dffsr), ID($dffsre),							   // set-reset
					     ID($dlatch), ID($adlatch), ID($dlatchsr), ID($adffe), ID($aldff), ID($aldffe) // latches

			);
		}

		Cell *dffCast(Cell *cell)
		{
			auto din = cell->getPort(ID::D);
			auto dout = cell->getPort(ID::Q);
			auto dff = m_module->addDff(fresh("dff_cast"), cell->getPort(ID::CLK), cell->getPort(ID::D), cell->getPort(ID::Q),
						    cell->getParam(ID::CLK_POLARITY).as_bool());
			dff->setPort(ID::CLK, cell->getPort(ID::CLK));
			// dff->setPort(ID::Q, cell->getPort(ID::Q));
			dff->setParam(ID::CLK_POLARITY, cell->getParam(ID::CLK_POLARITY));
			dff->attributes.swap(cell->attributes);

			return dff;
		}
		Cell *sDffCast(Cell *cell)
		{
			auto dff = dffCast(cell);
			dff->setPort(ID::Q, cell->getPort(ID::Q));
			return dff;
		}

		SigSpec muxed(const SigSpec &A, const SigSpec &B, const SigSpec &S, bool polarity)
		{
			auto y_size = A.size();
			log_assert(y_size == B.size());
			auto y_wire = m_module->addWire(fresh("mux_out"), y_size);
			m_module->addMux(fresh("dff_mux"), polarity ? A : B, polarity ? B : A, S, y_wire);
			return SigSpec(y_wire);
		}
	};
	void transform(Design *design, Module *mod)
	{

		auto builder = Builder(mod);

		std::vector<Cell *> convertibles;
		pool<Module *> sub_modules;

		// Try to set the initial values of each state element first
		SigMap sigmap(mod);
		FfInitVals initvals(&sigmap, mod);

		for (auto cell : mod->cells()) {

			if (cell->type == ID($dff) || builder.isConvertible(cell)) {
				SigSpec q_sig = cell->getPort(ID::Q);
				Const init = initvals(q_sig);
				if (!init.is_fully_def() == false) {

					cell->set_const_attribute(ID::INIT, init);
					log("Setting %s.%s.%s (port=%s, net=%s) to %s.\n", log_id(mod), log_id(cell), log_id(ID::INIT), log_id(ID::Q),
					    log_signal(q_sig), log_const(init));
				}
			}
		}

		// then convert them all into $dffs (remove built-in enable and resets)
		for (const auto &cell : mod->cells()) {
			if (builder.isConvertible(cell)) {
				convertibles.push_back(cell);
			} else {
				if (builder.isUnsupported(cell)) {
					log_error("%s.%s of type %s is not supported!\n", log_id(mod), log_id(cell), log_id(cell->type));
				}
				auto user_cell = design->module(cell->type);
				if (user_cell != NULL) {
					sub_modules.insert(user_cell);
				}
			}
		}

		for (auto cell : convertibles) {

			if (cell->type == ID($dffe)) {
				auto dff = builder.sDffCast(cell);
				auto en_pol = cell->getParam(ID::EN_POLARITY).as_bool();
				auto q_sig = dff->getPort(ID::Q);
				auto d_sig = cell->getPort(ID::D);
				auto din = builder.muxed(q_sig, d_sig, cell->getPort(ID::EN), en_pol);
				dff->setPort(ID::D, din);
				mod->remove(cell);

			} else if (cell->type == ID($sdff)) {
				auto dff = builder.sDffCast(cell);
				auto srst_pol = cell->getParam(ID::SRST_POLARITY).as_bool();
				auto srst_value = SigSpec(cell->getParam(ID::SRST_VALUE));
				auto d_sig = cell->getPort(ID::D);
				auto din = builder.muxed(d_sig, srst_value, cell->getPort(ID::SRST), srst_pol);
				dff->setPort(ID::D, din);
				mod->remove(cell);

			} else if (cell->type == ID($sdffe)) {
				auto dff = builder.sDffCast(cell);
				auto srst_pol = cell->getParam(ID::SRST_POLARITY).as_bool();
				auto en_pol = cell->getParam(ID::EN_POLARITY).as_bool();
				auto q_sig = dff->getPort(ID::Q);
				auto d_sig = cell->getPort(ID::D);
				auto srst_value = SigSpec(cell->getParam(ID::SRST_VALUE));
				auto din = builder.muxed(builder.muxed(q_sig, d_sig, cell->getPort(ID::EN), en_pol), srst_value,
							 cell->getPort(ID::SRST), srst_pol);
				dff->setPort(ID::D, din);
				mod->remove(cell);

			} else if (cell->type == ID($sdffce)) {

				auto dff = builder.sDffCast(cell);
				auto srst_pol = cell->getParam(ID::SRST_POLARITY).as_bool();
				auto en_pol = cell->getParam(ID::EN_POLARITY).as_bool();
				auto q_sig = dff->getPort(ID::Q);
				auto d_sig = cell->getPort(ID::D);
				auto srst_value = SigSpec(cell->getParam(ID::SRST_VALUE));
				auto din = builder.muxed(q_sig, builder.muxed(d_sig, srst_value, cell->getPort(ID::SRST), srst_pol),
							 cell->getPort(ID::EN), en_pol);
				dff->setPort(ID::D, din);
				mod->remove(cell);

			} else if (cell->type == ID($adff)) {
				// the difference here is that we put the MUX of the reset at the
				// Q port AND the D port
				auto dff = builder.dffCast(cell);
				auto arst_pol = cell->getParam(ID::ARST_POLARITY).as_bool();

				auto arst_value = SigSpec(cell->getParam(ID::ARST_VALUE));
				auto q_sig = cell->getPort(ID::Q);
				auto d_sig = cell->getPort(ID::D);
				auto arst_sig = cell->getPort(ID::ARST);
				// put a mux before of the dff
				auto din = builder.muxed(d_sig, arst_value, arst_sig, arst_pol);
				dff->setPort(ID::D, din);

				// and put another mux after the dff
				auto qout = mod->addWire(builder.fresh("qout"), q_sig.size());
				dff->setPort(ID::Q, qout);
				mod->addMux(builder.fresh("arst_mux"), arst_pol ? qout : arst_value, arst_pol ? arst_value : qout, arst_sig, cell->getPort(ID::Q));

				mod->remove(cell);

			} else if (cell->type == ID($adffe)) {
				// reset has priority over enable
				auto dff = builder.dffCast(cell);
				auto arst_pol = cell->getParam(ID::ARST_POLARITY).as_bool();
				auto en_pol = cell->getParam(ID::EN_POLARITY).as_bool();
				auto arst_value = SigSpec(cell->getParam(ID::ARST_VALUE));
				auto q_sig = cell->getPort(ID::Q);
				auto d_sig = cell->getPort(ID::D);
				auto arst_sig = cell->getPort(ID::ARST);
				// put a mux before of the dff

				auto din = builder.muxed(builder.muxed(q_sig, d_sig, cell->getPort(ID::EN), en_pol), arst_value, arst_sig, arst_pol);
				dff->setPort(ID::D, din);

				// and put another mux after the dff
				auto qout = mod->addWire(builder.fresh("qout"), q_sig.size());
				dff->setPort(ID::Q, qout);
				mod->addMux(builder.fresh("arst_mux"), arst_pol ? qout : arst_value, arst_pol ? arst_value : qout, arst_sig, cell->getPort(ID::Q));

				mod->remove(cell);
			}  else {
				log_abort();
			}
		}



		for (auto sm : sub_modules) {
			transform(design, sm);
		}
	}
	void execute(std::vector<std::string>, Design *design) override
	{

		log_header(design, "Executing Manticore DFF Pass\n");

		auto top = design->top_module();

		if (top == nullptr) {
			log_error("Top module not defined!");
		}

		transform(design, top);
	}
} ManticoreDff;

PRIVATE_NAMESPACE_END