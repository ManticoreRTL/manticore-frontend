#include "kernel/yosys.h"

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct ManticoreDff : public Pass {

	ManticoreDff() : Pass("manticore_dff", "Replace all valid state elements with DFF cells, removing built-in reset and enables"){};
	void help() override
	{ //   |---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|---v---|
		log("\n");
		log("    manticore_init [-track FILE]\n");
		log("\n");
		log("Initialize the design with an optional list of signals to keep track of. The pass\n");
		log("basically appends (* keep *) attributes to the requested signals so that \n");
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
			return cell->type.in(ID($dffe),	 // dff with EN
					     ID($sdff),	 // dff with sync reset
					     ID($sdffe), // dff with sync reset over EN
					     ID($sdffce) // diff with (chip)-EN and sync reset (reset only applies if enabled)
			);
		}
		Cell *dffCast(Cell *cell)
		{
			auto din = cell->getPort(ID::D);
			auto dout = cell->getPort(ID::Q);
            auto dff = m_module->addDff(
                fresh("dff_cast"),
                cell->getPort(ID::CLK),
                cell->getPort(ID::D),
                cell->getPort(ID::Q),
                cell->getParam(ID::CLK_POLARITY).as_bool()
            );
			dff->setPort(ID::CLK, cell->getPort(ID::CLK));
			dff->setPort(ID::Q, cell->getPort(ID::Q));
			dff->setParam(ID::CLK_POLARITY, cell->getParam(ID::CLK_POLARITY));
			dff->attributes.swap(cell->attributes);

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

		for (const auto &cell : mod->cells()) {
			if (builder.isConvertible(cell)) {
				convertibles.push_back(cell);
			} else {
				auto user_cell = design->module(cell->type);
				if (user_cell != NULL) {
					sub_modules.insert(user_cell);
				}
			}
		}

		for (auto cell : convertibles) {

			if (cell->type == ID($dffe)) {
				auto dff = builder.dffCast(cell);
				auto en_pol = cell->getParam(ID::EN_POLARITY).as_bool();
				auto q_sig = dff->getPort(ID::Q);
				auto d_sig = cell->getPort(ID::D);
				auto din = builder.muxed(q_sig, d_sig, cell->getPort(ID::EN), en_pol);
				dff->setPort(ID::D, din);
				mod->remove(cell);


			} else if (cell->type == ID($sdff)) {
				auto dff = builder.dffCast(cell);
				auto srst_pol = cell->getParam(ID::SRST_POLARITY).as_bool();
				auto srst_value = SigSpec(cell->getParam(ID::SRST_VALUE));
				auto d_sig = cell->getPort(ID::D);
				auto din = builder.muxed(d_sig, srst_value, cell->getPort(ID::SRST), srst_pol);
				dff->setPort(ID::D, din);
				mod->remove(cell);

			} else if (cell->type == ID($sdffe)) {
				auto dff = builder.dffCast(cell);
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

				auto dff = builder.dffCast(cell);
				auto srst_pol = cell->getParam(ID::SRST_POLARITY).as_bool();
				auto en_pol = cell->getParam(ID::EN_POLARITY).as_bool();
				auto q_sig = dff->getPort(ID::Q);
				auto d_sig = cell->getPort(ID::D);
				auto srst_value = SigSpec(cell->getParam(ID::SRST_VALUE));
				auto din = builder.muxed(q_sig, builder.muxed(d_sig, srst_value, cell->getPort(ID::SRST), srst_pol),
							 cell->getPort(ID::EN), en_pol);
				dff->setPort(ID::D, din);
				mod->remove(cell);

			} else {

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