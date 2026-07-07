#include "../classes/FlowAIAgent3D.hpp"

#include <godot_cpp/classes/engine.hpp>

namespace FlowAI {
    Ref<StandardMaterial3D> _agent_path_mat;

	void FlowAIAgent3D::draw_agent_pathnode_path(PackedVector3Array p_path) {
        if (!is_debug_enabled(DEBUG_VISUALIZE_AGENT_PATH) || p_path.size() < 1) return;

        immediate_mesh->clear_surfaces();

        if (p_path.size() < 2) {
            return;
        }

        immediate_mesh->surface_begin(Mesh::PRIMITIVE_LINES);

        for (int i = 0; i < p_path.size() - 1; ++i) {
            Vector3 current_point = p_path[i];
            Vector3 next_point = p_path[i + 1];
            current_point.y += 0.2f;
            next_point.y += 0.2f;

            immediate_mesh->surface_set_color(get_agent_path_line_color());
            immediate_mesh->surface_add_vertex(current_point);
            immediate_mesh->surface_set_color(get_agent_path_line_color());
            immediate_mesh->surface_add_vertex(next_point);
        }

        immediate_mesh->surface_end();

        if (_agent_path_mat.is_null()) {
            _agent_path_mat.instantiate();
            _agent_path_mat->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
            _agent_path_mat->set_albedo(get_agent_path_line_color());
        }

        // In this way the user can update the color in real time. I'll do a support to it one day.
        if (path_preview->get_surface_override_material_count() > 0)
            path_preview->set_surface_override_material(0, _agent_path_mat);
	}
}