#include "../classes/FlowAIAgent3D.hpp"
#include "../classes/FlowAIManager.hpp"
#include "../classes/FlowAIPathnode.hpp"

#include <godot_cpp/classes/engine.hpp>

namespace FlowAI {
	FlowAIAgent3D::FlowAIAgent3D() {};

	FlowAIAgent3D::~FlowAIAgent3D() {}

	// PROTECTED
	void FlowAIAgent3D::_bind_methods() {
		BIND_ENUM_CONSTANT(PATH_FOUND);
		BIND_ENUM_CONSTANT(PATH_NOT_FOUND);
		BIND_ENUM_CONSTANT(PATH_LAYER_BLOCKED);

		ClassDB::bind_method(D_METHOD("is_path_complete"), &FlowAIAgent3D::is_path_complete);

		ClassDB::bind_method(D_METHOD("set_target_pathnode", "target_pathnode", "layers_mask", "strict_layers"), &FlowAIAgent3D::set_target_pathnode);
		ClassDB::bind_method(D_METHOD("set_random_path", "strict_layers"), &FlowAIAgent3D::set_random_path);
		ClassDB::bind_method(D_METHOD("set_path_desired_distance", "_value"), &FlowAIAgent3D::set_path_desired_distance);
		ClassDB::bind_method(D_METHOD("set_path_layers", "p_layers"), &FlowAIAgent3D::set_path_layers);

		ClassDB::bind_method(D_METHOD("get_next_pathnode_position"), &FlowAIAgent3D::get_next_pathnode_position);
		ClassDB::bind_method(D_METHOD("get_current_manager"), &FlowAIAgent3D::get_current_manager);
		ClassDB::bind_method(D_METHOD("get_pathnode_closest_to_pos"), &FlowAIAgent3D::get_pathnode_closest_to_pos);
		ClassDB::bind_method(D_METHOD("get_path_desired_distance"), &FlowAIAgent3D::get_path_desired_distance);
		ClassDB::bind_method(D_METHOD("get_path_layers"), &FlowAIAgent3D::get_path_layers);

		ClassDB::add_property(
			"FlowAIAgent3D", PropertyInfo(
				Variant::INT,
				"path_layers",
				PROPERTY_HINT_LAYERS_3D_NAVIGATION,
				"",
				PROPERTY_USAGE_DEFAULT
			),
			"set_path_layers", "get_path_layers"
		);

		ClassDB::add_property("FlowAIAgent3D", PropertyInfo(
			Variant::FLOAT, 
			"path_desired_distance", 
			PROPERTY_HINT_NONE, 
			"", 
			PROPERTY_USAGE_DEFAULT
		), 
			"set_path_desired_distance", "get_path_desired_distance"
		);
	}

