#include "../classes/FlowAIAgent3D.hpp"
#include "../classes/FlowAIManager.hpp"
#include "../classes/FlowAIPathnode.hpp"
#include "../FlowAIDebug.hpp"
#include <godot_cpp/classes/engine.hpp>

namespace FlowAI {
	FlowAIAgent3D::FlowAIAgent3D() {};

	FlowAIAgent3D::~FlowAIAgent3D() {}

	// PROTECTED
	void FlowAIAgent3D::_bind_methods() {
		ClassDB::bind_method(D_METHOD("is_path_complete"), &FlowAIAgent3D::is_path_complete);

		ClassDB::bind_method(D_METHOD("set_target_pathnode"), &FlowAIAgent3D::set_target_pathnode);
		ClassDB::bind_method(D_METHOD("set_random_path"), &FlowAIAgent3D::set_random_path);
		ClassDB::bind_method(D_METHOD("get_next_pathnode_position"), &FlowAIAgent3D::get_next_pathnode_position);
	}

	void FlowAIAgent3D::_notification(int p_what) {
		switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
			if (FlowAIDebug::draw_agent_path) {
				path_preview = memnew(MeshInstance3D);
				immediate_mesh.instantiate();
				path_preview->set_mesh(immediate_mesh);
				add_child(path_preview);
				break;
			}
		case NOTIFICATION_READY:
			set_physics_process(true);
			if (!Engine::get_singleton()->is_editor_hint()) {
				actor_owner = Object::cast_to<CharacterBody3D>(get_parent());
				astar_macro = FlowAIManager::get_singleton()->get_macro_astar();
				manager_sector_list = FlowAIManager::get_singleton()->get_sectors_list();
				// Get pathnode_list (vector) and organize in the manager_pathnode_list.
				for (auto pathnode_ptr : FlowAIManager::get_singleton()->get_pathnode_list()) {
					manager_pathnode_list[pathnode_ptr->get_id()] = pathnode_ptr;
				}
			}
			break;
		case NOTIFICATION_PHYSICS_PROCESS:
			if (!current_pathnodes_path.is_empty() && actor_owner) {
				float dist = actor_owner->get_global_position().distance_to(current_pathnodes_path[current_path_index]);
				if (dist < path_desired_distance) {
					set_next_path_index();
				}
			}
			break;
		}
	}

	/////////////////////////////////////////////////////////////////////////////
	// PUBLIC CALLS
	/////////////////////////////////////////////////////////////////////////////

	void FlowAIAgent3D::set_target_pathnode(FlowAIPathnode* pathnode) {}

	void FlowAIAgent3D::set_random_path() {
		path_complete = false;
		std::vector<FlowAIPathnode*> pathnode_list = FlowAIManager::get_singleton()->get_pathnode_list();

		int random_pathnode_index = UtilityFunctions::randi_range(0, (int)pathnode_list.size() - 1);
		FlowAIPathnode* random_pathnode = pathnode_list[random_pathnode_index];

		if (!random_pathnode) {
			//UtilityFunctions::print("[FlowAI::set_random_path] Null Index Pathnode");
			return;
		}

		request_path(random_pathnode->get_global_position());
	}

	Vector3 FlowAIAgent3D::get_next_pathnode_position() {
		bool is_actor_valid = actor_owner && godot::ObjectDB::get_instance(actor_owner->get_instance_id()) && actor_owner->is_inside_tree();

		if (path_complete || current_pathnodes_path.is_empty()) {
			if (is_actor_valid) return actor_owner->get_global_position();
			return Vector3(0.0, 0.0, 0.0);
		}
		else {
			if (current_path_index >= 0 && current_path_index < current_pathnodes_path.size()) {
				return current_pathnodes_path[current_path_index];
			}

			if (is_actor_valid) return actor_owner->get_global_position();
		}
		return Vector3(0.0, 0.0, 0.0);
	}

	/////////////////////////////////////////////////////////////////////////////
	// PRIVATE CALLS
	/////////////////////////////////////////////////////////////////////////////
	void FlowAIAgent3D::request_path(Vector3 _pos) {
		if (!actor_owner || !astar_macro) return;

		// 
		current_sectors_in_corridor.clear();
		current_sectors_path.clear();

		Ref<AStar3D> astar_micro; // Local AStar
		astar_micro.instantiate();

		current_sectors_path = generate_section_path(_pos);

		if (current_sectors_path.size() == 0) return;

		// TODO: CREATE A FUNCTION TO ADD POINTS
		// -------------- (Micro) --------------
		// Add points - section pathnodes
		for (auto &obj : current_sectors_in_corridor) {
			unsigned int _sector_id = obj.key;
			auto it = manager_sector_list.find(_sector_id);

			for (uint32_t pathnode_id : it->second.get_pathnodes()) {
				auto node_it = manager_pathnode_list.find(pathnode_id);
				if (node_it == manager_pathnode_list.end()) continue;
				FlowAIPathnode* ref = node_it->second;
				if (ref) {
					astar_micro->add_point(pathnode_id, ref->get_global_position()); // TODO: WEIGHT_SCALE soon...
				}
			}
		}

		// Connect points
		for (auto& obj : current_sectors_in_corridor) {
			auto it = manager_sector_list.find(obj.key);
			if (it == manager_sector_list.end()) continue;

			for (uint32_t origin_id : it->second.get_pathnodes()) {
				auto origin_it = manager_pathnode_list.find(origin_id);
				if (origin_it == manager_pathnode_list.end()) continue;
				FlowAIPathnode* origin = origin_it->second;
				if (!origin) continue;

				for (auto link_id : origin->get_links()) {
					if (!astar_micro->has_point(link_id)) continue;
					if (!astar_micro->are_points_connected(origin_id, link_id, origin->is_bidirectional())) {
						astar_micro->connect_points(origin_id, link_id, origin->is_bidirectional());
					}
				}
			}
		}

		// Swap to dont crash the game!
		current_path_index = 0;
		path_complete = false;
		current_pathnodes_path = generate_pathnode_path(_pos, astar_micro, manager_pathnode_list);
		draw_agent_pathnode_path(current_pathnodes_path);

		UtilityFunctions::print("current_sectors_path.size(): ", current_sectors_path.size());
		UtilityFunctions::print("current_pathnodes_path.size(): ", current_pathnodes_path.size());

		return;
	}

	void FlowAIAgent3D::set_next_path_index() {
		current_path_index++;
		if (current_path_index >= current_pathnodes_path.size()) {
			path_complete = true;
			current_pathnodes_path.clear();
			current_path_index = 0;
			UtilityFunctions::print("[FlowAIAgent3D] Path completed.");
		}
	}

	FlowAIPathnode* FlowAIAgent3D::get_pathnode_closest_to_pos_in_corridor(const Vector3 _pos, Ref<AStar3D>& _astar_micro, const std::unordered_map<uint32_t, FlowAIPathnode*>& pathnode_map) {
		FlowAIPathnode* closest_pathnode = nullptr;
		double min_dist = INFINITY;

		PackedInt64Array points = _astar_micro->get_point_ids();
		for (int i = 0; i < points.size(); i++) {
			uint32_t id = (uint32_t)points[i];
			auto it = pathnode_map.find(id);
			if (it == pathnode_map.end() || !it->second) continue;

			float dist = it->second->get_global_position().distance_squared_to(_pos);
			if (dist < min_dist) {
				min_dist = dist;
				closest_pathnode = it->second;
			}
		}
		return closest_pathnode;
	}

	PackedVector3Array FlowAIAgent3D::generate_section_path(Vector3 _pos) {
		// -------------- (Macro) --------------
		PackedVector3Array path_data;

		// Get Section path corridor
		FlowAISector* start_sector = FlowAIManager::get_singleton()->get_sector_by_pos(actor_owner->get_global_position());
		FlowAISector* end_sector = FlowAIManager::get_singleton()->get_sector_by_pos(_pos);

		if (start_sector != nullptr && end_sector != nullptr) {
			if (!astar_macro->has_point(start_sector->get_id()) || !astar_macro->has_point(end_sector->get_id())) {
				UtilityFunctions::print("[FlowAI] Sector not in macro graph: ", start_sector->get_id(), " / ", end_sector->get_id());
				return PackedVector3Array();
			}

			// Check if is the same sector.
			if (start_sector->get_id() != end_sector->get_id()) {
				//UtilityFunctions::print("Pos Target: ", _pos);
				UtilityFunctions::print("Start Sector: ", start_sector->get_id());
				UtilityFunctions::print("End Sector: ", end_sector->get_id());

				path_data = astar_macro->get_point_path(start_sector->get_id(), end_sector->get_id());

				// we need to store the FlowAISector in a HashMap to get the micro_pathnodes list.
				for (int i = 0; i < path_data.size(); i++) {
					FlowAISector* sector_node_ref = FlowAIManager::get_singleton()->get_sector_by_pos(path_data[i]);
					if (sector_node_ref != nullptr) current_sectors_in_corridor[sector_node_ref->get_id()] = sector_node_ref;
					else UtilityFunctions::print("Return sector node ref nullptr");
				}
			}
		}
		return path_data;
	}

	PackedVector3Array FlowAIAgent3D::generate_pathnode_path(Vector3 _pos, Ref<AStar3D>& astar_micro, const std::unordered_map<uint32_t, FlowAIPathnode*>& manager_pathnode_list) {
		PackedVector3Array path_data;
		FlowAIPathnode* start_pathnode = get_pathnode_closest_to_pos_in_corridor(actor_owner->get_global_position(), astar_micro, manager_pathnode_list);
		FlowAIPathnode* end_pathnode = get_pathnode_closest_to_pos_in_corridor(_pos, astar_micro, manager_pathnode_list);

		if (start_pathnode == nullptr || end_pathnode == nullptr) {
			UtilityFunctions::print("Start Pathnode or EndPathnode is nullptr");
			return PackedVector3Array();
		}

		if (ObjectDB::get_instance(start_pathnode->get_instance_id()) && ObjectDB::get_instance(end_pathnode->get_instance_id())) {
			if (!astar_micro->has_point(start_pathnode->get_id()) || !astar_micro->has_point(end_pathnode->get_id())) {
				UtilityFunctions::print("[FlowAI] Start or end pathnode outside corridor — micro graph incomplete");
				return PackedVector3Array();
			}

			if (start_pathnode->get_id() == end_pathnode->get_id()) {
				UtilityFunctions::print("[FlowAI] Start e end são o mesmo pathnode");
				return PackedVector3Array();
			}

			PackedInt64Array micro_id_path = astar_micro->get_id_path(start_pathnode->get_id(), end_pathnode->get_id());
			for (int i = 0; i < micro_id_path.size(); i++) {
				uint32_t node_id = (uint32_t)micro_id_path[i];
				auto it = manager_pathnode_list.find(node_id);
				if (it == manager_pathnode_list.end() || !it->second) {
					path_data.clear();
					return PackedVector3Array();
				}
				path_data.push_back(it->second->get_global_position());
			}

			//current_pathnodes_path = astar_micro->get_point_path(start_pathnode->get_id(), end_pathnode->get_id());
		}		
		return path_data;
	}
}