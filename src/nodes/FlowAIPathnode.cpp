#include "FlowAI.hpp"
#include <godot_cpp/classes/physics_ray_query_parameters3d.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/classes/physics_direct_space_state3d.hpp>

using namespace godot;

namespace FlowAI {
	FlowAIPathnode::FlowAIPathnode() {};
	FlowAIPathnode::~FlowAIPathnode() {};

	void FlowAIPathnode::_bind_methods() {
		ClassDB::bind_method(D_METHOD("add_next_pathnode"), &FlowAIPathnode::add_next_pathnode);
		ClassDB::bind_method(D_METHOD("snap_to_ground"), &FlowAIPathnode::snap_to_ground);
		ClassDB::bind_method(D_METHOD("get_id"), &FlowAIPathnode::get_id);
		ClassDB::bind_method(D_METHOD("get_prev_node_id"), &FlowAIPathnode::get_prev_node_id);
		ClassDB::bind_method(D_METHOD("get_links"), &FlowAIPathnode::get_links);
	}

	void FlowAIPathnode::add_next_pathnode() {
		auto* parent = get_parent();
		FlowAIManager* manager = Object::cast_to<FlowAIManager>(parent);

		if (manager) {
			manager->add_new_pathnode(id);
		}
	}
	void FlowAIPathnode::snap_to_ground() {
		UtilityFunctions::print("Snap to grounndddd");
	}
}