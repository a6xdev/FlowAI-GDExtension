#pragma once

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/variant/quaternion.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <vector>
#include <unordered_map>
#include <string>

namespace godot {
	class FlowAIPathnode : public Node3D {
		GDCLASS(FlowAIPathnode, Node3D)
	public:
		uint32_t id = 0;
		uint32_t prev_pathnode_id = 0;
		uint32_t sector_id = -1;
		uint32_t context_layers; // bitmask (ex: Pedestrian, Vehicle, Crosswalk, etc)
		float corridor_width = 1.0;
		PackedInt32Array links;
	protected:
		static void _bind_methods() {};
	};


	class FlowAIManager : public Node {
		GDCLASS(FlowAIManager, Node)

	public:
		FlowAIManager();
		~FlowAIManager();

		void _enter_tree() override {};
		void _process(double delta) override {};

		void add_new_pathnode(uint32_t prev_pathnode_id = 0);
	protected:
		static void _bind_methods();
	private:
		std::unordered_map<uint32_t, FlowAIPathnode*> _pathnodes_database;

		std::vector<FlowAIPathnode*> _arr_get_pathnode_list();
		uint32_t _get_available_pathnode_id();
	};
}

struct FlowAISector {
	uint32_t id;
	godot::Vector3 center_position;
	std::vector<uint32_t> micro_nodes;
	std::vector<uint32_t> neighbor_sectors;   // macro sectors neighbors
	bool is_active;                           // if its far from camera, become false
};