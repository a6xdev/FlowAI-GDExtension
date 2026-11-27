#include "../classes/FlowAIManager.hpp"
#include "../classes/FlowAIPathnode.hpp"
#include "../classes/FlowAILayer.hpp"

#include <unordered_set>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/resource_saver.hpp>
#include <godot_cpp/classes/label3d.hpp>

Color line_color = Color(1.0f, 0.0f, 0.0f, 0.5f);

namespace FlowAI {
	FlowAIManager* FlowAIManager::singleton = nullptr;
	HashMap<Vector2i, FlowAISector> m_sectors_database;
	std::vector<FlowAILayer*> m_layers_database;

	FlowAIManager::FlowAIManager() { if (singleton == nullptr) singleton = this; };
	FlowAIManager::~FlowAIManager() { if (singleton == this) singleton = nullptr; };

	void FlowAIManager::_bind_methods() {
		ClassDB::bind_method(D_METHOD("bake_sections"), &FlowAIManager::bake_sections);

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
		if (Engine::get_singleton()->is_editor_hint()) {
		}

		switch (p_what) {

		// ENTER TREE
		case NOTIFICATION_ENTER_TREE:
			set_process(true);
			
			// Section Grid Preview
			if (grid_preview == NULL) {
				grid_preview = memnew(MeshInstance3D);
				imm_grid_mesh.instantiate();
				grid_preview->set_mesh(imm_grid_mesh);
				add_child(grid_preview);
			}

			// Pathnode Connections Preview
			if (pathnode_connections_preview == NULL) {
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
				m_layers_database = get_layers_list(); // update "m_pathnodes_database"
			}

			if (!bake_data.is_null()) { 
				//_draw_sections_grid(bake_data); 
			}
			break;

		// PROCESS
		case NOTIFICATION_PROCESS:
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
		
		UtilityFunctions::print("[FlowAI] Bake complete! Nodes Baked: ", nodes_baked);
	}

	void FlowAIManager::connect_pathnodes(FlowAIPathnode* _fromNode, FlowAIPathnode* _toNode) {
		if (_fromNode && _toNode) {
			PackedInt32Array link_data = _toNode->get_links();
			if (!link_data.has(_toNode->get_id())) {
				link_data.push_back(_toNode->get_id());
				_fromNode->set_links(link_data);
				UtilityFunctions::print("[FlowAI] Pathnodes '", _fromNode->get_name(), "' and '", _toNode->get_name(), " were connected!");
			}
		}
	}

	void FlowAIManager::disconnect_pathnodes(FlowAIPathnode* _fromNode, FlowAIPathnode* _toNode) {
		if (_fromNode && _toNode) {
			PackedInt32Array link_data = _fromNode->get_links();
			if (link_data.has(_toNode->get_id())) {
				link_data.erase(_toNode->get_id());
				_fromNode->set_links(link_data);
				UtilityFunctions::print("[FlowAI] Pathnodes '", _fromNode->get_name(), "' and '", _toNode->get_name(), " were disconnected!");
			}
		}
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
		for (const auto& layer : m_layers_database) {
			for (const auto& current_node : layer->get_pathnode_list()) {
				if (!current_node) continue;

				unsigned int current_sector_id = current_node->get_sector_id();
				PackedInt32Array linked_ids = current_node->get_links();

				for (int j = 0; j < linked_ids.size(); ++j) {
					uint32_t target_node_id = linked_ids[j];

					FlowAIPathnode* target_node = layer->get_pathnode(target_node_id);
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
		m_layers_database.clear();

		Dictionary main_payload = bake_data->get_sectors_payload();
		Array sector_coords = main_payload.keys();

		// update m_layers_database
		m_layers_database = get_layers_list();
		
		// Create all sectors
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
	// Dont get pathnodes that are disabled
	std::vector<FlowAILayer*> FlowAIManager::get_layers_list() {
		TypedArray<Node> my_children = get_children();
		std::vector<FlowAILayer*> arr_layer_list;
		m_layers_database.clear();

		for (int i = 0; i < my_children.size(); i++) {
			FlowAILayer* layer = Object::cast_to<FlowAILayer>(my_children[i]);
			if (layer) { arr_layer_list.push_back(layer); }
		}
		return arr_layer_list;
	}

	// Get all pathnodes from all layers.
	std::vector<FlowAIPathnode*> FlowAIManager::get_pathnode_list() {
		std::vector<FlowAIPathnode*> _pathnodes;
		for (const auto& layer : get_layers_list()) {
			for (const auto& pathnode : layer->get_pathnode_list()) {
				_pathnodes.push_back(pathnode);
			}
		}
		return _pathnodes;
	}

	FlowAILayer* FlowAIManager::get_layer_by_name(const StringName& _layer_name) {
		if (_layer_name.is_empty()) {
			return nullptr;
		}

		for (auto& it : get_layers_list()) {
			if (it->get_name() == _layer_name) {
				return it;
			}
		}

		return nullptr;
	}

	// this function can only be used on runtime.
	FlowAISector* FlowAIManager::get_sector_by_coord(Vector2i _coord) const {
		auto it = m_sectors_database.find(_coord);
		if (it != m_sectors_database.end()) {
			return &(it->value);
		}
		return nullptr;
	}

	FlowAISector* FlowAIManager::get_sector_by_id(uint32_t _id) const {
		for (auto& E : m_sectors_database) {
			if (E.value.get_id() == _id) {
				return &(E.value);
			}
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

	void FlowAIManager::_sanitize_all_links() {
		std::vector<FlowAILayer*> layer_list = get_layers_list();

		std::unordered_set<uint32_t> valid_ids;
		for (auto layer : layer_list) {
			for (auto pathnode : layer->get_pathnode_list()) {
				valid_ids.insert(pathnode->get_id());
			}
		}

		for (auto layer : layer_list) {
			PackedInt32Array  valid_links;
			for (auto pathnode : layer->get_pathnode_list()) {
				for (auto link_id : pathnode->get_links()) {
					if (valid_ids.count(link_id) > 0) {
						valid_links.append(link_id);
					}
				}
				pathnode->set_links(valid_links);
			}
		}
	}

}