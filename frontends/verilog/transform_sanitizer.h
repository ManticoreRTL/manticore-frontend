#include "frontends/ast/ast.h"
#include "kernel/yosys.h"
#include <list>
#include <stdint.h>
#include <stdio.h>

namespace masm_frontend
{

using namespace Yosys;

class TransformSanitizer
{
      public:
	TransformSanitizer(AST::AstNode *orig, AST::AstNode *transformed) : m_orig(orig), m_transformed(transformed) {


    }
	void check() const;

      private:
	void build__(AST::AstNode *parent, bool is_orig = true) const;
	using Child = AST::AstNode;
	using Parent = AST::AstNode;
	AST::AstNode *m_orig, *m_transformed;
    // using HashMap = std::unordered_map<Child *, Parent *>;
    using HashMap = dict<Child *, Parent *>;
	mutable HashMap m_orig_map;
	mutable HashMap m_transformed_map;
};
} // namespace masm_frontend