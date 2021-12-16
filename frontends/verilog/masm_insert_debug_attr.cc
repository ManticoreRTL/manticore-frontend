#include "masm_insert_debug_attr.h"



namespace masm_frontend {

AST::AstNode* MasmInsertDebugAttributes::transformed() {

    if (!m_done)
		transformNode(m_mutable_design);
	m_done = true;
	return m_mutable_design;

}

void MasmInsertDebugAttributes::transformNode(AstNode* node) {

    if (node->type == AST::AST_WIRE && (node->is_reg || node->is_logic || node->is_input || node->is_output)) {

        node->attributes.insert(
            std::make_pair(ID::MASM_DEBUG_SYMBOL, AstNode::mkconst_str(node->str))
        );
    } else if (node->type == AST::AST_CELL) {
        node->attributes.insert(
            std::make_pair(ID::MASM_DEBUG_SYMBOL, AstNode::mkconst_str(node->str))
        );
    }
    for(auto& child : node->children) {
        transformNode(child);
    }
}
}