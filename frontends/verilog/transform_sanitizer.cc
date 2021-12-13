
#include "transform_sanitizer.h"

namespace masm_frontend
{
void TransformSanitizer::check() const
{
	build__(m_orig, true);
	build__(m_transformed, false);
}
void TransformSanitizer::build__(AST::AstNode *parent, bool is_orig) const
{

	HashMap &map_ref = is_orig ? m_orig_map : m_transformed_map;

	for (const auto &child : parent->children) {

		auto iter_ok = map_ref.insert(std::make_pair(child, parent));
		if (!iter_ok.second) {
            if (!is_orig) {
                log_warning("Bad tree transform!\n");
            } else {
                log_warning("Bad original tree\n");
            }
            // HashMap& other_map = is_orig ? m_transformed_map : m_orig_map;
            // auto registered_parent = m_orig_map.find(child)
            log("\nChild:\n");
            child->dumpVlog(NULL, "\t\t");
            log("\nRegistered parent:\n");
            map_ref.find(child)->second->dumpVlog(NULL, "\t\t");
            log("\nAttempted parent:\n");
            parent->dumpVlog(NULL, "\t\t");
			log_error("could not insert child %p is already registered with another parent %p",
                child, map_ref.find(child)->second);
		}
        build__(child, is_orig);
	}
}

} // namespace masm_frontend