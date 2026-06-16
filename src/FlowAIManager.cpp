#include "FlowAI.hpp"

namespace godot {
	void FlowAIManager::_bind_methods() {
		ClassDB::bind_method(D_METHOD("add_new_pathnode"), &FlowAIManager::add_new_pathnode);
	}

	FlowAIManager::FlowAIManager() {};
	FlowAIManager::~FlowAIManager() {};

	void FlowAIManager::add_new_pathnode(uint32_t prev_pathnode_id) {
		FlowAIPathnode* new_pathnode = memnew(FlowAIPathnode);
		Node* scene_root = get_tree() ? get_tree()->get_edited_scene_root() : nullptr;

		add_child(new_pathnode);
		if (scene_root) new_pathnode->set_owner(scene_root);
		else new_pathnode->set_owner(this);

		uint32_t unique_id = _get_available_pathnode_id();
		String gd_str_unique_name = "pathnode_" + String::num_int64(unique_id);
		StringName unique_name = StringName(gd_str_unique_name);

		new_pathnode->id = unique_id;
		new_pathnode->set_name(unique_name);

		auto it = _pathnodes_database.find(prev_pathnode_id);
		if (it != _pathnodes_database.end()) {
			FlowAIPathnode* prev_pathnode = Object::cast_to<FlowAIPathnode>(it->second);
			if (prev_pathnode) {
				new_pathnode->prev_pathnode_id = prev_pathnode->id;
				new_pathnode->set_position(prev_pathnode->get_position());
				prev_pathnode->links.append(unique_id);
			}
		}

		UtilityFunctions::print("Added new pathnode.");
		return;
	}

	// PRIVATE
	std::vector<FlowAIPathnode*> FlowAIManager::_arr_get_pathnode_list() {
		TypedArray<Node> my_children = get_children();
		std::vector<FlowAIPathnode*> arr_pathnode_list;
		_pathnodes_database.clear(); // Clear to get the updated list

		for (int i = 0; i < my_children.size(); i++) {
			FlowAIPathnode* pathnode = Object::cast_to<FlowAIPathnode>(my_children[i]);
			if (pathnode) {
				_pathnodes_database[pathnode->id] = pathnode;
				arr_pathnode_list.push_back(pathnode);
			}
		}

		return arr_pathnode_list;
	}

	uint32_t FlowAIManager::_get_available_pathnode_id() {
		std::vector<FlowAIPathnode*> arr_pathnodes_list = _arr_get_pathnode_list();
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