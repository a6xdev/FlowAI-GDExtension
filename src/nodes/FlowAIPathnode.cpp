#include "FlowAI.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/physics_ray_query_parameters3d.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/classes/physics_direct_space_state3d.hpp>
#include <godot_cpp/classes/box_mesh.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>
#include <godot_cpp/classes/project_settings.hpp>

using namespace godot;

namespace FlowAI {
	FlowAIPathnode::FlowAIPathnode() {};
	FlowAIPathnode::~FlowAIPathnode() {};

	void FlowAIPathnode::_bind_methods() {
		ClassDB::bind_method(D_METHOD("set_id"), &FlowAIPathnode::set_id);
		ClassDB::bind_method(D_METHOD("set_prev_node_id"), &FlowAIPathnode::set_prev_node_id);
		ClassDB::bind_method(D_METHOD("set_links"), &FlowAIPathnode::set_links);
		ClassDB::bind_method(D_METHOD("set_path_layers", "p_layers"), &FlowAIPathnode::set_path_layers);
		ClassDB::bind_method(D_METHOD("set_sector_id", "p_layers"), &FlowAIPathnode::set_sector_id);
		ClassDB::bind_method(D_METHOD("set_sector_coord", "p_layers"), &FlowAIPathnode::set_sector_coord);
		ClassDB::bind_method(D_METHOD("set_bidirectional", "p_layers"), &FlowAIPathnode::set_bidirectional);

		ClassDB::bind_method(D_METHOD("get_id"), &FlowAIPathnode::get_id);
		ClassDB::bind_method(D_METHOD("get_prev_node_id"), &FlowAIPathnode::get_prev_node_id);
		ClassDB::bind_method(D_METHOD("get_links"), &FlowAIPathnode::get_links);
		ClassDB::bind_method(D_METHOD("get_path_layers"), &FlowAIPathnode::get_path_layers);
		ClassDB::bind_method(D_METHOD("get_sector_id"), &FlowAIPathnode::get_sector_id);
		ClassDB::bind_method(D_METHOD("get_sector_coord"), &FlowAIPathnode::get_sector_coord);
		ClassDB::bind_method(D_METHOD("is_bidirectional"), &FlowAIPathnode::is_bidirectional);

		ClassDB::add_property("FlowAIPathnode", PropertyInfo(Variant::INT, "id", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE), "set_id", "get_id");
		ClassDB::add_property("FlowAIPathnode", PropertyInfo(Variant::INT, "prev_pathnode_id", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE), "set_prev_node_id", "get_prev_node_id");
		ClassDB::add_property("FlowAIPathnode", PropertyInfo(Variant::INT, "sector_id", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE), "set_sector_id", "get_sector_id");
		ClassDB::add_property("FlowAIPathnode", PropertyInfo(Variant::PACKED_INT32_ARRAY, "links", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE), "set_links", "get_links");

		ClassDB::bind_method(D_METHOD("add_next_pathnode"), &FlowAIPathnode::add_next_pathnode);
		ClassDB::bind_method(D_METHOD("snap_to_ground"), &FlowAIPathnode::snap_to_ground);

		String settings_prefix = "flow_ai/navigation_3d";

		ClassDB::add_property("FlowAIPathnode", PropertyInfo(Variant::BOOL, "bidirectional"), "set_bidirectional", "is_bidirectional");
		ClassDB::add_property(
			"FlowAIPathnode",
			PropertyInfo(
				Variant::INT,
				"path_layers",
				PROPERTY_HINT_LAYERS_3D_NAVIGATION,
				"",
				PROPERTY_USAGE_DEFAULT
			),
			"set_path_layers",
			"get_path_layers"
		);

	}

