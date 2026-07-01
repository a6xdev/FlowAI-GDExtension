#include "FlowAIEditor.hpp"
#include <godot_cpp/classes/v_box_container.hpp>

using namespace godot;

namespace FlowAI {
    // ----------------------- //
    //      FlowAIPathnode
    // ----------------------- //
    void FlowAIEditorInspector::_bind_methods() {
        ClassDB::bind_method(D_METHOD("signal_manager_bake"), &FlowAIEditorInspector::signal_manager_bake);
        ClassDB::bind_method(D_METHOD("signal_manager_add_new_pathnode"), &FlowAIEditorInspector::signal_manager_add_new_pathnode);
        ClassDB::bind_method(D_METHOD("signal_pathnode_add_next_pathnode"), &FlowAIEditorInspector::signal_pathnode_add_next_pathnode);
        ClassDB::bind_method(D_METHOD("signal_pathnode_snap_to_ground"), &FlowAIEditorInspector::signal_pathnode_snap_to_ground);
    }

    bool FlowAIEditorInspector::_can_handle(Object* p_object) const {
        if (p_object == nullptr) return false;
        bool is_manager = Object::cast_to<FlowAIManager>(p_object) != nullptr;
        bool is_pathnode = Object::cast_to<FlowAIPathnode>(p_object) != nullptr;
        return is_manager || is_pathnode;
    }

    void FlowAIEditorInspector::_parse_begin(Object* p_object) {
        if (p_object == nullptr) return;

        target_node = p_object;
        if (FlowAIManager* manager = Object::cast_to<FlowAIManager>(p_object)) {
            _parse_manager(manager);
            return;
        }

        if (FlowAIPathnode* pathnode = Object::cast_to<FlowAIPathnode>(p_object)) {
            _parse_pathnode(pathnode);
            return;
        }
    }

    void FlowAIEditorInspector::_parse_manager(Object* target_node) {
        FlowAIManager* target_manager = Object::cast_to<FlowAIManager>(target_node);
        if (!target_manager) return;

        Button* btn_bake = memnew(Button);
        Button* btn_add_new_pathnode = memnew(Button);

        btn_bake->set_text("Bake");
        btn_add_new_pathnode->set_text("Add New Pathnode");

        btn_bake->connect("pressed", Callable(this, "signal_manager_bake"));
        btn_add_new_pathnode->connect("pressed", Callable(this, "signal_manager_add_new_pathnode"));

        add_custom_control(btn_bake);
        add_custom_control(btn_add_new_pathnode);
    }

    void FlowAIEditorInspector::_parse_pathnode(Object* target_node) {
        FlowAIPathnode* target_pathnode = Object::cast_to<FlowAIPathnode>(target_node);
        if (!target_pathnode) return;

        // Get the pathnode manager
        FlowAIManager* manager = Object::cast_to<FlowAIManager>(target_pathnode->get_parent());

        String prev_node_id_text = (target_pathnode->get_prev_node_id() == -1) ? "Nill" : String::num_int64(target_pathnode->get_prev_node_id());
        PackedInt32Array pathnode_links_id = target_pathnode->get_links();
        std::vector<FlowAIPathnode*> pathnodes_list = manager->get_pathnode_list();

        Label* lbl_id = memnew(Label);
        Label* lbl_prev_node_id = memnew(Label);
        Label* lbl_section_id = memnew(Label);
        Label* lbl_section_coord = memnew(Label);
        Label* lbl_links_title = memnew(Label);
        VBoxContainer* lbl_links_vbox_container = memnew(VBoxContainer);
        Button* btn_add = memnew(Button);
        Button* btn_snap = memnew(Button);

        lbl_id->set_text("Pathnode ID: " + String::num_int64(target_pathnode->get_id()));
        lbl_prev_node_id->set_text("Previous Pathnode ID: " + prev_node_id_text);
        lbl_section_id->set_text("Section ID: " + target_pathnode->get_sector_id());
        lbl_section_coord->set_text("Section Coord: " + target_pathnode->get_sector_coord());
        lbl_links_title->set_text("Links: [Array] - " + String::num_int64(target_pathnode->get_links().size()));
        btn_add->set_text("Add Next Pathnode");
        btn_snap->set_text("Snap to Ground");

        // Show all nodes kinked to the selected pathnode
        for (auto link_id : target_pathnode->get_links()) {
            FlowAIPathnode* link_pathnode_ref = pathnodes_list[link_id - 1]; // An array always start in index 0. HA!
            if (link_pathnode_ref) {
                Label* lbl_link = memnew(Label);
                String item_text = String("  >  [") + String::num_int64(link_id) + "]: " + String(link_pathnode_ref->get_name());
                lbl_link->set_text(item_text);
                lbl_links_vbox_container->add_child(lbl_link);
            }
        }

        btn_add->connect("pressed", Callable(this, "signal_pathnode_add_next_pathnode"));
        btn_snap->connect("pressed", Callable(this, "signal_pathnode_snap_to_ground"));

        add_custom_control(lbl_id);
        add_custom_control(lbl_prev_node_id);
        add_custom_control(lbl_section_id);
        add_custom_control(lbl_section_coord);
        add_custom_control(lbl_links_title);
        if (!target_pathnode->get_links().is_empty()) add_custom_control(lbl_links_vbox_container);
        add_custom_control(btn_add);
        add_custom_control(btn_snap);
    }

    void FlowAIEditorInspector::signal_manager_bake() {
        if (FlowAIManager* manager = Object::cast_to<FlowAIManager>(target_node)) {
            manager->bake_sections();
        }
    }

    void FlowAIEditorInspector::signal_manager_add_new_pathnode() {
        if (FlowAIManager* manager = Object::cast_to<FlowAIManager>(target_node)) {
            manager->add_new_pathnode();
        }
    }

    void FlowAIEditorInspector::signal_pathnode_add_next_pathnode() {
        if (FlowAIPathnode* pathnode = Object::cast_to<FlowAIPathnode>(target_node)) {
            pathnode->add_next_pathnode();
        }
    }

    void FlowAIEditorInspector::signal_pathnode_snap_to_ground() {
        if (FlowAIPathnode* pathnode = Object::cast_to<FlowAIPathnode>(target_node)) {
            pathnode->snap_to_ground();
        }
    }
}