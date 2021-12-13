#include "masm_expect.h"
namespace masm_frontend
{

#define DISPATCH_DEF(ast_type, node) void HoistExpectTasks::__transform_##ast_type(AstNode *node)
#define no_warn_unused __attribute__((unused))

AST::AstNode *HoistExpectTasks::transformed()
{
	if (!m_is_transformed)
		transformNode(m_mutable_design);
	m_is_transformed = true;
	return m_mutable_design;
}

void HoistExpectTasks::transformNode(AstNode *mutable_ast)
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
		// DISPATCH_CALL(AST_WHILE)
		// DISPATCH_CALL(AST_FOR)
		DISPATCH_CALL(AST_CASE)
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
DISPATCH_DEF(FALLBACK, node)
{
	// for(auto& child : node->children) {
	//     transformNode(child);
	// }

	TRANSFORM_CHILDREN(node);
}

DISPATCH_DEF(AST_TCALL, task_call)
{

	if (task_call->str == "$masm_expect") {
		log_error("Did not expect $masm_expect!");
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

DISPATCH_DEF(AST_WHILE, no_warn_unused while_node) {}

DISPATCH_DEF(AST_FOR, no_warn_unused for_node) {}

// TODO handle AST_CASEZ and AST_CASEX
DISPATCH_DEF(AST_CASE, switch_node)
{

	auto switch_cond_expr = switch_node->children[0];

	std::vector<AstNode *> non_default_labels;
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
				delete cond_expr;
				m_conditions.pop();
				non_default_labels.push_back(case_label);
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
				for (const auto &label : non_default_labels) {
					auto negated = new AstNode(AST::AST_LOGIC_NOT, label->clone());
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
}

DISPATCH_DEF(AST_BLOCK, block)
{

	std::vector<AstNode *> transformed_children;

	for (auto &child : block->children) {

		if (child->type == AST::AST_TCALL) {
			log("Observed AST_TCALL %s\n", child->str.c_str());
		}
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



			AstNode *expect_task_call = child;

			// set the cell arguments as .fmt(..), .varg0(..), .varg1(..)
			if (GetSize(expect_task_call->children) == 0) {
				log_error("Expect with no condition!");
				log_abort();
			}
			// create a reg for the condition and add it to the enclosing module
			AstNode *condition_reg = new AstNode(AST::AST_WIRE);
			condition_reg->str = freshName("reg_cond", child);
			condition_reg->is_reg = true;
			m_new_nodes.push_back(condition_reg);
			// m_module.top()->children.push_back(condition_reg);

			// now construct an always block to set the condition value
			m_conditions.push(expect_task_call->children[0]);
			AstNode *conjunction_expr = conjunction(m_conditions);
			m_conditions.pop();
			AstNode *lvalue = new AstNode(AST::AST_IDENTIFIER);
			lvalue->str = condition_reg->str;
			AstNode *cond_always = new AstNode(AST::AST_ALWAYS, m_current_clock.top()->clone(),
							   new AstNode(AST::AST_BLOCK, new AstNode(AST::AST_ASSIGN_LE, lvalue, conjunction_expr)));
			// TODO: do we need an initial condition value?
			// Do not directly add it to the enclosing module, because we
			// may still have other always blocks that are not visited
			// this is because we are mutating the AST in place :(
			m_new_nodes.push_back(cond_always);

			// Also create an output to prevent the cell deletion by opt_clean
			AstNode *condition_out = new AstNode(AST::AST_WIRE);
			condition_out->str = freshName("cont_out", child);
			condition_out->is_output = true;
			AstNode *cond_out_assign = new AstNode(AST::AST_ASSIGN, new AstNode(AST::AST_IDENTIFIER), lvalue->clone());
			cond_out_assign->children.front()->str = condition_out->str;
			m_new_nodes.push_back(condition_out);
			m_new_nodes.push_back(cond_out_assign);

			// cell instantiation
			const auto instance_name = freshName("instance", child);
			const auto module_name = freshName("MASM_PRIVILAGED", child);
			AstNode *expect_cell = new AstNode(AST::AST_CELL, new AstNode(AST::AST_CELLTYPE));
			expect_cell->str = instance_name;
			// set the cell/black box type to be a fresh black box name
			expect_cell->children[0]->str = module_name;

			// add a parameter to hold the format string (if exists)
			if (GetSize(expect_task_call->children) > 1) {
				// first set the fmt parameter of the expect cell
				AstNode *format_param = new AstNode(AST::AST_PARASET);
				format_param->str = "\\fmt";
				AstNode *format_expr = expect_task_call->children[1];
				format_param->children.push_back(format_expr->clone());
				expect_cell->children.push_back(format_param);
			}
			{
				AstNode *mode_param = new AstNode(AST::AST_PARASET);
				mode_param->str = "\\mode";
				mode_param->children.push_back(expect_task_call->mkconst_str(expect_task_call->str));
				expect_cell->children.push_back(mode_param);
			}
			// keep the source location
			expect_cell->location = expect_task_call->location;
			expect_cell->filename = expect_task_call->filename;

			// now connect the condition reg to the cell

			AstNode *condition_connection = new AstNode(AST::AST_ARGUMENT, lvalue->clone());
			condition_connection->str = "\\cond";
			expect_cell->children.push_back(condition_connection);

			// now handle variable arguments for the fmt as inputs to the cell
			for (int varg_ix = 2; varg_ix < GetSize(expect_task_call->children); varg_ix++) {

				AstNode *varg_expr = expect_task_call->children[varg_ix];
				log("expect varg%d = %s\n", varg_ix - 2, varg_expr->str.c_str());
				AstNode *varg_param = new AstNode(AST::AST_ARGUMENT, varg_expr->clone());
				varg_param->str = stringf("\\varg%d", varg_ix - 2);
				expect_cell->children.push_back(varg_param);
			}
			// finally add the cell to the module, this is safe since
			// we are not going to visit cell...
			m_new_nodes.push_back(expect_cell);
			// delete this node, no longer needed
			// AstNode*
			delete child;
		} else {
			transformNode(child);
			transformed_children.push_back(child);
		}
	}

	block->children = transformed_children;
}

}; // namespace masm_frontend