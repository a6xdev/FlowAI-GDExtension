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
#include <godot_cpp/classes/marker3d.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/immediate_mesh.hpp>
#include <godot_cpp/classes/a_star3d.hpp>
#include <godot_cpp/classes/character_body3d.hpp>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>

using namespace godot;

namespace FlowAI {
	struct FlowAISector {
		FlowAISector() = default;
		~FlowAISector() = default;

		unsigned int			id = 0;
		godot::Vector3			center_position;
		std::vector<uint32_t>	micro_pathnodes;
		bool					is_active;	// if its far from camera, become false

		unsigned int			get_id() const { return id; };
		Vector3					get_center_position() const { return center_position; };
		std::vector<uint32_t>	get_pathnodes() const { return micro_pathnodes; };

		bool operator<(const FlowAISector& other) const {
			return id < other.get_id();
		}
	};

	class FlowAIBakeData : public Resource {
		GDCLASS(FlowAIBakeData, Resource)
	public:
		FlowAIBakeData();
		~FlowAIBakeData();

		void set_sectors_payload(const Dictionary& p_payload) { sectors_payload = p_payload; }
		Dictionary get_sectors_payload() const { return sectors_payload; }
	private:
		Dictionary sectors_payload;
	protected:
		static void _bind_methods();
	};
}