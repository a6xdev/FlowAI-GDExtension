#include "FlowAI.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/resource_saver.hpp>

using namespace godot;

namespace FlowAI {
	FlowAIManager* FlowAIManager::singleton = nullptr;

	FlowAIManager::FlowAIManager() {
		if (singleton == nullptr) {
			singleton = this;
		}
	};
	FlowAIManager::~FlowAIManager() {
		if (singleton == this) {
			singleton = nullptr;
		}
	};

	void FlowAIManager::_bind_methods() {
		ClassDB::bind_method(D_METHOD("bake_sections"), &FlowAIManager::bake_sections);
		ClassDB::bind_method(D_METHOD("add_new_pathnode"), &FlowAIManager::add_new_pathnode);

		ClassDB::bind_method(D_METHOD("set_bake_data"), &FlowAIManager::set_bake_data);
		ClassDB::bind_method(D_METHOD("set_section_size"), &FlowAIManager::set_section_size);
		ClassDB::bind_method(D_METHOD("set_section_rows"), &FlowAIManager::set_section_rows);
		ClassDB::bind_method(D_METHOD("set_section_cols"), &FlowAIManager::set_section_cols);

		ClassDB::bind_method(D_METHOD("get_section_size"), &FlowAIManager::get_section_size);
		ClassDB::bind_method(D_METHOD("get_section_rows"), &FlowAIManager::get_section_rows);
		ClassDB::bind_method(D_METHOD("get_section_cols"), &FlowAIManager::get_section_cols);
		ClassDB::bind_method(D_METHOD("get_bake_data"), &FlowAIManager::get_bake_data);

		ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "bake_data", PROPERTY_HINT_RESOURCE_TYPE, "FlowAIBakeData"), "set_bake_data", "get_bake_data");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "section_size"), "set_section_size", "get_section_size");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "section_rows"), "set_section_rows", "get_section_rows");
		ADD_PROPERTY(PropertyInfo(Variant::INT, "section_cols"), "set_section_cols", "get_section_cols");
	}

	void FlowAIManager::_notification(int p_what) {
		switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
			// Previews for debug
			if (grid_preview == nullptr) {
				grid_preview = memnew(MeshInstance3D);
				if (imm_grid_mesh.is_null()) { imm_grid_mesh.instantiate(); }
				grid_preview->set_mesh(imm_grid_mesh);
				add_child(grid_preview);
			}

			if (!bake_data.is_null()) {
				_draw_sections_grid();
			}

			break;
		case NOTIFICATION_READY: // CREATE NODES
			set_process(true);
			if (!Engine::get_singleton()->is_editor_hint()) {
				astar_3d = memnew(AStar3D);
			}
			break;
		case NOTIFICATION_PROCESS: // PROCESS NODES
			break;
		case NOTIFICATION_EXIT_TREE:
			grid_preview->queue_free();
			break;
		}
	}

	// CALLS
	void active_section(std::vector<uint32_t> micro_pathnodes) {}
	void disable_section(std::vector<uint32_t> micro_pathnodes) {}

	// ------------------------------------------ //
	// ------------------------------------------ //
	void FlowAIManager::bake_sections() {
		if (bake_data.is_null()) {
			UtilityFunctions::print("[FlowAI] ERROR::BAKE_DATA::IS_EMPTY");
			return;
		}

		std::vector<FlowAIPathnode*> all_pathnodes = get_pathnode_list();
		_sectors_database.clear();

		// Populates the HashMap by pre-creating empty sections within the specified limit.
		for (int r = 0; r < section_rows; ++r) {
			for (int c = 0; c < section_cols; ++c) {
				Vector2i coord(c, r);
				FlowAISector new_sector = FlowAISector();
				new_sector.center_position = Vector3(coord.x, 0.0, coord.y);
				_sectors_database[coord] = new_sector;
			}
		}

		int nodes_baked = 0;
		int nodes_out_of_bounds = 0;

		for (int i = 0; i < all_pathnodes.size(); ++i) {
			FlowAIPathnode* pathnode = Object::cast_to<FlowAIPathnode>(all_pathnodes[i]);
			if (!pathnode) continue;

			// Calculate mathematically which square this node is stepping on
			Vector2i node_sector = _get_section_coords(pathnode->get_global_position());
			UtilityFunctions::print("[FlowAI] Pathnode: ", pathnode->get_name(), " | Section Coords : ", node_sector);

			// If its within the grid limits, add it to that section list.
			if (_is_within_grid_bounds(node_sector)) {
				_sectors_database[node_sector].micro_pathnodes.push_back(pathnode->get_id());
				pathnode->set_sector_coord(node_sector);
				nodes_baked++;
			}
			else {
				nodes_out_of_bounds++;
			}
		}

		// Save data in the user created FlowAIBakeData
		Dictionary main_payload;
		int half_cols = section_cols / 2;
		int half_rows = section_rows / 2;

		for (int r = -half_rows; r < half_rows; ++r) {
			for (int c = -half_cols; c < half_cols; ++c) {
				Vector2i coord(c, r);
				if (_sectors_database.has(coord)) {
					const FlowAISector& sector = _sectors_database[coord];

					Dictionary sector_dict;
					sector_dict["center_position"] = sector.center_position;

					Array nodes_array;
					for (int i = 0; i < sector.micro_pathnodes.size(); ++i) {
						FlowAIPathnode* node = get_pathnode_list()[i];
						if (node) {
							Dictionary node_data;
							node_data["id"] = node->get_id();
							node_data["position"] = node->get_global_position();
							node_data["links"] = node->get_links();
							nodes_array.append(node_data);
						}
					}
					sector_dict["nodes"] = nodes_array;
					main_payload[coord] = sector_dict;
				}
			}
		}

		bake_data->set_sectors_payload(main_payload);
		ResourceSaver::get_singleton()->save(bake_data);
		_draw_sections_grid();

		UtilityFunctions::print("[FlowAI] Bake complete! Nodes Baked: ", nodes_baked);
	}

	// ------------------------------------------ //
	// ------------------------------------------ //
	void FlowAIManager::add_new_pathnode(int32_t prev_pathnode_id) {
		FlowAIPathnode* new_pathnode = memnew(FlowAIPathnode);
		Node* scene_root = get_tree() ? get_tree()->get_edited_scene_root() : nullptr;

		add_child(new_pathnode);
		if (scene_root) new_pathnode->set_owner(scene_root);
		else new_pathnode->set_owner(this);

		uint32_t unique_id = _get_available_pathnode_id();
		String gd_str_unique_name = "pathnode_" + String::num_int64(unique_id);
		StringName unique_name = StringName(gd_str_unique_name);

		new_pathnode->set_id(unique_id);
		new_pathnode->set_name(unique_name);

		// If prev_pathnode_id is != -1, mean that the user is creating a new pathnode based on a selected pathnode.
		if (prev_pathnode_id != -1) {
			auto it = _pathnodes_database.find(prev_pathnode_id);
			if (it != _pathnodes_database.end()) {
				FlowAIPathnode* prev_pathnode = Object::cast_to<FlowAIPathnode>(it->second);
				if (prev_pathnode) {
					PackedInt32Array prev_node_links_arr = prev_pathnode->get_links();
					new_pathnode->set_prev_node_id(prev_pathnode->get_id());
					new_pathnode->set_position(prev_pathnode->get_position());
					prev_node_links_arr.append(unique_id);
					prev_pathnode->set_links(prev_node_links_arr);
				}
			}
		}

		// Select the new pathnode
		if (Engine::get_singleton()->is_editor_hint()) {
			if (EditorInterface* editor = EditorInterface::get_singleton()) {
				editor->edit_node(new_pathnode);
			}
		}
		
		UtilityFunctions::print("Added new pathnode.");
		return;
	}

	// ------------------------------------------ //
	// ------------------------------------------ //
	std::vector<FlowAIPathnode*> FlowAIManager::get_pathnode_list() {
		TypedArray<Node> my_children = get_children();
		std::vector<FlowAIPathnode*> arr_pathnode_list;
		_pathnodes_database.clear(); // Clear to get the updated list

		for (int i = 0; i < my_children.size(); i++) {
			FlowAIPathnode* pathnode = Object::cast_to<FlowAIPathnode>(my_children[i]);
			if (pathnode) {
				_pathnodes_database[pathnode->get_id()] = pathnode;
				arr_pathnode_list.push_back(pathnode);
			}
		}

		return arr_pathnode_list;
	}

	/////////////////////////////////////////////////////////////////////////////
	// PRIVATE
	/////////////////////////////////////////////////////////////////////////////

	void FlowAIManager::_draw_sections_grid() {
		if (imm_grid_mesh.is_null()) return;

		imm_grid_mesh->clear_surfaces();

		int half_cols = section_cols / 2;
		int half_rows = section_rows / 2;
		float min_x = -half_cols * section_size;
		float max_x = half_cols * section_size;
		float min_z = -half_rows * section_size;
		float max_z = half_rows * section_size;

		Color line_color = Color(1.0f, 0.5f, 0.0f, 0.2f);
		Color border_color = Color(1.0f, 0.2f, 0.0f, 0.6f);
		Color active_sector_color = Color(0.0f, 1.0f, 0.5f, 0.4f);

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

	// ------------------------------------------ //
	// ------------------------------------------ //
	void FlowAIManager::_draw_pathnode_connections() {}

	// ------------------------------------------ //
	// ------------------------------------------ //
	Vector2i FlowAIManager::_get_section_coords(Vector3 _global_pos) {
		// The manager is in the (0.0, 0.0, 0.0) origin.
		int col = Math::floor(_global_pos.x / (float)section_size);
		int row = Math::floor(_global_pos.z / (float)section_size);
		return Vector2i(col, row);
	}

	bool FlowAIManager::_is_within_grid_bounds(Vector2i p_coords) const {
		int half_cols = section_cols / 2;
		int half_rows = section_cols / 2;
		return (p_coords.x >= -half_cols && p_coords.x < half_cols) &&
			(p_coords.y >= -half_rows && p_coords.y < half_rows);
	}

	uint32_t FlowAIManager::_get_available_pathnode_id() {
		std::vector<FlowAIPathnode*> arr_pathnodes_list = get_pathnode_list();
		uint32_t counter_id = 0;
		while (true) {
			if (_pathnodes_database.find(counter_id) != _pathnodes_database.end()) {
				counter_id++;
				continue;
			}
			return counter_id;
		}
	}
}