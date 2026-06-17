#include "FlowAIEditor.hpp"
#include <godot_cpp/classes/v_box_container.hpp>

using namespace godot;

namespace FlowAI {
    // FlowAIPathnode
    void FlowAIPathnodeInspector::_bind_methods() {
        ClassDB::bind_method(D_METHOD("_on_add_next_pressed"), &FlowAIPathnodeInspector::_on_add_next_pressed);
        ClassDB::bind_method(D_METHOD("_on_snap_ground_pressed"), &FlowAIPathnodeInspector::_on_snap_ground_pressed);
    }

    bool FlowAIPathnodeInspector::_can_handle(Object* p_object) const {
        return p_object->is_class("FlowAIPathnode");
    }

    void FlowAIPathnodeInspector::_parse_begin(Object* p_object) {
        target_pathnode = Object::cast_to<FlowAIPathnode>(p_object);
        if (!target_pathnode) return;

        String prev_node_id_text = (target_pathnode->get_prev_node_id() == -1) ? "Nill" : String::num_int64(target_pathnode->get_prev_node_id());
        String sector_id_text = (target_pathnode->get_sector_id() == -1) ? "Nill" : String::num_int64(target_pathnode->get_sector_id());

        Label* lbl_id = memnew(Label);
        Label* lbl_prev_node_id = memnew(Label);
        Label* lbl_sector = memnew(Label);
        Label* lbl_links_title = memnew(Label);
        VBoxContainer* lbl_links_vbox_container = memnew(VBoxContainer);
        Button* btn_add = memnew(Button);
        Button* btn_snap = memnew(Button);

        lbl_id->set_text("Pathnode ID: " + String::num_int64(target_pathnode->get_id()));
        lbl_prev_node_id->set_text("Previous Pathnode ID: " + prev_node_id_text);
        lbl_sector->set_text("Sector ID: " + sector_id_text);
        lbl_links_title->set_text("Links: [Array] - " + String::num_int64(target_pathnode->get_links().size()));
        btn_add->set_text("Add Next Pathnode");
        btn_snap->set_text("Snap to Ground");

        // Show all nodes kinked to the selected pathnode
        for (auto link_id : target_pathnode->get_links()) {
            Label* lbl_link = memnew(Label);
            String item_text = String("  >  [") + String::num_int64(link_id) + "]: Connected Node";
            lbl_link->set_text(item_text);
            lbl_links_vbox_container->add_child(lbl_link);
        }

        btn_add->connect("pressed", Callable(this, "_on_add_next_pressed"));
        btn_snap->connect("pressed", Callable(this, "_on_snap_ground_pressed"));

        add_custom_control(lbl_id);
        add_custom_control(lbl_prev_node_id);
        add_custom_control(lbl_sector);
        add_custom_control(lbl_links_title);
        if (!target_pathnode->get_links().is_empty()) add_custom_control(lbl_links_vbox_container);
        add_custom_control(btn_add);
        add_custom_control(btn_snap);
    }

    void FlowAIPathnodeInspector::_on_add_next_pressed() {
        if (target_pathnode) target_pathnode->add_next_pathnode();
    }

    void FlowAIPathnodeInspector::_on_snap_ground_pressed() {
        if (target_pathnode) target_pathnode->snap_to_ground();
    }

    // FlowAIManager
}