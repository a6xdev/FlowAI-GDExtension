#include "../FlowAI.hpp"
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
			path_renderer = memnew(MeshInstance3D);
			immediate_mesh.instantiate();
			path_renderer->set_mesh(immediate_mesh);
			add_child(path_renderer);
			break;
		case NOTIFICATION_READY:
			set_physics_process(true);
			if (!Engine::get_singleton()->is_editor_hint()) {
				actor_owner = Object::cast_to<CharacterBody3D>(get_parent());
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
		int random_pathnode_index = UtilityFunctions::randi_range(0, pathnode_list.size());
		FlowAIPathnode* random_pathnode = pathnode_list[random_pathnode_index];

		UtilityFunctions::print("[set_random_path] Random Pathnode Defined: ", random_pathnode_index);

		request_path(random_pathnode->get_global_position());
	}

	Vector3 FlowAIAgent3D::get_next_pathnode_position() {
		if (path_complete || current_pathnodes_path.is_empty()) {
			return actor_owner->get_global_position();
		}
		else {
			return current_pathnodes_path[current_path_index];
		}
		return Vector3(0.0, 0.0, 0.0);
	}

	/////////////////////////////////////////////////////////////////////////////
	// PRIVATE CALLS
	/////////////////////////////////////////////////////////////////////////////
	void FlowAIAgent3D::request_path(Vector3 _pos) {
		if (!actor_owner) return;

		AStar3D* astar_macro = FlowAIManager::get_singleton()->get_macro_astar();
		Ref<AStar3D> astar_micro; // Local AStar

		astar_micro.instantiate();

		// -------------- (Macro) --------------
		// Get Section path corridor
		FlowAISector start_sector = FlowAIManager::get_singleton()->get_sector_by_pos(actor_owner->get_global_position());
		FlowAISector end_sector = FlowAIManager::get_singleton()->get_sector_by_pos(_pos);

		UtilityFunctions::print("Pos Target: ", _pos);
		UtilityFunctions::print("Start Sector: ", start_sector.get_id());
		UtilityFunctions::print("End Sector: ", end_sector.get_id());


		if (astar_macro->has_point(start_sector.get_id()) && astar_macro->has_point(end_sector.get_id())) {

			if (start_sector.get_id() == end_sector.get_id()) {
				UtilityFunctions::print("Test");
				return;
			}

			// Check if exists path before call get_point_path
			PackedInt64Array id_path = astar_macro->get_id_path(start_sector.get_id(), end_sector.get_id());
			if (id_path.is_empty()) {
				UtilityFunctions::print("Macro: sem caminho entre ", start_sector.get_id(), " e ", end_sector.get_id());
				return;
			}

			current_true_sectors_path = astar_macro->get_point_path(start_sector.get_id(), end_sector.get_id());

			// we need to store the FlowAISector in a HashMap to get the micro_pathnodes list.
			for (int i = 0; i < current_true_sectors_path.size(); i++) {
				FlowAISector sector_node_ref = FlowAIManager::get_singleton()->get_sector_by_pos(current_true_sectors_path[i]);
				current_sectors_database[sector_node_ref.get_id()] = current_true_sectors_path;
			}
		}


		// -------------- (Micro) --------------
		// get the nearest pathnode.
		std::unordered_map<uint32_t, FlowAIPathnode*> manager_pathnode_list;
		std::unordered_map<unsigned int, FlowAISector> manager_sector_list = FlowAIManager::get_singleton()->get_sectors_list();

		// Get pathnode_list (vector) and organize in the manager_pathnode_list.
		for (auto pathnode_ptr : FlowAIManager::get_singleton()->get_pathnode_list()) {
			manager_pathnode_list[pathnode_ptr->get_id()] = pathnode_ptr;
		}

		// Add points - section pathnodes
		for (auto &obj : current_sectors_database) {
			unsigned int _sector_id = obj.key;
			auto it = manager_sector_list.find(_sector_id);
			
			if (it != manager_sector_list.end()) {
				FlowAISector _sector = it->second;

				for (uint32_t pathnode_id : _sector.get_pathnodes()) {
					auto node_it = manager_pathnode_list.find(pathnode_id);
					if (node_it == manager_pathnode_list.end()) continue;
					FlowAIPathnode* pathnode_ref = node_it->second;

					if (pathnode_ref != nullptr) {
						UtilityFunctions::print("add_point - pathnode_id: ", pathnode_id);
						astar_micro->add_point(pathnode_id, pathnode_ref->get_global_position()); // TODO: WEIGHT_SCALE soon...
					}
				}
			}
		}

		// Connect points
		for (auto& obj : current_sectors_database) {
			auto it = manager_sector_list.find(obj.key);
			if (it == manager_sector_list.end()) continue;

			FlowAISector& _sector = it->second;

			for (uint32_t origin_id : _sector.get_pathnodes()) {
				auto origin_it = manager_pathnode_list.find(origin_id);
				if (origin_it == manager_pathnode_list.end()) continue;

				FlowAIPathnode* origin_pathnode = origin_it->second;

				for (auto link_id : origin_pathnode->get_links()) {
					if (!astar_micro->has_point(link_id)) continue;
					if (!astar_micro->are_points_connected(origin_id, link_id, origin_pathnode->is_bidirectional())) {
						astar_micro->connect_points(origin_id, link_id, origin_pathnode->is_bidirectional());
					}
				}
			}
		}


		// Get Pathnode path
		FlowAIPathnode* start_pathnode = get_pathnode_closest_to_pos(actor_owner->get_global_position());
		FlowAIPathnode* end_pathnode = get_pathnode_closest_to_pos(_pos);

		UtilityFunctions::print("start_pathnode: ", start_pathnode->get_name());
		UtilityFunctions::print("end_pathnode: ", end_pathnode->get_name());

		current_pathnodes_path = astar_micro->get_point_path(start_pathnode->get_id(), end_pathnode->get_id());
		draw_path(current_pathnodes_path);
	
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

	FlowAIPathnode* FlowAIAgent3D::get_pathnode_closest_to_pos(Vector3 _pos) {
		FlowAIPathnode* closest_pathnode = nullptr;
		double min_dist = INFINITY;
		for (const auto pathnode : FlowAIManager::get_singleton()->get_pathnode_list()) {
			double distance_to_node = _pos.distance_to(pathnode->get_global_position());
			if (distance_to_node < min_dist) {
				min_dist = distance_to_node;
				closest_pathnode = pathnode;
			}
		}
		UtilityFunctions::print("[get_pathnode_closest_to_pos] pathnode: ", closest_pathnode->get_name());
		return closest_pathnode;
	}

	PackedVector3Array FlowAIAgent3D::get_current_sections_path() { return current_true_sectors_path; }
	PackedVector3Array FlowAIAgent3D::get_current_pathnodes_path() { return current_pathnodes_path; }
}