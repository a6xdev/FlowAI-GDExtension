#include "../FlowAI.hpp"

namespace FlowAI {
	FlowAIAgent3D::FlowAIAgent3D() {};
	FlowAIAgent3D::~FlowAIAgent3D() {}

	void FlowAIAgent3D::set_target_pathnode(FlowAIPathnode* pathnode) {}

	void FlowAIAgent3D::get_random_path() {}

	Vector3 FlowAIAgent3D::get_next_pathnode_position() {}

	// PROTECTED
	void FlowAIAgent3D::_bind_methods() {}
	void FlowAIAgent3D::_notification(int p_what) {}

	// PRIVATE
	PackedVector3Array FlowAIAgent3D::find_path(FlowAIPathnode* start, FlowAIPathnode* target) {}
	Array FlowAIAgent3D::get_current_sections_path() {}
	Array FlowAIAgent3D::get_current_pathnodes_path() {}
}