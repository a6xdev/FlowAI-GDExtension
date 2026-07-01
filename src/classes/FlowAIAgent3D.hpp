#include "../FlowAI.hpp"

namespace FlowAI {
	class FlowAIPathnode;	

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

		void set_target_pathnode(FlowAIPathnode* pathnode);
		void set_random_path();
		Vector3 get_next_pathnode_position();
	protected:
		static void _bind_methods();
		void _notification(int p_what);
	private:
		CharacterBody3D* actor_owner = nullptr;

		MeshInstance3D* path_preview = nullptr;
		Ref<ImmediateMesh> immediate_mesh;

		HashMap<unsigned int, FlowAISector*> current_sectors_database;
		PackedVector3Array current_true_sectors_path;
		PackedVector3Array current_pathnodes_path;
		int current_path_index = 0;
		bool path_complete = false;

		void request_path(Vector3 _pos_target);
		void draw_agent_pathnode_path(PackedVector3Array p_path);

		void set_next_path_index();
		FlowAIPathnode* get_pathnode_closest_to_pos(Vector3 _pos);

		PackedVector3Array get_current_sections_path();
		PackedVector3Array get_current_pathnodes_path();
	};
}