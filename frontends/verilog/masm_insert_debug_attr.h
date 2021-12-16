#include "frontends/ast/ast.h"
#include "kernel/yosys.h"
#include <list>
#include <stdint.h>
#include <stdio.h>

namespace masm_frontend
{

#define DISPATCH_DECL(ast_type) void __transform_##ast_type(AstNode *n)

using namespace Yosys;
class MasmInsertDebugAttributes
{

      public:
	using AstNode = AST::AstNode;
	using NodeType = AST::AstNodeType;
	MasmInsertDebugAttributes(AstNode *design)
	{
		m_design = design;
		m_mutable_design = design->clone();
        m_done = false;
	}


	AstNode *transformed();



      private:
    void transformNode(AstNode*);
	AstNode *m_design;
	AstNode *m_mutable_design;
    std::vector<AstNode*> hierarchy;
    bool m_done;
};

}; // namespace masm_frontend