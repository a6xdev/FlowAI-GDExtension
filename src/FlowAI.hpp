#pragma once

#include <godot_cpp/classes/os.hpp>
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
#include <godot_cpp/classes/standard_material3d.hpp>
#include <vector>
#include <map>
#include <unordered_map>
#include <string>

using namespace godot;

namespace FlowAI {
	enum DebugOption {
		DEBUG_ENABLE_LOGGING,
		DEBUG_VISUALIZE_PATHNODE,
		DEBUG_VISUALIZE_SECTION,
		DEBUG_VISUALIZE_CONNECTIONS,
		DEBUG_VISUALIZE_AGENT_PATH,
	};

	inline String get_d_enable_logging() { return "flow_ai/debug/enable_logging"; }
	inline String get_d_visualize_pathnode_debug() { return "flow_ai/debug/visualize_pathnode_debug"; }
	inline String get_d_visualize_connections() { return "flow_ai/debug/visualize_connections"; }
	inline String get_d_visualize_section_debug() { return "flow_ai/debug/visualize_section_debug"; }
	inline String get_d_visualize_agent_path() { return "flow_ai/debug/visualize_agent_path"; }

	inline Color get_agent_path_line_color() { return Color(1.0f, 0.0f, 0.0f, 0.5f); }
	inline Color pathnode_connections_color() { return Color(0.157f, 0.157f, 0.769f, 0.8f); }
	inline Color get_grid_main_color() { return Color(0.0f, 1.0f, 0.5f, 0.4f); }
	inline Color get_active_sector_color() { return Color(0.0f, 0.5f, 0.5f, 0.4f); }

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

	//void HelloWorld(std::string _text = "print") { return; }

	bool is_debug_enabled(DebugOption p_option);
	static bool is_debug_build() { return OS::get_singleton()->is_debug_build(); }
	Color get_color_from_navigation_layer_mask(uint32_t _layer_mask);
}