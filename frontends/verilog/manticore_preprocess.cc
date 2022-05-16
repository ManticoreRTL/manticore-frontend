#include "manticore_preprocess.h"

namespace masm_frontend
{

#define DISPATCH_DEF(ast_type, node) void ManticorePreprocess::__transform_##ast_type(AstNode *node)
#define no_warn_unused __attribute__((unused))

AST::AstNode *ManticorePreprocess::transformed()
{
	if (!m_is_transformed)
		transformNode(m_mutable_design);
	m_is_transformed = true;
	return m_mutable_design;
}

void ManticorePreprocess::transformNode(AstNode *mutable_ast)
{

// implement double dispatch manually because we are not
// using polymorphism

// using namespace AST;
#define DISPATCH_CALL(ast_type)                                                                                                                      \
	case AST::ast_type:                                                                                                                          \
		__transform_##ast_type(mutable_ast);                                                                                                 \
		break;

	// log("transforming node from %s\n", mutable_ast->loc_string().c_str());
	// mutable_ast->dumpVlog(NULL, "\t\t");
	// log("\n\n");
	// The vtable
	switch (mutable_ast->type) {
		DISPATCH_CALL(AST_MODULE)
		DISPATCH_CALL(AST_ALWAYS)
		DISPATCH_CALL(AST_BLOCK)
		DISPATCH_CALL(AST_WHILE)
		DISPATCH_CALL(AST_FOR)
		DISPATCH_CALL(AST_REPEAT)
		DISPATCH_CALL(AST_CASE)
		DISPATCH_CALL(AST_DESIGN)
	default:
		__transform_FALLBACK(mutable_ast);
		break;
	}
}

#define TRANSFORM_CHILDREN(node)                                                                                                                     \
	for (auto &c : node->children) {                                                                                                             \
		transformNode(c);                                                                                                                    \
	}
/**
 * @brief Fallback case of transformation, calls the \c transformNode
 * method on each child
 *
 */
DISPATCH_DEF(FALLBACK, node) { TRANSFORM_CHILDREN(node); }

DISPATCH_DEF(AST_DESIGN, design)
{

	// set_simplify_design_context(design);
	int name_index = 0;
	for (const auto &child : design->children) {
		if (child->type == AST::AST_MODULE)
			if (child->str == "\\MASM_PRIVILAGED") {
				log_warning("Please avoid using MASM_PRIVILAGED as a module name");
				name_index += 1;
			}
	}

	m_masm_privilaged_name = stringf("\\MASM_PRIVILAGED_%d", name_index);
	log_assert(m_defined_modules.empty());
	TRANSFORM_CHILDREN(design);

	if (m_add_masm_privilaged) {
		auto create_module_param = [](const std::string &name, AstNode *expr) -> AstNode * {
			AstNode *mod_param = new AstNode(AST::AST_PARAMETER, expr);
			mod_param->str = name;
			return mod_param;
		};

		auto create_io_wire = [](const std::string &name, bool is_input) -> AstNode * {
			AstNode *range = new AstNode(AST::AST_RANGE, AstNode::mkconst_int(0, false), AstNode::mkconst_int(0, false));
			range->is_signed = false;
			AstNode *input_wire = new AstNode(AST::AST_WIRE, range);
			input_wire->is_input = is_input;
			input_wire->is_output = !is_input;
			input_wire->str = name;
			return input_wire;
		};
		AstNode *module_def = new AstNode(AST::AST_MODULE, create_module_param("\\msg", AstNode::mkconst_str("<EXPECT_FAILED>")),
						  create_module_param("\\mode", AstNode::mkconst_str("<UNDEF>")), create_io_wire("\\cond", true),
						  create_io_wire("\\cond_out", false));
		module_def->attributes.insert(std::make_pair(ID::blackbox, AstNode::mkconst_int(1, false)));
		module_def->attributes.insert(std::make_pair(ID::MASM_PRIVILAGED, AstNode::mkconst_int(1, false)));
		module_def->str = m_masm_privilaged_name;
		design->children.push_back(module_def);
	}
}

DISPATCH_DEF(AST_WHILE, node)
{
	m_while_loop.push(node);
	TRANSFORM_CHILDREN(node);
	m_while_loop.pop();
}

DISPATCH_DEF(AST_FOR, node)
{
	m_for_loop.push(node);
	TRANSFORM_CHILDREN(node);
	m_for_loop.pop();
}

DISPATCH_DEF(AST_REPEAT, node)
{
	m_repeat.push(node);
	TRANSFORM_CHILDREN(node);
	m_repeat.pop();
}

DISPATCH_DEF(AST_TCALL, task_call)
{

	if (task_call->str == "$masm_expect" || task_call->str == "$masm_abort" || task_call->str == "$masm_stop") {
		log_error("Did not expect %s at %s\n!", task_call->str.c_str(), task_call->loc_string().c_str());
	} else {
		TRANSFORM_CHILDREN(task_call);
	}
}

DISPATCH_DEF(AST_MODULE, module)
{

	// ensure no other module encloses this one
	log_assert(m_module.empty());
	log_assert(m_new_nodes.empty());
	m_module.push(module);
	// log("Handling module %s\n", module->str.c_str());
	m_defined_modules.insert(module->str);
	// set the current module scope
	for (auto &child : module->children) {
		transformNode(child);
	}
	// unset the current module scope
	while (m_new_nodes.empty() == false) {
		module->children.push_back(m_new_nodes.back());
		m_new_nodes.pop_back();
	}

	m_module.pop();
}

DISPATCH_DEF(AST_ALWAYS, always_node)
{

	// check if the always statement is clocked.

	AstNode *used_clock = nullptr;
	for (const auto &child : always_node->children) {
		if (child->type == AST::AST_POSEDGE || child->type == AST::AST_NEGEDGE) {
			used_clock = child;
			break;
		}
	}

	// enclosed by clocked always blocks, we could also skip transforming
	// the nodes that are not clocked, but we do not do so to be able to
	// emit and error if an expect without a clocked block is encountered
	if (used_clock != nullptr) {
		m_current_clock.push(used_clock);
	}
	TRANSFORM_CHILDREN(always_node);
	if (used_clock) {
		m_current_clock.pop();
	}
}

// TODO handle AST_CASEZ and AST_CASEX
DISPATCH_DEF(AST_CASE, switch_node)
{

	auto switch_cond_expr = switch_node->children[0];

	std::vector<AstNode *> non_default_conds;
	for (auto &case_cond : switch_node->children) {
		if (case_cond->type == AST::AST_COND) {
			// go through all non-default cases and
			// recurse inside, while pushing a new condition
			// to the stack of observed ones
			auto case_label = case_cond->children[0];
			if (case_label->type != AST::AST_DEFAULT) {
				auto case_value = case_cond->children[0];
				auto cond_expr = new AstNode(AST::AST_EQ, switch_cond_expr->clone(), case_value->clone());
				m_conditions.push(cond_expr);
				transformNode(case_cond);
				auto top = m_conditions.top();
				log_assert(top == cond_expr);
				m_conditions.pop();
				non_default_conds.push_back(cond_expr);
				// delete cond_expr;
			}
		}
	}

	// now go thorugh all the labels again, this time only
	// handle the default case by creating a big conjunction of negation
	// of other labels
	for (auto *case_cond : switch_node->children) {
		if (case_cond->type == AST::AST_COND) {

			auto case_label = case_cond->children[0];
			if (case_label->type == AST::AST_DEFAULT) {

				Stack negated_labels;
				for (const auto &non_default : non_default_conds) {
					auto negated = new AstNode(AST::AST_LOGIC_NOT, non_default->clone());
					negated_labels.push(negated);
				}

				m_conditions.push(conjunction(negated_labels));
				transformNode(case_cond);
				delete m_conditions.top();
				m_conditions.pop();
				while (negated_labels.empty() == false) {
					delete negated_labels.top();
					negated_labels.pop();
				}
			}
		}
	}

	for (auto *c : non_default_conds) {
		delete c;
	}
}

DISPATCH_DEF(AST_BLOCK, block)
{

	std::vector<AstNode *> transformed_children;

	for (auto &child : block->children) {

		if (child->type == AST::AST_TCALL && (child->str == "$masm_expect" || child->str == "$masm_stop" || child->str == "$masm_abort")) {

			log("Found masm system call at %s\n", child->loc_string().c_str());

			if (m_module.size() != 1) {
				log_error("Found %s outside a module (or inside nested modules)!\n", child->str.c_str());
				log_abort();
			}
			if (m_repeat.size() != 0) {
				log_error("%s inside repeat not supported yet!\n", child->str.c_str());
				log_abort();
			}
			if (m_current_clock.size() != 1) {
				log_error("%s should be inside a single clocked always block\n", child->str.c_str());
				log_abort();
			}
			if (m_while_loop.size() != 0) {
				log_error("%s inside while is not supported yet\n", child->str.c_str());
				log_abort();
			}
			if (m_for_loop.size() != 0) {
				log_error("%s inside for loop is not supported yet\n", child->str.c_str());
				log_abort();
			}

			if (isValidTaskSignature(child) == false) {
				log_abort();
			}

			AstNode *expect_task_call = child;

			AstNode *condition_reg = new AstNode(AST::AST_WIRE);
			condition_reg->str = freshName("reg_cond", child);
			condition_reg->is_reg = true;
			AstNode *condition_id = new AstNode(AST::AST_IDENTIFIER);
			condition_id->str = condition_reg->str;
			AstNode *condition_init =
			  new AstNode(AST::AST_INITIAL,
				      new AstNode(AST::AST_BLOCK, new AstNode(AST::AST_ASSIGN_EQ, condition_id, AstNode::mkconst_int(1, false, 1))));

			m_new_nodes.push_back(condition_reg);
			m_new_nodes.push_back(condition_init);
			// m_module.top()->children.push_back(condition_reg);

			// now construct an always block to set the condition value
			auto make_implication = [this](Stack &nested_conditions, AstNode *task_call) -> AstNode * {
				AstNode *impl = nullptr;
				AstNode *q = nullptr;
				// a $masm_expect nested inside if-else statement can be viewed
				// as another one in th outer-most scope with condition p -> q
				// where p is the conjunction of nested conditions and q is the
				// local task call condition.
				if (task_call->str == "$masm_expect") {
					// q is the argument to $masm_expect
					q = task_call->children[0]->clone();
				} else if (task_call->str == "$masm_stop" || task_call->str == "$masm_abort") {
					// stop or abort are like expect(false, "msg")
					q = AstNode::mkconst_int(0, false, 1);
				}

				log_assert(q != nullptr);
				if (GetSize(nested_conditions) > 0) {
					// create conjunction
					AstNode *p = this->conjunction(nested_conditions);
					AstNode *not_p = new AST::AstNode(AST::AST_LOGIC_NOT, p);
					AstNode *cond = new AST::AstNode(AST::AST_LOGIC_OR, not_p, q);
					impl = cond;
				} else {
					impl = q;
				}
				return impl;
			};

			AstNode *implication_expr = make_implication(m_conditions, expect_task_call);
			log_assert(implication_expr != nullptr);
			AstNode *lvalue = new AstNode(AST::AST_IDENTIFIER);
			lvalue->str = condition_reg->str;
			AstNode *cond_always = new AstNode(AST::AST_ALWAYS, m_current_clock.top()->clone(),
							   new AstNode(AST::AST_BLOCK, new AstNode(AST::AST_ASSIGN_LE, lvalue, implication_expr)));
			// TODO: do we need an initial condition value?
			// Do not directly add it to the enclosing module, because we
			// may still have other always blocks that are not visited
			// this is because we are mutating the AST in place :(
			m_new_nodes.push_back(cond_always);

			// cell instantiation
			const auto instance_name = freshName("instance", child);
			const auto module_name = m_masm_privilaged_name;
			// create a cell instant
			AstNode *expect_cell = new AstNode(AST::AST_CELL, new AstNode(AST::AST_CELLTYPE));

			expect_cell->str = instance_name;
			// set the cell/black box type to be a fresh black box name
			expect_cell->children[0]->str = module_name;

			auto create_param_set = [](const std::string &name, AstNode *expr) -> AstNode * {
				AstNode *parset = new AstNode(AST::AST_PARASET, expr);
				parset->str = name;
				return parset;
			};

			// add a parameter to hold the format string (if exists)
			if (expect_task_call->str == "$masm_expect" && GetSize(expect_task_call->children) > 1) {
				// Set the msg parameter of the expect cell
				const std::string param_name = "\\msg";
				AstNode *format_expr = expect_task_call->children[1];
				expect_cell->children.push_back(create_param_set("\\msg", format_expr->clone()));
			}
			// set the mode parameter, indicating the type of privilaged call
			expect_cell->children.push_back(create_param_set("\\mode", AstNode::mkconst_str(expect_task_call->str)));

			// keep the source location
			expect_cell->location = expect_task_call->location;
			expect_cell->filename = expect_task_call->filename;

			// now connect the condition reg to the cell

			AstNode *condition_connection = new AstNode(AST::AST_ARGUMENT, lvalue->clone());
			condition_connection->str = "\\cond";
			expect_cell->children.push_back(condition_connection);

			// Also create an output to prevent the cell deletion by opt_clean
			{
				AstNode *condition_out = new AstNode(AST::AST_WIRE);
				condition_out->str = freshName("cond_out", child);
				condition_out->is_output = true;
				AstNode *condition_out_id = new AstNode(AST::AST_IDENTIFIER);
				condition_out_id->str = condition_out->str;
				AstNode *cond_out_arg = new AstNode(AST::AST_ARGUMENT, condition_out_id);
				cond_out_arg->str = "\\cond_out";
				expect_cell->children.push_back(cond_out_arg->clone());
				m_new_nodes.push_back(condition_out);
				// ensure the optimizer won't remove this
				expect_cell->attributes.insert(std::make_pair(ID::keep, AstNode::mkconst_int(1, false)));
			}

			m_new_nodes.push_back(expect_cell);
			// delete this node, no longer needed
			// AstNode*
			m_add_masm_privilaged = true; // notify the design to add the MASM_PRIVILAGED blackbox
			delete child;
		} else if (child->type == AST::AST_TCALL && (child->str == "$display" || child->str == "$finish" || child->str == "$stop")) {


			// create a new AstNode represeting this $display/$stop/$finish with some added information
			// e.g., $display("my message with %d var args from %s", 2, "Manticore")
			// becomes $manticore_display(cond, order, "my message with %d var args from %s", 2, "Manticore");
			// which is later translated to an RTILIL blackbox cell in getRTIIL

			AstNode *cond_expr;
			if (m_conditions.size() == 0) {
				cond_expr = AstNode::mkconst_int(1, false, 1);
			} else {
				cond_expr = conjunction(m_conditions);
			}

			AstNode *manticore_node = new AstNode(AST::AST_MANTICORE, cond_expr, AstNode::mkconst_int(m_ordering++, false));
			for (const auto &grand_child : child->children) {
				manticore_node->children.push_back(grand_child->clone());
			}
			manticore_node->str = child->str;
			manticore_node->location = child->location;
			manticore_node->filename = child->filename;
			manticore_node->attributes.swap(child->attributes);
			transformed_children.push_back(manticore_node);
			delete child;

		} else if (child->type == AST::AST_ASSERT) {
			AstNode *en_expr;
			if (m_conditions.size() == 0) {
				en_expr = AstNode::mkconst_int(1, false, 1);
			} else {
				en_expr = conjunction(m_conditions);
			}
			AstNode *manticore_assert = new AstNode(AST::AST_MANTICORE, en_expr, AstNode::mkconst_int(m_ordering++, false));
			for (const auto &grand_child : child->children) {
				manticore_assert->children.push_back(grand_child->clone());
			}
			manticore_assert->str = "$assert";
			manticore_assert->location = child->location;
			manticore_assert->filename = child->filename;
			manticore_assert->attributes.swap(child->attributes);
			transformed_children.push_back(manticore_assert);
			delete child;
		} else {
			transformNode(child);
			transformed_children.push_back(child);
		}
	}

	block->children = transformed_children;
}

}; // namespace masm_frontend