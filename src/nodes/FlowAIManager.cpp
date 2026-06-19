#include "FlowAI.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/editor_interface.hpp>

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

	void FlowAIManager::_ready() {
		if (!Engine::get_singleton()->is_editor_hint()) {
			astar_3d = memnew(AStar3D);
			bake_sections();
		}
	}

	void FlowAIManager::_bind_methods() {
		ClassDB::bind_method(D_METHOD("bake_sections"), &FlowAIManager::bake_sections);
		ClassDB::bind_method(D_METHOD("add_new_pathnode"), &FlowAIManager::add_new_pathnode);

		ClassDB::bind_method(D_METHOD("set_section_size"), &FlowAIManager::set_section_size);
		ClassDB::bind_method(D_METHOD("set_section_rows"), &FlowAIManager::set_section_rows);
		ClassDB::bind_method(D_METHOD("set_section_cols"), &FlowAIManager::set_section_cols);

		ClassDB::bind_method(D_METHOD("get_section_size"), &FlowAIManager::get_section_size);
		ClassDB::bind_method(D_METHOD("get_section_rows"), &FlowAIManager::get_section_rows);
		ClassDB::bind_method(D_METHOD("get_section_cols"), &FlowAIManager::get_section_cols);

		ClassDB::add_property("FlowAIManager", PropertyInfo(Variant::INT, "section_size"), "set_section_size", "get_section_size");
		ClassDB::add_property("FlowAIManager", PropertyInfo(Variant::INT, "section_rows"), "set_section_rows", "get_section_rows");
		ClassDB::add_property("FlowAIManager", PropertyInfo(Variant::INT, "section_cols"), "set_section_cols", "get_section_cols");
	}

	void active_section(std::vector<uint32_t> micro_pathnodes) {

	}

	void disable_section(std::vector<uint32_t> micro_pathnodes) {

	}

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
	void FlowAIManager::bake_sections() {
		std::vector<FlowAIPathnode*> all_pathnodes = get_pathnode_list();
		_sectors_database.clear();

		// Populates the HashMap by pre-creating empty sections within the specified limit.
		for (int r = 0; r < section_rows; ++r) {
			for (int c = 0; c < section_cols; ++c) {
				Vector2i coord(c, r);
				FlowAISector new_sector = FlowAISector();
				new_sector.center_position = Vector3(coord.x, 0.0, coord.y);
				_sectors_database[coord] = FlowAISector();
			}
		}

		int nodes_baked = 0;
		int nodes_out_of_bounds = 0;
		
		for (int i = 0; i < all_pathnodes.size(); ++i) {
			FlowAIPathnode* pathnode = Object::cast_to<FlowAIPathnode>(all_pathnodes[i]);
			if (!pathnode) continue;

			// Calculate mathematically which square this node is stepping on
			Vector2i node_sector = _get_section_coords(pathnode->get_global_position());
			UtilityFunctions::print("Pathnode: ", pathnode->get_name(), ", Section Coords : ", node_sector);

			// If its within the grid limits, add it to that section list.
			if (_is_within_grid_bounds(node_sector)) {
				_sectors_database[node_sector].micro_pathnodes.push_back(pathnode->get_id());
				pathnode->set_sector(_sectors_database[node_sector]);
				nodes_baked++;
			}
			else {
				nodes_out_of_bounds++;
			}
		}

		UtilityFunctions::print("[FlowAI] Bake complete! Nodes Baked: ", nodes_baked);
	}

	Vector2i FlowAIManager::_get_section_coords(Vector3 _global_pos) {
		// The manager is in the (0.0, 0.0, 0.0) origin.
		int col = Math::floor(_global_pos.x / (float)section_size);
		int row = Math::floor(_global_pos.z / (float)section_size);
		return Vector2i(col, row);
	}

	bool FlowAIManager::_is_within_grid_bounds(Vector2i p_coords) const {
		return (p_coords.x >= 0 && p_coords.x < section_cols) &&
			(p_coords.y >= 0 && p_coords.y < section_rows);
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