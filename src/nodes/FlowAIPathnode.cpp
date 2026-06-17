#include "FlowAI.hpp"
#include <godot_cpp/classes/physics_ray_query_parameters3d.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/classes/physics_direct_space_state3d.hpp>
#include <godot_cpp/classes/box_mesh.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>

using namespace godot;

namespace FlowAI {
	FlowAIPathnode::FlowAIPathnode() {};
	FlowAIPathnode::~FlowAIPathnode() {};

	void FlowAIPathnode::_bind_methods() {
		ClassDB::bind_method(D_METHOD("set_id"), &FlowAIPathnode::set_id);
		ClassDB::bind_method(D_METHOD("set_prev_node_id"), &FlowAIPathnode::set_prev_node_id);
		ClassDB::bind_method(D_METHOD("set_sector_id"), &FlowAIPathnode::set_sector_id);
		ClassDB::bind_method(D_METHOD("set_links"), &FlowAIPathnode::set_links);

		ClassDB::bind_method(D_METHOD("get_id"), &FlowAIPathnode::get_id);
		ClassDB::bind_method(D_METHOD("get_prev_node_id"), &FlowAIPathnode::get_prev_node_id);
		ClassDB::bind_method(D_METHOD("get_sector_id"), &FlowAIPathnode::get_sector_id);
		ClassDB::bind_method(D_METHOD("get_links"), &FlowAIPathnode::get_links);

		ClassDB::add_property("FlowAIPathnode", PropertyInfo(Variant::INT, "id", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE), "set_id", "get_id");
		ClassDB::add_property("FlowAIPathnode", PropertyInfo(Variant::INT, "prev_pathnode_id", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE), "set_prev_node_id", "get_prev_node_id");
		ClassDB::add_property("FlowAIPathnode", PropertyInfo(Variant::INT, "sector_id", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE), "set_sector_id", "get_sector_id");
		ClassDB::add_property("FlowAIPathnode", PropertyInfo(Variant::PACKED_INT32_ARRAY, "links", PROPERTY_HINT_NONE, "", PROPERTY_USAGE_STORAGE), "set_links", "get_links");

		ClassDB::bind_method(D_METHOD("add_next_pathnode"), &FlowAIPathnode::add_next_pathnode);
		ClassDB::bind_method(D_METHOD("snap_to_ground"), &FlowAIPathnode::snap_to_ground);
	}

	void FlowAIPathnode::_enter_tree() {
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
	}

	void FlowAIPathnode::add_next_pathnode() {
		auto* parent = get_parent();
		FlowAIManager* manager = Object::cast_to<FlowAIManager>(parent);

		if (manager) {
			manager->add_new_pathnode(id);
		}
	}
	void FlowAIPathnode::snap_to_ground() {
		UtilityFunctions::print("Snap to grounndddd");
	}
}