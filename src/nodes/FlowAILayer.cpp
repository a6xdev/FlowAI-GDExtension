#include "../classes/FlowAILayer.hpp"
#include "../classes/FlowAIPathnode.hpp"
#include "../classes/FlowAIManager.hpp"

#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/resource_saver.hpp>

#include <unordered_set>

namespace FlowAI {
	std::unordered_map<uint32_t, FlowAIPathnode*> m_layer_pathnodes_database;

	FlowAILayer::FlowAILayer() {}
	FlowAILayer::~FlowAILayer() {}

	void FlowAILayer::_bind_methods() {
		ClassDB::bind_method(D_METHOD("create_pathnode"), &FlowAILayer::create_pathnode);
		ClassDB::bind_method(D_METHOD("set_layer_color"), &FlowAILayer::set_layer_color);

		//ClassDB::bind_method(D_METHOD("get_pathnode"), &FlowAILayer::get_pathnode);
		ClassDB::bind_method(D_METHOD("get_pathnode_list"), &FlowAILayer::get_pathnode_list_gd);
		ClassDB::bind_method(D_METHOD("get_layer_color"), &FlowAILayer::get_layer_color);

		ADD_PROPERTY(PropertyInfo(Variant::COLOR, "layer_color"), "set_layer_color", "get_layer_color");
	}

	void FlowAILayer::_notification(int p_what) {}

	void FlowAILayer::create_pathnode(int32_t _prevPathnodeID) {
		FlowAIPathnode* new_pathnode = memnew(FlowAIPathnode);
		Node* scene_root = get_tree() ? get_tree()->get_edited_scene_root() : nullptr;

		add_child(new_pathnode);
		if (scene_root) new_pathnode->set_owner(scene_root);
		else new_pathnode->set_owner(this);

		uint32_t unique_id = get_available_pathnode_id();
		StringName unique_name = get_name() + StringName("_pathnode_" + String::num_int64(unique_id));

		new_pathnode->set_id(unique_id);
		new_pathnode->set_pathnode_name(unique_name);

		// If prev_pathnode_id == -1, mean that the user is creating a new pathnode based on a selected pathnode.
		if (_prevPathnodeID != -1) {
			auto it = m_layer_pathnodes_database.find(_prevPathnodeID);
			if (it != m_layer_pathnodes_database.end()) {
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

		// Select the new pathnode in the godot editor
		if (Engine::get_singleton()->is_editor_hint()) {
			if (EditorInterface* editor = EditorInterface::get_singleton()) {
				editor->edit_node(new_pathnode);
			}
		}

		UtilityFunctions::print("[FlowAI] Added new pathnode: ", unique_name);
	}

	FlowAIPathnode* FlowAILayer::get_pathnode(uint32_t _id) {
		return m_layer_pathnodes_database[_id];
	}

	std::vector<FlowAIPathnode*> FlowAILayer::get_pathnode_list() {
		TypedArray<Node> my_children = get_children();
		std::vector<FlowAIPathnode*> arr_pathnode_list;
		m_layer_pathnodes_database.clear();

		for (int i = 0; i < my_children.size(); i++) {
			FlowAIPathnode* pathnode = Object::cast_to<FlowAIPathnode>(my_children[i]);
			if (pathnode) {
				m_layer_pathnodes_database[pathnode->get_id()] = pathnode;
				arr_pathnode_list.push_back(pathnode);
			}
		}
		return arr_pathnode_list;
	}
	
	TypedArray<FlowAIPathnode> FlowAILayer::get_pathnode_list_gd() {
		TypedArray<FlowAIPathnode> result;

		for (auto& pathnode : get_pathnode_list()) {
			result.push_back(pathnode);
		}

		return result;
	}

	uint32_t FlowAILayer::get_available_pathnode_id() {
		std::vector<FlowAIPathnode*> arr_pathnodes_list = get_pathnode_list();
		uint32_t counter_id = 0;

		while (true) {
			if (m_layer_pathnodes_database.find(counter_id) != m_layer_pathnodes_database.end()) {
				counter_id++;
				continue;
			}
			return counter_id;
		}
	}
}