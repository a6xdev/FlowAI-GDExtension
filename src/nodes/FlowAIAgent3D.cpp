#include "../FlowAI.hpp"

namespace FlowAI {
	FlowAIAgent3D::FlowAIAgent3D() {
		actor_owner = Object::cast_to<CharacterBody3D>(get_parent());
	};

	FlowAIAgent3D::~FlowAIAgent3D() {}

	// PROTECTED
	void FlowAIAgent3D::_bind_methods() {}

	/////////////////////////////////////////////////////////////////////////////
	// PUBLIC CALLS
	/////////////////////////////////////////////////////////////////////////////

	void FlowAIAgent3D::set_target_pathnode(FlowAIPathnode* pathnode) {}

	void FlowAIAgent3D::get_random_path() {}

	Vector3 FlowAIAgent3D::get_next_pathnode_position() {
		return Vector3(0.0, 0.0, 0.0);
	}

	/////////////////////////////////////////////////////////////////////////////
	// PRIVATE CALLS
	/////////////////////////////////////////////////////////////////////////////
	void FlowAIAgent3D::request_path(Vector3 _pos) const {
		if (!actor_owner) return;

		AStar3D* astar_macro = FlowAIManager::get_singleton()->get_macro_astar();
		FlowAIPathnode* start_pathnode = nullptr;
		PackedVector3Array sectors_path;

		// Get Section path corridor
		FlowAISector start_sector = FlowAIManager::get_singleton()->get_sector_by_pos(actor_owner->get_global_position());
		FlowAISector end_sector = FlowAIManager::get_singleton()->get_sector_by_pos(_pos);

		if (astar_macro->has_point(start_sector.get_id()) && astar_macro->has_point(end_sector.get_id())) {
			sectors_path = astar_macro->get_point_path(start_sector.get_id(), end_sector.get_id());
		}

		UtilityFunctions::print("sectors_path: ", sectors_path);

		// get the nearest pathnode.
		//double min_dist = 0.0;
		//for (const auto pathnode : FlowAIManager::get_singleton()->get_pathnode_list()) {
		//	double distance_to_node = actor_owner->get_global_position().distance_to(pathnode->get_global_position());
		//	if (distance_to_node < min_dist) {
		//		min_dist = distance_to_node;
		//		start_pathnode = pathnode;
		//	}
		//}
	}

	PackedVector3Array FlowAIAgent3D::get_current_sections_path() { return current_sectors_path; }
	PackedVector3Array FlowAIAgent3D::get_current_pathnodes_path() { return current_pathnodes_path; }
}