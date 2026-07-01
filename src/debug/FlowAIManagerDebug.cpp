#include "../classes/FlowAIManager.hpp"
#include "../classes/FlowAIPathnode.hpp"
#include "../FlowAIDebug.hpp"

// Draw the beautiful sections grid
// Also draw a "X" in sections that have pathnodes attached
// TODO: The use should choice if want do render this grid or not

// If are you baking your pathnodes and the grid isnt updating while you rebuild this plugin many times with godot opened.
// Dont worry, its completely normal. Just restart the project that the grid update will start working again.
// the same applies to the pathnode_connection.

namespace FlowAI {
	//Ref<StandardMaterial3D> _grid_mat;
	Ref<StandardMaterial3D> _connections_mat;

	void FlowAIManager::_draw_sections_grid(Ref<FlowAIBakeData> bake_data) {
		if (imm_grid_mesh.is_null()) return;

		/// fuck grid material.
		//if (_grid_mat.is_null()) {
		//	_grid_mat.instantiate();
		//	_grid_mat->set_albedo(FlowAIDebug::grid_main_color);
		//}

		//// In this way the user can update the color in real time. I'll do a support to it one day.
		//if (grid_preview->get_surface_override_material_count() > 0)
		//	grid_preview->set_surface_override_material(0, _grid_mat);

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
			imm_grid_mesh->surface_add_vertex(Vector3(x, 0.05f, min_z));
			imm_grid_mesh->surface_add_vertex(Vector3(x, 0.05f, max_z));
		}

		// Draw rows
		for (int r = -half_rows; r <= half_rows; ++r) {
			float z = r * section_size;
			imm_grid_mesh->surface_add_vertex(Vector3(min_x, 0.05f, z));
			imm_grid_mesh->surface_add_vertex(Vector3(max_x, 0.05f, z));
		}

		// Draw "X" in sections that have pathnodes
		if (bake_data.is_valid() && FlowAIDebug::draw_x_in_sections_that_have_pathnode) {
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

							imm_grid_mesh->surface_set_color(FlowAIDebug::active_sector_color);

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

	void FlowAIManager::_draw_pathnode_connections(std::unordered_map<uint32_t, FlowAIPathnode*> database) {
		if (imm_pathnode_connections_mesh.is_null()) return;

		if (_connections_mat.is_null()) {
			_connections_mat.instantiate();
			_connections_mat->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
			_connections_mat->set_albedo(FlowAIDebug::pathnode_connections_color);
		}

		// In this way the user can update the color in real time. I'll do a support to it one day.
		if (pathnode_connections_preview->get_surface_override_material_count() > 0)
			pathnode_connections_preview->set_surface_override_material(0, _connections_mat);

		std::vector<FlowAIPathnode*> all_pathnodes = get_pathnode_list();
		if (all_pathnodes.size() < 1) return;

		imm_pathnode_connections_mesh->clear_surfaces();
		imm_pathnode_connections_mesh->surface_begin(Mesh::PRIMITIVE_LINES);

		for (int i = 0; i < all_pathnodes.size(); ++i) {
			FlowAIPathnode* current_node = all_pathnodes[i];
			if (!current_node) continue;

			Vector3 start_pos = current_node->get_global_position();
			start_pos.y += 0.1f;

			PackedInt32Array linked_ids = current_node->get_links();

			for (int j = 0; j < linked_ids.size(); ++j) {
				uint32_t target_id = linked_ids[j];

				FlowAIPathnode* target_node = nullptr;
				auto it = database.find(target_id);
				if (it != database.end()) {
					target_node = it->second;
				}

				Vector3 end_pos = target_node->get_global_position();
				end_pos.y += 0.1f;

				if (target_node) {
					imm_pathnode_connections_mesh->surface_set_color(FlowAIDebug::pathnode_connections_color);
					imm_pathnode_connections_mesh->surface_add_vertex(start_pos);
					imm_pathnode_connections_mesh->surface_set_color(FlowAIDebug::pathnode_connections_color);
					imm_pathnode_connections_mesh->surface_add_vertex(end_pos);
				}

			}
		}
		imm_pathnode_connections_mesh->surface_end();
	}
}