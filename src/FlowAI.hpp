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
#include <vector>
#include <unordered_map>
#include <string>

#include "resources/FlowAIBakeData.hpp"

using namespace godot;

namespace FlowAI {
	struct FlowAISector {
		godot::Vector3 center_position;
		std::vector<uint32_t> micro_pathnodes;
		std::vector<uint32_t> neighbor_sectors;   // macro sectors neighbors
		bool is_active;                           // if its far from camera, become false
	};

	// Why a Marker3D and not a Node3D? because is more easy select an marker3D in Godot Editor.
	class FlowAIPathnode : public Marker3D {
		GDCLASS(FlowAIPathnode, Marker3D)
	public:
		FlowAIPathnode();
		~FlowAIPathnode();

		void add_next_pathnode();
		void snap_to_ground();

		void set_id(int32_t _id) { id = _id; };
		void set_prev_node_id(int32_t _id) { prev_pathnode_id = _id; };
		void set_links(PackedInt32Array _links) { links = _links; };
		void set_path_layers(uint32_t p_layers); // Needs update preview mesh color in cpp.
		void set_sector_coord(Vector2i _sector) { sector_coord = _sector; };
		void set_bidirectional(bool _value) { m_bidirectional = _value; };

		int32_t get_id() const { return id; };
		int32_t get_prev_node_id() const { return prev_pathnode_id; };
		PackedInt32Array get_links() const { return links; };
		uint32_t get_path_layers() const { return path_layers; }
		Vector2i get_sector_coord() const { return sector_coord; };
		bool is_bidirectional() const { return m_bidirectional; };
	protected:
		static void _bind_methods();
		void _notification(int p_what);
	private:
		int32_t id = 0;
		int32_t prev_pathnode_id = -1;
		uint32_t path_layers = 1; // bitmask (ex: Pedestrian, Vehicle, Crosswalk, etc)
		Vector2i sector_coord = Vector2i(0.0, 0.0);
		PackedInt32Array links;
		bool m_bidirectional = true;

		MeshInstance3D* mesh_preview = nullptr;
		MeshInstance3D* line_preview = nullptr;
		Ref<ImmediateMesh> imm_mesh;

		void _update_mesh_preview_colors();
		void _redraw_connections();
	};

	// ----------------------------------------- //
	// manager the sections and pathnodes
	// ----------------------------------------- //
	class FlowAIManager : public Node {
		GDCLASS(FlowAIManager, Node)

	public:
		FlowAIManager();
		~FlowAIManager();

		void active_section(std::vector<uint32_t> micro_pathnodes);
		void disable_section(std::vector<uint32_t> micro_pathnodes);

		void bake_sections();
		void add_new_pathnode(int32_t prev_pathnode_id = -1);

		void set_bake_data(const Ref<FlowAIBakeData>& p_data) { bake_data = p_data; }
		void set_section_size(uint16_t _value) { section_size = _value; }
		void set_section_rows(uint16_t _value) { section_rows = _value; }
		void set_section_cols(uint16_t _value) { section_cols = _value; }

		static FlowAIManager* get_singleton() { return singleton; }

		AStar3D* get_astar() const { return astar_3d; }
		uint16_t get_section_size() const { return section_size; }
		uint16_t get_section_rows() const { return section_rows; }
		uint16_t get_section_cols() const { return section_cols; }
		Ref<FlowAIBakeData> get_bake_data() const { return bake_data; }
		std::vector<FlowAIPathnode*> get_pathnode_list();
	protected:
		static void _bind_methods();
		void _notification(int p_what);
	private:
		static FlowAIManager* singleton;
		Ref<FlowAIBakeData> bake_data;
		AStar3D* astar_3d = nullptr;
		uint16_t section_size = 64;
		uint16_t section_rows = 8;
		uint16_t section_cols = 8;
		MeshInstance3D* grid_preview = nullptr;
		Ref<ImmediateMesh> imm_grid_mesh;

		std::unordered_map<uint32_t, FlowAIPathnode*> _pathnodes_database;
		HashMap<Vector2i, FlowAISector> _sectors_database;

		void _draw_sections_grid();
		void _draw_pathnode_connections();

		Vector2i _get_section_coords(Vector3 _global_pos);
		bool _is_within_grid_bounds(Vector2i p_coords) const;
		uint32_t _get_available_pathnode_id();
	};

	// ----------------------------------------- //
	// Mr. Bond (our Agent lol)
	// ----------------------------------------- //
	class FlowAIAgent3D : public Node {
		GDCLASS(FlowAIAgent3D, Node)
	public:
		FlowAIAgent3D();
		~FlowAIAgent3D();

		void set_target_pathnode(FlowAIPathnode* pathnode);
		void get_random_path();
		Vector3 get_next_pathnode_position();
	protected:
		static void _bind_methods();
		void _notification(int p_what);
	private:
		PackedVector3Array find_path(FlowAIPathnode* start, FlowAIPathnode* target);
		Array get_current_sections_path();
		Array get_current_pathnodes_path();
	};
}
