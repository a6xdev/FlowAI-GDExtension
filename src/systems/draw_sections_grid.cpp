#include "../FlowAI.hpp"

// Draw the beautiful sections grid
// Also draw a "X" in sections that have pathnodes attached
// TODO: The use should choice if want do render this grid or not

namespace FlowAI {
	// maybe these colors var should be in plugin config
	Color line_color = Color(1.0f, 0.5f, 0.0f, 0.2f);
	Color border_color = Color(1.0f, 0.2f, 0.0f, 0.6f);
	Color active_sector_color = Color(0.0f, 1.0f, 0.5f, 0.4f);

	void FlowAIManager::_draw_sections_grid() {
		if (imm_grid_mesh.is_null()) return;

		imm_grid_mesh->clear_surfaces();

		int half_cols = section_cols / 2;
		int half_rows = section_rows / 2;
		float min_x = -half_cols * section_size;
		float max_x = half_cols * section_size;
		float min_z = -half_rows * section_size;
		float max_z = half_rows * section_size;

		imm_grid_mesh->surface_begin(Mesh::PRIMITIVE_LINES);

		// Draw cols
		for (int c = -half_cols; c <= half_cols; ++c) {
			float x = c * section_size;
			Color current_color = (c == -half_cols || c == half_cols) ? border_color : line_color;
			imm_grid_mesh->surface_set_color(current_color);
			imm_grid_mesh->surface_add_vertex(Vector3(x, 0.05f, min_z));
			imm_grid_mesh->surface_set_color(current_color);
			imm_grid_mesh->surface_add_vertex(Vector3(x, 0.05f, max_z));
		}

		// Draw rows
		for (int r = -half_rows; r <= half_rows; ++r) {
			float z = r * section_size;
			Color current_color = (r == -half_rows || r == half_rows) ? border_color : line_color;
			imm_grid_mesh->surface_set_color(current_color);
			imm_grid_mesh->surface_add_vertex(Vector3(min_x, 0.05f, z));
			imm_grid_mesh->surface_set_color(current_color);
			imm_grid_mesh->surface_add_vertex(Vector3(max_x, 0.05f, z));
		}

		// Draw "X" in sections that have pathnodes
		if (bake_data.is_valid()) {
			Dictionary payload = bake_data->get_sectors_payload();

			for (int r = -half_rows; r < half_rows; ++r) {
				for (int c = -half_cols; c < half_cols; ++c) {
					Vector2i coord(c, r);
					if (payload.has(coord)) {
						Dictionary sector_dict = payload[coord];
						Array nodes_in_sector = sector_dict["nodes"];
						if (!nodes_in_sector.is_empty()) {
							float x_min = c * section_size;
							float x_max = x_min + section_size;
							float z_min = r * section_size;
							float z_max = z_min + section_size;

							imm_grid_mesh->surface_set_color(active_sector_color);

							imm_grid_mesh->surface_add_vertex(Vector3(x_min, 0.06f, z_min));
							imm_grid_mesh->surface_add_vertex(Vector3(x_max, 0.06f, z_max));

							imm_grid_mesh->surface_add_vertex(Vector3(x_max, 0.06f, z_min));
							imm_grid_mesh->surface_add_vertex(Vector3(x_min, 0.06f, z_max));
						}
					}
				}
			}
		}
		imm_grid_mesh->surface_end();
	}
}