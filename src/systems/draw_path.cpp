#include "../FlowAI.hpp"

Color path_line_color = Color(1.0f, 0.0f, 0.0f, 0.5f);

namespace FlowAI {

    void FlowAIAgent3D::draw_path(PackedVector3Array p_path) {
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

            immediate_mesh->surface_set_color(path_line_color);
            immediate_mesh->surface_add_vertex(current_point);
            immediate_mesh->surface_set_color(path_line_color);
            immediate_mesh->surface_add_vertex(next_point);
        }

        immediate_mesh->surface_end();
	}
}