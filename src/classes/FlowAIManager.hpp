#include "../FlowAI.hpp"


namespace FlowAI {
	class FlowAIPathnode;
	class FlowAIBakeData;

	// ----------------------------------------- //
	// manager the sections and pathnodes
	// ----------------------------------------- //
	class FlowAIManager : public Node {
		GDCLASS(FlowAIManager, Node)
	public:
		FlowAIManager();
		~FlowAIManager();

		void bake_sections();
		void add_new_pathnode(int32_t prev_pathnode_id = -1);

		void set_bake_data(const Ref<FlowAIBakeData>& p_data) { bake_data = p_data; }
		void set_section_size(uint16_t _value) { section_size = _value; }
		void set_section_rows(uint16_t _value) { section_rows = _value; }
		void set_section_cols(uint16_t _value) { section_cols = _value; }

		static FlowAIManager* get_singleton() { return singleton; }

		AStar3D* get_macro_astar() const { return astar_macro; }
		uint16_t get_section_size() const { return section_size; }
		uint16_t get_section_rows() const { return section_rows; }
		uint16_t get_section_cols() const { return section_cols; }
		Ref<FlowAIBakeData> get_bake_data() const { return bake_data; }
		std::unordered_map<unsigned int, FlowAISector> get_sectors_list();
		std::vector<FlowAIPathnode*> get_pathnode_list();
		FlowAISector* get_sector_by_id(uint32_t _id) const;
		FlowAISector* get_sector_by_coord(Vector2i _coord) const;
		FlowAISector* get_sector_by_pos(Vector3 _pos) const;
		FlowAISector* get_closest_sector_by_pos_that_have_pathnode(Vector3 _pos) const;
	protected:
		static void _bind_methods();
		void _notification(int p_what);
	private:
		static FlowAIManager* singleton;
		Ref<FlowAIBakeData> bake_data;
		AStar3D* astar_macro = nullptr;
		int section_size = 64;
		int section_rows = 8;
		int section_cols = 8;

		// FlowAIManager Previews
		MeshInstance3D* grid_preview = nullptr;
		MeshInstance3D* pathnode_connections_preview = nullptr;
		Ref<ImmediateMesh> imm_grid_mesh;
		Ref<ImmediateMesh> imm_pathnode_connections_mesh;

		// ALERT: Maybe I should put these draw functions in a debug .cpp file.
		void _draw_sections_grid(Ref<FlowAIBakeData> bake_data);
		void _draw_pathnode_connections(std::unordered_map<uint32_t, FlowAIPathnode*> database);

		void _setup_macro_astar(); // Setup micro (sections) and macro (pathnode) AStar3D.
		void _reload_database_from_bake_data();
		Vector2i _get_section_coords(Vector3 _global_pos) const;
		bool _is_within_grid_bounds(Vector2i p_coords) const;
		uint32_t _get_available_pathnode_id();
	};
}