	void FlowAIAgent3D::_notification(int p_what) {
		switch (p_what) {
		case NOTIFICATION_ENTER_TREE:
			path_preview = memnew(MeshInstance3D);
			immediate_mesh.instantiate();
			path_preview->set_mesh(immediate_mesh);
			add_child(path_preview);
			break;
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
				Vector3 true_actor_pos = actor_owner->get_global_position();
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

	FlowAIPathResult FlowAIAgent3D::set_target_pathnode(FlowAIPathnode* target_pathnode, uint32_t layers_mask, bool strict_layers) {
		path_complete = false;
		return request_path(target_pathnode->get_global_position(), layers_mask, strict_layers);
	}

	FlowAIPathResult FlowAIAgent3D::set_random_path(bool strict_layers) {
		std::vector<FlowAIPathnode*> pathnode_list = FlowAIManager::get_singleton()->get_pathnode_list_by_layer(get_path_layers());
		int random_pathnode_index = UtilityFunctions::randi_range(0, (int)pathnode_list.size() - 1);
		FlowAIPathnode* random_pathnode = pathnode_list[random_pathnode_index];
		path_complete = false;

		if (!random_pathnode) {
			UtilityFunctions::print("[FlowAI::set_random_path] Null Index Pathnode");
			return PATH_NOT_FOUND;
		}

		return request_path(random_pathnode->get_global_position(), get_path_layers(), strict_layers);
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

	FlowAIManager* FlowAIAgent3D::get_current_manager() const { return FlowAIManager::get_singleton(); }

	FlowAIPathnode* FlowAIAgent3D::get_pathnode_closest_to_pos(const Vector3 _pos) const {
		FlowAIPathnode* closest_pathnode = nullptr;
		double min_dist = INFINITY;
		for (const auto pathnode : FlowAIManager::get_singleton()->get_pathnode_list()) {
			double distance_to_node = _pos.distance_to(pathnode->get_global_position());
			if (distance_to_node < min_dist) {
				min_dist = distance_to_node;
				closest_pathnode = pathnode;
			}
		}
		return closest_pathnode;
	}

	/////////////////////////////////////////////////////////////////////////////
	// PRIVATE CALLS
	/////////////////////////////////////////////////////////////////////////////
	FlowAIPathResult FlowAIAgent3D::request_path(Vector3 _pos, uint32_t _layer_mask, bool strict_layers) {
		FlowAIPathResult path_result = PATH_NOT_FOUND;

		if (!actor_owner || !astar_macro) return path_result;

		current_sectors_in_corridor.clear();
		current_sectors_path.clear();

		// Get Section path corridor
		FlowAISector* start_sector = FlowAIManager::get_singleton()->get_closest_sector_by_pos_that_have_pathnode(actor_owner->get_global_position());
		FlowAISector* end_sector = FlowAIManager::get_singleton()->get_closest_sector_by_pos_that_have_pathnode(_pos);

		if (start_sector != nullptr && end_sector != nullptr) {
			if (!astar_macro->has_point(start_sector->get_id()) || !astar_macro->has_point(end_sector->get_id())) {
				UtilityFunctions::print("[FlowAI] Sector not in macro graph: ", start_sector->get_id(), " / ", end_sector->get_id());
				path_result = PATH_NOT_FOUND;
			}

			// Check if start_sector and end_sector dain't the same sector.
			if (is_debug_enabled(DEBUG_ENABLE_LOGGING)) {
				UtilityFunctions::print("[FlowAI] Start Sector: ", start_sector->get_id());
				UtilityFunctions::print("[FlowAI] End Sector: ", end_sector->get_id());
			}

			if (start_sector->get_id() != end_sector->get_id()) {
				current_sectors_path = astar_macro->get_point_path(start_sector->get_id(), end_sector->get_id());

				// we need to store the FlowAISector in a HashMap to get the micro_pathnodes list.
				for (int i = 0; i < current_sectors_path.size(); i++) {
					FlowAISector* sector_node_ref = FlowAIManager::get_singleton()->get_sector_by_pos(current_sectors_path[i]);
					if (sector_node_ref != nullptr) current_sectors_in_corridor[sector_node_ref->get_id()] = sector_node_ref;
				}
			}
			else {
				current_sectors_in_corridor[start_sector->get_id()] = start_sector;
			}
		}

		path_result = generate_pathnode_path(_pos, start_sector, end_sector, _layer_mask, strict_layers);
		draw_agent_pathnode_path(current_pathnodes_path);

		if (is_debug_enabled(DEBUG_ENABLE_LOGGING)) {
			UtilityFunctions::print("[FlowAI] current_sectors_path.size(): ", current_sectors_path.size());
			UtilityFunctions::print("[FlowAI] current_pathnodes_path.size(): ", current_pathnodes_path.size());
		}

		return path_result;
	}

	FlowAIPathResult FlowAIAgent3D::generate_pathnode_path(Vector3 _pos, FlowAISector* start_sector, FlowAISector* end_sector, uint32_t _layers, bool strict_layers) {
		bool success = try_build_micro_path(_pos, _layers);
		FlowAIPathResult path_result;

		if (!success) { // If it failure, expand corridor and try again
			PackedInt64Array neighbors = astar_macro->get_point_connections(start_sector->get_id()); // get connected sections neighbors
			for (int i = 0; i < neighbors.size(); i++) {
				FlowAISector* neighbor = FlowAIManager::get_singleton()->get_sector_by_id((uint32_t)neighbors[i]);
				if (neighbor) current_sectors_in_corridor[neighbor->get_id()] = neighbor;
			}

			success = try_build_micro_path(_pos, _layers);

			if (!success) { // if still failure, use complete sections macro
				current_sectors_in_corridor.clear();
				for (int i = 0; i < neighbors.size(); i++) {
					FlowAISector* s = FlowAIManager::get_singleton()->get_sector_by_id((uint32_t)neighbors[i]);
					if (s) current_sectors_in_corridor[s->get_id()] = s;
				}
				current_sectors_in_corridor[start_sector->get_id()] = start_sector;
				success = try_build_micro_path(_pos, _layers);
			}

			if (!success) {
				if (strict_layers) return PATH_LAYER_BLOCKED;

				bool path_exists_ignoring_layers = try_build_micro_path(_pos, _layers, true); // Test without layers
				if (path_exists_ignoring_layers) {
					return path_result = PATH_LAYER_BLOCKED;
				}
				else {
					return path_result = PATH_NOT_FOUND;
				}
			}
		}
		return (success) ? PATH_FOUND : path_result;
	}

	void FlowAIAgent3D::set_next_path_index() {
		current_path_index++;
		if (current_path_index >= current_pathnodes_path.size()) {
			path_complete = true;
			current_pathnodes_path.clear();
			current_path_index = 0;
			if (is_debug_enabled(DEBUG_ENABLE_LOGGING)) UtilityFunctions::print("[FlowAIAgent3D] Path completed.");
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

	bool FlowAIAgent3D::try_build_micro_path(Vector3 _pos, uint32_t _layers, bool ignore_layers) {
		Ref<AStar3D> astar_micro; // Local AStar
		PackedVector3Array path_data;

		astar_micro.instantiate();

		if (current_sectors_in_corridor.is_empty()) return false;


		// -------------- (Micro) --------------
		// Add points - section pathnodes
		for (auto& obj : current_sectors_in_corridor) {
			unsigned int _sector_id = obj.key;
			auto it = manager_sector_list.find(_sector_id);

			for (uint32_t pathnode_id : it->second.get_pathnodes()) {
				auto node_it = manager_pathnode_list.find(pathnode_id);
				if (node_it == manager_pathnode_list.end()) 
					continue;

				FlowAIPathnode* ref = node_it->second;
				bool layer_compatible = (ref->get_path_layers() & _layers) > 0;
				if (ref && (ignore_layers || layer_compatible)) {
					astar_micro->add_point(pathnode_id, ref->get_global_position(), ref->get_weight_scale());
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
				bool layer_compatible = (origin->get_path_layers() & _layers) > 0;

				if (origin && (ignore_layers || layer_compatible)) {
					for (auto link_id : origin->get_links()) {
						if (!astar_micro->has_point(link_id)) continue;
						if (!astar_micro->are_points_connected(origin_id, link_id, origin->is_bidirectional())) {
							astar_micro->connect_points(origin_id, link_id, origin->is_bidirectional());
						}
					}
				}
			}
		}

		FlowAIPathnode* start_pathnode = get_pathnode_closest_to_pos_in_corridor(actor_owner->get_global_position(), astar_micro, manager_pathnode_list);
		FlowAIPathnode* end_pathnode = get_pathnode_closest_to_pos_in_corridor(_pos, astar_micro, manager_pathnode_list);

		if (!astar_micro->has_point(start_pathnode->get_id()) || !astar_micro->has_point(end_pathnode->get_id())) {
			UtilityFunctions::print("[FlowAI] Start or end pathnode outside corridor — micro graph incomplete");
			return false;
		}

		if (start_pathnode->get_id() == end_pathnode->get_id()) {
			path_data.push_back(start_pathnode->get_global_position());
		}
		else {
			// If start_pathnode and end_pathnode ain't the same:
			PackedInt64Array micro_id_path = astar_micro->get_id_path(start_pathnode->get_id(), end_pathnode->get_id());

			if (micro_id_path.is_empty()) return false;

			for (int i = 0; i < micro_id_path.size(); i++) {
				uint32_t node_id = (uint32_t)micro_id_path[i];
				auto it = manager_pathnode_list.find(node_id);
				if (it == manager_pathnode_list.end() || !it->second) continue;
				path_data.push_back(it->second->get_global_position());
			}
		}

		current_path_index = 0;
		path_complete = false;
		current_pathnodes_path = path_data;
		return true;
	}
}