	// Create debug meshes
	void FlowAIPathnode::_notification(int p_what) {
		switch (p_what) {
			case NOTIFICATION_ENTER_TREE: {
				if (Engine::get_singleton()->is_editor_hint()) {
					set_process_mode(PROCESS_MODE_ALWAYS);
				}

				if (mesh_preview == nullptr) {
					mesh_preview = memnew(MeshInstance3D);

					Ref<BoxMesh> box_mesh;
					box_mesh.instantiate();
					box_mesh->set_size(Vector3(0.2f, 0.2f, 0.2f));

					Ref<StandardMaterial3D> material;
					material.instantiate();
					material->set_albedo(Color(0.0f, 0.6f, 1.0f, 0.7f)); // blue neon with 70% of opacity
					material->set_transparency(BaseMaterial3D::TRANSPARENCY_ALPHA);
					material->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);

					box_mesh->set_material(material);
					mesh_preview->set_mesh(box_mesh);

					add_child(mesh_preview);
				}

				if (line_preview == nullptr) {
					line_preview = memnew(MeshInstance3D);
					imm_mesh.instantiate();
					line_preview->set_mesh(imm_mesh);

					Ref<StandardMaterial3D> line_material;
					line_material.instantiate();
					line_material->set_shading_mode(BaseMaterial3D::SHADING_MODE_UNSHADED);
					line_preview->set_material_override(line_material);

					add_child(line_preview);
				}
			} break;

			case NOTIFICATION_READY: {
				_update_mesh_preview_colors();
			} break;
		}
	}

	// Calls
	void FlowAIPathnode::add_next_pathnode() {
		auto* parent = get_parent();
		FlowAIManager* manager = Object::cast_to<FlowAIManager>(parent);

		if (manager) {
			manager->add_new_pathnode(id);
		}
	}

	void FlowAIPathnode::snap_to_ground() {
		Ref<World3D> world = get_world_3d();
		if (world.is_null()) return;
		PhysicsDirectSpaceState3D* space_state = world->get_direct_space_state();
		if (!space_state) return;
		Vector3 start_pos = get_global_position();
		Vector3 end_pos = start_pos + Vector3(0, -100.0f, 0);
		Ref<PhysicsRayQueryParameters3D> query = PhysicsRayQueryParameters3D::create(start_pos, end_pos);
		Dictionary result = space_state->intersect_ray(query);
		if (!result.is_empty()) {
			Vector3 hit_point = result["position"];
			set_global_position(hit_point);
		}
	}

	void FlowAIPathnode::set_path_layers(uint32_t p_layers) {
		path_layers = p_layers;
		_update_mesh_preview_colors();
	}


	void FlowAIPathnode::_update_mesh_preview_colors() {
		if (mesh_preview == nullptr) return;

		Ref<Mesh> mesh = mesh_preview->get_mesh();
		if (mesh.is_null()) return;

		Ref<StandardMaterial3D> mat = mesh->surface_get_material(0);
		if (mat.is_null()) return;

		ProjectSettings* settings = ProjectSettings::get_singleton();
		if (!settings) return;

		// find the first active layer in our mask.
		int active_layer = 1;
		for (int i = 0; i < 32; ++i) {
			if ((path_layers & (1 << i)) != 0) {
				active_layer = i + 1;
				break;
			}
		}

		// Find the color of this layer in ProjectSettings
		String color_path = "flow_ai/layer_colors/layer_" + String::num_int64(active_layer);
		if (settings->has_setting(color_path)) {
			Color layer_color = settings->get_setting(color_path);
			mat->set_albedo(layer_color);
		}
	}

	void FlowAIPathnode::_redraw_connections() {
		if (imm_mesh.is_null() || mesh_preview == nullptr) return;

		imm_mesh->clear_surfaces();

		if (links.is_empty()) return;

		Ref<Mesh> mesh = mesh_preview->get_mesh(); if (mesh.is_null()) return;
		Ref<StandardMaterial3D> mat = mesh->surface_get_material(0); if (mat.is_null()) return;
		Color line_color = mat->get_albedo();
		line_color.a = 1.0f;

		Vector3 local_start(0, 0, 0);
		imm_mesh->surface_begin(Mesh::PRIMITIVE_LINES);

		for (int i = 0; i < links.size(); ++i) {
			int32_t target_id = links[i];
			Node* parent = get_parent();
			if (!parent) continue;

			FlowAIPathnode* target_node = nullptr;
			for (int j = 0; j < parent->get_child_count(); ++j) {
				if (FlowAIPathnode* sibling = Object::cast_to<FlowAIPathnode>(parent->get_child(j))) {
				}
			}

			if (target_node) {
				Vector3 local_end = to_local(target_node->get_global_position());

				imm_mesh->surface_set_color(line_color);
				imm_mesh->surface_add_vertex(local_start);

				imm_mesh->surface_set_color(line_color);
				imm_mesh->surface_add_vertex(local_end);
			}
		}

		imm_mesh->surface_end();
	}
}