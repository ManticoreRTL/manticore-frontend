#include "frontends/ast/ast.h"
#include "kernel/yosys.h"
#include <list>
#include <stdint.h>
#include <stdio.h>

namespace masm_frontend
{

#define DISPATCH_DECL(ast_type) void __transform_##ast_type(AstNode *n)

using namespace Yosys;
class HoistExpectTasks
{

      public:
	using AstNode = AST::AstNode;
	using NodeType = AST::AstNodeType;
	HoistExpectTasks(AstNode *design)
	{
		m_design = design;
		m_mutable_design = design->clone();
		m_is_transformed = false;
		m_fresh_index = 0;
	}

	AstNode *transformed();

	static std::string getMasmPrivilagedModuleName() {
		return "\\$MASM_PRIVILAGED";
	}

      private:
#define DEFINE_DISPATH_METHOD(ast_type) void __transform
	using Stack = std::stack<AstNode *, std::vector<AstNode *>>;

	void transformNode(AstNode *mutable_ast);
	void __transform_FALLBACK(AstNode *n);
	DISPATCH_DECL(AST_MODULE);
	DISPATCH_DECL(AST_ALWAYS);
	DISPATCH_DECL(AST_BLOCK);
	DISPATCH_DECL(AST_FOR);
	DISPATCH_DECL(AST_WHILE);
	DISPATCH_DECL(AST_REPEAT);
	DISPATCH_DECL(AST_CASE);
	DISPATCH_DECL(AST_TCALL);
	DISPATCH_DECL(AST_DESIGN);

	std::string freshName(const std::string &prefix, const AstNode *node) const
	{

		return stringf("\\%s.%s.%d", prefix.c_str(), node->str.c_str(), m_fresh_index++);
	}

	// create (newly allocated) conjunction expression from the stack of
	// given conditions. The input is not changed and the output should be
	// memory-managed by the caller
	AstNode *conjunction(const Stack &conds) const
	{
		log_assert(conds.size() >= 1);
		if (conds.size() == 1) {
			// I am not sure if cloning is necessary
			return conds.top()->clone();

		} else {
			auto pop_return = [](Stack &s) -> AstNode * {
				// I am not sure if cloning is necessary
				auto ret_val = s.top()->clone();
				s.pop();
				return ret_val;
			};
			// copy the conditions to a new stack
			Stack mutable_conds = conds;
			AstNode *carry = pop_return(mutable_conds);
			log_assert(mutable_conds.size() > 0);

			while (mutable_conds.size() != 0) {

				auto c1 = pop_return(mutable_conds);
				AstNode *conjunct = new AstNode(AST::AST_LOGIC_AND, c1, carry);
				carry = conjunct;
			}
			return carry;
		}
	}

	bool isValidTaskSignature(AstNode *task_call) const
	{
		log_assert(task_call->type == AST::AST_TCALL);

		if (task_call->str == "$masm_expect") {

			if (GetSize(task_call->children) > 2) {
				log_error("%s requires at most 2 argument\n", task_call->str.c_str());
				return false;
			} else {
				return true;
			}
		} else if (task_call->str == "$masm_stop" || task_call->str == "$masm_abort") {

			if (GetSize(task_call->children) != 0) {
				// check that the first argument is the fmt
				log_error("Invalid number of arguments in %s at %s\n", task_call->str.c_str(), task_call->loc_string().c_str());
				return false;
			} else {
				return true;
			}
		} else {
			log_error("Unhandled masm task %s\n", task_call->str.c_str());
			return false;
		}
	}

	AstNode *m_design;
	AstNode *m_mutable_design;
	Stack m_conditions;
	Stack m_for_loop;
	Stack m_while_loop;
	Stack m_repeat;
	Stack m_module;
	Stack m_current_clock;
	std::vector<AstNode *> m_new_nodes; // container for new nodes to be added to a module
	// we can not directly add new nodes to the module because we perform the transformation
	// depth-first, therefore, internal nodes should not modify their parent to avoid invalidating
	// children iterators.
	// std::vector<AstNode *> m_new_blackboxes;
	bool m_is_transformed;
	mutable int m_fresh_index;
};

void hoistExpect(AST::AstNode *node);

void hoistExpectFromModule(AST::AstNode *module);

AST::AstNode *hoistExpectFromAlways(AST::AstNode *always, AST::AstNode *module);

AST::AstNode *transformClockedAlways(AST::AstNode *always, AST::AstNode *enclosing_module);

AST::AstNode *hoistExpectFromClockedBlock(AST::AstNode *block, AST::AstNode *enclosing_module, const std::vector<AST::AstNode *> &conditions,
					  const std::vector<AST::AstNode *> &enclosing_for, const std::vector<AST::AstNode *> &enclosing_while);

AST::AstNode *hoistExpectFromCaseStatement(AST::AstNode *case_stmt, AST::AstNode *enclosing_module, const std::vector<AST::AstNode *> &conditions,
					   const std::vector<AST::AstNode *> &enclosing_for, const std::vector<AST::AstNode *> &enclosing_while);

}; // namespace masm_frontend