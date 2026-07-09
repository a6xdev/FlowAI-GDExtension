#include "../FlowAI.hpp"
#include <godot_cpp/classes/label3d.hpp>

namespace FlowAI {
	// Why a Marker3D and not a Node3D? because is more easy select an marker3D in Godot Editor.
	class FlowAIPathnode : public Marker3D {
		GDCLASS(FlowAIPathnode, Marker3D)
	public:
		FlowAIPathnode();
		~FlowAIPathnode();

		void add_next_pathnode();
		void snap_to_ground();

		void set_pathnode_name(String _name) { 
			set_name(_name);
			pathnode_name_preview->set_text(_name); 
		}
		void set_disabled(bool _value) { _disabled = _value; _update_mesh_preview_colors(); }
		void set_bidirectional(bool _value) { _bidirectional = _value; };
		void set_weight_scale(float _value) { _weight_scale = _value; };

		void set_id(int32_t _id) { id = _id; };
		void set_prev_node_id(int32_t _id) { prev_pathnode_id = _id; };
		void set_links(PackedInt32Array _links) { links = _links; };
		void set_path_layers(uint32_t p_layers); // Needs update preview mesh color in cpp.
		void set_sector_id(unsigned int _id) { sector_id = _id; };
		void set_sector_coord(Vector2i _sector) { sector_coord = _sector; };
		void set_pathnode_debug(bool _bool);

		bool is_disabled() const { return _disabled; }
		bool is_bidirectional() const { return _bidirectional; };
		float get_weight_scale() const { return _weight_scale; };

		int32_t get_id() const { return id; };
		int32_t get_prev_node_id() const { return prev_pathnode_id; };
		PackedInt32Array get_links() const { return links; };
		uint32_t get_path_layers() const { return path_layers; }
		unsigned int get_sector_id() const { return sector_id; }
		Vector2i get_sector_coord() const { return sector_coord; };
	protected:
		static void _bind_methods();
		void _notification(int p_what);
	private:
		bool _disabled = false;
		bool _bidirectional = true;
		float _weight_scale = 1.0;

		int32_t id = 0;
		int32_t prev_pathnode_id = -1;
		uint32_t path_layers = 1; // bitmask (ex: Pedestrian, Vehicle, Crosswalk, etc)
		unsigned int sector_id = 0;
		Vector2i sector_coord = Vector2i(0.0, 0.0);
		PackedInt32Array links;


		MeshInstance3D* pathnode_mesh_preview = nullptr;
		Label3D* pathnode_name_preview = nullptr;

		void init_pathnode_debug();
		void _update_mesh_preview_colors();
	};
}