#include "../classes/FlowAIManager.hpp"
#include "../classes/FlowAIPathnode.hpp"
#include "../FlowAIDebug.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/resource_saver.hpp>
#include <godot_cpp/classes/label3d.hpp>

Color line_color = Color(1.0f, 0.0f, 0.0f, 0.5f);

namespace FlowAI {
	FlowAIManager* FlowAIManager::singleton = nullptr;
	HashMap<Vector2i, FlowAISector> m_sectors_database;
	std::unordered_map<uint32_t, FlowAIPathnode*> m_pathnodes_database;

	FlowAIManager::FlowAIManager() { if (singleton == nullptr) singleton = this; };
	FlowAIManager::~FlowAIManager() { if (singleton == this) singleton = nullptr; };

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
			set_process(true);
			
			// Section Grid Preview
			if (grid_preview == NULL && FlowAIDebug::draw_sections_grid == true) {
				grid_preview = memnew(MeshInstance3D);
				imm_grid_mesh.instantiate();
				grid_preview->set_mesh(imm_grid_mesh);
				add_child(grid_preview);
			}

			// Pathnode Connections Preview
			if (pathnode_connections_preview == NULL && FlowAIDebug::draw_pathnode_connections_grid == true) {
				pathnode_connections_preview = memnew(MeshInstance3D);
				imm_pathnode_connections_mesh.instantiate();
				pathnode_connections_preview->set_mesh(imm_pathnode_connections_mesh);
				add_child(pathnode_connections_preview);
			}

			if (!Engine::get_singleton()->is_editor_hint()) {
				// Start Manager in Runtime
				_reload_database_from_bake_data();
				_setup_macro_astar();
			}
			else {
				get_pathnode_list(); // update "m_pathnodes_database"
			}

