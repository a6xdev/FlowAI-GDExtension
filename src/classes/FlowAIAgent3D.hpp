#include "../FlowAI.hpp"

namespace FlowAI {
	class FlowAIPathnode;
	class FlowAIManager;

	// ----------------------------------------- //
	// Mr. Bond (our Agent lol)
	// ----------------------------------------- //
	class FlowAIAgent3D : public Node {
		GDCLASS(FlowAIAgent3D, Node)
	public:
		float path_desired_distance = 1.0f;

		FlowAIAgent3D();
		~FlowAIAgent3D();

		bool is_path_complete() const { return path_complete; }

		void set_target_pathnode(FlowAIPathnode* target_pathnode);
		void set_random_path();

		Vector3 get_next_pathnode_position();
		FlowAIManager* get_current_manager() const;
		FlowAIPathnode* get_pathnode_closest_to_pos(const Vector3 _pos) const;
	protected:
		static void _bind_methods();
		void _notification(int p_what);
	private:
		CharacterBody3D* actor_owner = nullptr;
		AStar3D* astar_macro = nullptr;
		HashMap<unsigned int, FlowAISector*> current_sectors_in_corridor;
		PackedVector3Array current_sectors_path;
		PackedVector3Array current_pathnodes_path;
		std::unordered_map<unsigned int, FlowAISector> manager_sector_list;
		std::unordered_map<uint32_t, FlowAIPathnode*> manager_pathnode_list;
		int current_path_index = 0;
		bool path_complete = false;

		MeshInstance3D* path_preview = nullptr;
		Ref<ImmediateMesh> immediate_mesh;

		void request_path(Vector3 _pos_target);
		void generate_section_path(Vector3 _pos);
		PackedVector3Array generate_pathnode_path(
			Vector3 _pos,
			Ref<AStar3D>& astar_micro,
			const std::unordered_map<uint32_t, FlowAIPathnode*>& manager_pathnode_list
		);
		void draw_agent_pathnode_path(PackedVector3Array p_path); // Debug

		void set_next_path_index();

		FlowAIPathnode* get_pathnode_closest_to_pos_in_corridor(
			const Vector3 _pos, 
			Ref<AStar3D>& _astar_micro,
			const std::unordered_map<uint32_t, FlowAIPathnode*>& pathnode_map
		);
		PackedVector3Array get_current_sections_path() { return current_sectors_path; }
		PackedVector3Array get_current_pathnodes_path() { return current_pathnodes_path; }
	};
}