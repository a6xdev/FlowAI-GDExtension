#include "../FlowAI.hpp"

namespace FlowAI {
	class FlowAIPathnode;
	class FlowAIManager;

	enum FlowAIPathResult {
		PATH_FOUND = 0,
		PATH_NOT_FOUND = 1,
		PATH_LAYER_BLOCKED = 2,
	};

	// ----------------------------------------- //
	// Mr. Bond (our Agent lol)
	// ----------------------------------------- //
	class FlowAIAgent3D : public Node {
		GDCLASS(FlowAIAgent3D, Node)
	public:
		FlowAIAgent3D();
		~FlowAIAgent3D();

		bool is_path_complete() const { return path_complete; }

		FlowAIPathResult set_target_pathnode(FlowAIPathnode* target_pathnode, String layer_mask);
		FlowAIPathResult set_random_path();
		void set_path_desired_distance(float _value) { m_path_desired_distance = _value; }
		void set_layer_name(String _layer) { m_layer_name = _layer; }

		Vector3 get_next_pathnode_position();
		FlowAIManager* get_current_manager() const;
		FlowAIPathnode* get_pathnode_closest_to_pos(const Vector3 _pos) const;
		float get_path_desired_distance() const { return m_path_desired_distance; }
		String get_layer_name() const { return m_layer_name; }
	protected:
		static void _bind_methods();
		void _notification(int p_what);
	private:
		// Agent Config:
		String m_layer_name = "FlowAILayer";
		float m_path_desired_distance = 1.0f;

		// Others
		CharacterBody3D* actor_owner = nullptr;
		AStar3D* astar_macro = nullptr;
		HashMap<unsigned int, FlowAISector*> current_sectors_in_corridor;
		PackedVector3Array current_sectors_path;
		PackedVector3Array current_pathnodes_path;
		std::unordered_map<unsigned int, FlowAISector> manager_sector_list;
		std::unordered_map<uint32_t, FlowAIPathnode*> manager_pathnode_list;
		int current_path_index = 0;
		bool path_complete = false;

		FlowAIPathResult request_path(Vector3 _pos_target, String _layer_name);
		FlowAIPathResult generate_pathnode_path(Vector3 _pos, FlowAISector* start_sector, FlowAISector* end_sector, String _layer_name);
		bool try_build_micro_path(Vector3 _pos, String _layer_name);

		void set_next_path_index();

		FlowAIPathnode* get_pathnode_closest_to_pos_in_corridor(const Vector3 _pos, Ref<AStar3D>& _astar_micro, const std::unordered_map<uint32_t, FlowAIPathnode*>& pathnode_map);
		PackedVector3Array get_current_sections_path() { return current_sectors_path; }
		PackedVector3Array get_current_pathnodes_path() { return current_pathnodes_path; }
	};
}

VARIANT_ENUM_CAST(FlowAI::FlowAIPathResult);