			if (!bake_data.is_null()) { 
				_draw_sections_grid(bake_data);
			}
			break;
		case NOTIFICATION_PROCESS:
			if (m_pathnodes_database.size() > 1) {
				_draw_pathnode_connections(m_pathnodes_database);
			}
			break;
		case NOTIFICATION_EXIT_TREE:
			break;
		}
	}


	/////////////////////////////////////////////////////////////////////////////
	// EDITOR
	/////////////////////////////////////////////////////////////////////////////

	void FlowAIManager::bake_sections() {
		if (bake_data.is_null()) {
			UtilityFunctions::print("[FlowAI] ERROR::BAKE_DATA::IS_EMPTY");
			return;
		}

		unsigned int section_id_counter = 0;
		int half_cols = section_cols / 2;
		int half_rows = section_rows / 2;
		int nodes_baked = 0;
		int nodes_out_of_bounds = 0;
		std::vector<FlowAIPathnode*> _pathnodes = get_pathnode_list();
		HashMap<Vector2i, FlowAISector> _sectors;
		Dictionary main_payload;

		// Populates the HashMap by pre-creating empty sections within the specified limit.
		for (int r = -half_rows; r < half_rows; ++r) {
			for (int c = -half_cols; c < half_cols; ++c) {
				Vector2i coord(c, r);
				FlowAISector new_sector = FlowAISector();
				new_sector.id = section_id_counter;
				new_sector.center_position = Vector3(c * section_size + (section_size / 2.0f), 0.0f, r * section_size + (section_size / 2.0f));
				_sectors[coord] = new_sector;
				section_id_counter++;
			}
		}

		// Set pathnode sector coord 
		for (int i = 0; i < _pathnodes.size(); ++i) {
			FlowAIPathnode* pathnode = Object::cast_to<FlowAIPathnode>(_pathnodes[i]);
			if (!pathnode) continue;

			// Calculate mathematically which square this node is stepping on
			Vector2i node_sector = _get_section_coords(pathnode->get_global_position());
			
			// If its within the grid limits, add it to that section list.
			if (_is_within_grid_bounds(node_sector)) {
				_sectors[node_sector].micro_pathnodes.push_back(pathnode->get_id());
				pathnode->set_sector_id(_sectors[node_sector].get_id());
				pathnode->set_sector_coord(node_sector);
				UtilityFunctions::print("[FlowAI] Pathnode: ", pathnode->get_name(), " | Section Coords : ", node_sector, " | Section ID: ", _sectors[node_sector].get_id());
				nodes_baked++;
			}
			else {
				nodes_out_of_bounds++;
			}
		}

		for (int r = -half_rows; r < half_rows; ++r) {
			for (int c = -half_cols; c < half_cols; ++c) {
				Vector2i coord(c, r);
				if (_sectors.has(coord)) {
					const FlowAISector& sector = _sectors[coord];

					Dictionary sector_dict;
					sector_dict["sector_id"] = sector.id;
					sector_dict["center_position"] = sector.center_position;

					Array nodes_array;
					for (uint32_t node_id : sector.get_pathnodes()) {
						nodes_array.append(node_id);
					}
					sector_dict["micro_pathnodes"] = nodes_array;
					main_payload[coord] = sector_dict;
				}
			}
		}

		bake_data->set_sectors_payload(main_payload);
		ResourceSaver::get_singleton()->save(bake_data);
		_draw_sections_grid(bake_data);

		UtilityFunctions::print("[FlowAI] Bake complete! Nodes Baked: ", nodes_baked);
	}

	// Add a new pathnode in the SceneTree.
	// Isn't stored in the pathnode_database because we use the SceneTree itself -
	// as the definitive source for the list of created pathnodes.
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
			auto it = m_pathnodes_database.find(prev_pathnode_id);
			if (it != m_pathnodes_database.end()) {
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

		UtilityFunctions::print("[FlowAI] Added new pathnode.");
		return;
	}

	/////////////////////////////////////////////////////////////////////////////
	// RUNTIME
	/////////////////////////////////////////////////////////////////////////////
	
	// Setup micro (sections) and macro (pathnode) AStar3D.
	void FlowAIManager::_setup_macro_astar() {
		// this function is used after _reload_data_from_bake_data() function because 
		// of the sections and pathnodes database.

		astar_macro = memnew(AStar3D);

		for (const auto &E : m_sectors_database) {
			Vector2i coord = E.key;
			FlowAISector sector_ref = E.value;
			astar_macro->add_point(sector_ref.get_id(), sector_ref.get_center_position());
		}

		// Connect neighbor sections based on micro (pathnodes connections)
		for (const auto& E : m_pathnodes_database) {
			FlowAIPathnode* current_node = E.second;
			if (!current_node) continue;

			unsigned int current_sector_id = current_node->get_sector_id();
			PackedInt32Array linked_ids = current_node->get_links();

			for (int j = 0; j < linked_ids.size(); ++j) {
				uint32_t target_node_id = linked_ids[j];

				auto it = m_pathnodes_database.find(target_node_id);
				if (it != m_pathnodes_database.end()) {
					FlowAIPathnode* target_node = it->second;

					if (target_node) {
						unsigned int target_sector_id = target_node->get_sector_id();
						if (current_sector_id != target_sector_id) {
							astar_macro->connect_points(current_sector_id, target_sector_id, true);
							//UtilityFunctions::print("[FlowAI] AStar Macro Points: ", current_sector_id, ", and ", target_sector_id, " has Connected");
						}
					}
				}
			}
		}

		UtilityFunctions::print("[FlowAI] Macro Graph generated and neighbors connected.");
	}

	// set section_database and pathnode_database based on FlowAIBakeData
	// Used only in runtime functions.
	void FlowAIManager::_reload_database_from_bake_data() {
		if (bake_data.is_null()) return;

		m_sectors_database.clear();
		m_pathnodes_database.clear();

		Dictionary main_payload = bake_data->get_sectors_payload();
		Array sector_coords = main_payload.keys();

		// update m_pathnodes_database
		get_pathnode_list();

		for (int i = 0; i < sector_coords.size(); i++) {
			Vector2i coord = sector_coords[i];
			Dictionary data = main_payload[coord];

			FlowAISector runtime_sector;
			runtime_sector.id = data["sector_id"];
			runtime_sector.center_position = data["center_position"];
			runtime_sector.is_active = false; // Define false as native
			Array pathnodes_arr = data["micro_pathnodes"];

			for (int j = 0; j < pathnodes_arr.size(); j++) {
				runtime_sector.micro_pathnodes.push_back((uint32_t)pathnodes_arr[j]);
			}
			m_sectors_database[coord] = runtime_sector;

			// Debug
			if (FlowAIDebug::section_debug) {
				Label3D* new_label = memnew(Label3D);
				new_label->set_text(String::num_int64(runtime_sector.get_id()));
				new_label->set_billboard_mode(BaseMaterial3D::BillboardMode::BILLBOARD_FIXED_Y);
				new_label->set_pixel_size(0.036);
				add_child(new_label);
				new_label->set_global_position(Vector3(runtime_sector.center_position.x, runtime_sector.center_position.y + 3, runtime_sector.center_position.z));
			}
		}
	}

	///////////////////////////////////////////////////////////////////////////
	// CALLS
	/////////////////////////////////////////////////////////////////////////////

	std::unordered_map<unsigned int, FlowAISector> FlowAIManager::get_sectors_list() {
		std::unordered_map<unsigned int, FlowAISector> list;
		for (auto &E : m_sectors_database) {
			// It Only return sectors that have pathnodes!
			// Maybe i can change it one day.
			// I dit it because in get_random_path, its more functional get only the sections that have pathnodes.
			if (E.value.micro_pathnodes.size() > 0) list[E.value.get_id()] = E.value;
		}
		return list;
	}

	// Works in editor and runtime
	std::vector<FlowAIPathnode*> FlowAIManager::get_pathnode_list() {
		TypedArray<Node> my_children = get_children();
		std::vector<FlowAIPathnode*> arr_pathnode_list;
		m_pathnodes_database.clear();

		for (int i = 0; i < my_children.size(); i++) {
			FlowAIPathnode* pathnode = Object::cast_to<FlowAIPathnode>(my_children[i]);
			if (pathnode) {
				m_pathnodes_database[pathnode->get_id()] = pathnode;
				arr_pathnode_list.push_back(pathnode);
			}
		}
		return arr_pathnode_list;
	}

	// this function can only be used on runtime.
	FlowAISector* FlowAIManager::get_sector_by_coord(Vector2i _coord) const {
		auto it = m_sectors_database.find(_coord);
		if (it != m_sectors_database.end()) {
			return &(it->value);
		}
		return nullptr;
	}

	// this function can only be used on runtime.
	FlowAISector* FlowAIManager::get_sector_by_pos(Vector3 _pos) const {
		Vector2i coord = _get_section_coords(_pos);
		auto it = m_sectors_database.find(coord);
		if (it != m_sectors_database.end()) {
			return &(it->value);
		}
		return nullptr;
	}

	FlowAISector* FlowAIManager::get_closest_sector_by_pos_that_have_pathnode(Vector3 _pos) const {
		Vector2i start_coord = _get_section_coords(_pos);
		auto it_start = m_sectors_database.find(start_coord);
		if (it_start != m_sectors_database.end()) {
			FlowAISector& current_sector = it_start->value;
			if (!current_sector.micro_pathnodes.empty()) {
				return &current_sector;
			}
		}

		// If the current_sector.micro_pathnode is empty: Get the closest sector that isnt empty!
		FlowAISector* closest_sector = nullptr;
		float min_dist = INFINITY;

		for (auto& E : m_sectors_database) {
			FlowAISector& sector = E.value;
			if (sector.micro_pathnodes.empty()) continue;
			float dist = _pos.distance_to(sector.get_center_position());
			if (dist < min_dist) {
				min_dist = dist;
				closest_sector = &sector;
			}
		}

		return closest_sector;
	}

	// Discover which section the pathnode position is in
	// The origin is always (0.0, 0.0, 0.0).
	Vector2i FlowAIManager::_get_section_coords(Vector3 _global_pos) const {
		int col = Math::floor(_global_pos.x / (float)section_size);
		int row = Math::floor(_global_pos.z / (float)section_size);
		return Vector2i(col, row);
	}

	// Check if pathnode position is in within grid bounds
	bool FlowAIManager::_is_within_grid_bounds(Vector2i p_coords) const {
		int half_cols = section_cols / 2;
		int half_rows = section_cols / 2;
		return (p_coords.x >= -half_cols && p_coords.x < half_cols) &&
			(p_coords.y >= -half_rows && p_coords.y < half_rows);
	}

	// Get a available ID.
	uint32_t FlowAIManager::_get_available_pathnode_id() {
		std::vector<FlowAIPathnode*> arr_pathnodes_list = get_pathnode_list();
		uint32_t counter_id = 0;

		while (true) {
			if (m_pathnodes_database.find(counter_id) != m_pathnodes_database.end()) {
				counter_id++;
				continue;
			}
			return counter_id;
		}
	}
}