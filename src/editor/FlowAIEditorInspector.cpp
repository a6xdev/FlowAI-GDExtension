#include "FlowAIEditor.hpp"
#include "../classes/FlowAIManager.hpp"
#include "../classes/FlowAILayer.hpp"
#include "../classes/FlowAIPathnode.hpp"
#include "../classes/FlowAIAgent3D.hpp"
#include <godot_cpp/classes/v_box_container.hpp>

using namespace godot;

namespace FlowAI {
    FlowAIPathnode* pathnode_selected_01 = nullptr;
    FlowAIPathnode* pathnode_selected_02 = nullptr;

    void FlowAIEditorInspector::_bind_methods() {
        ClassDB::bind_method(D_METHOD("bake"), &FlowAIEditorInspector::bake);
        ClassDB::bind_method(D_METHOD("connect_pathnodes"), &FlowAIEditorInspector::connect_pathnodes);
        ClassDB::bind_method(D_METHOD("disconnect_pathnodes"), &FlowAIEditorInspector::disconnect_pathnodes);
        ClassDB::bind_method(D_METHOD("create_pathnode"), &FlowAIEditorInspector::create_pathnode);
        ClassDB::bind_method(D_METHOD("create_next_pathnode"), &FlowAIEditorInspector::create_next_pathnode);
        ClassDB::bind_method(D_METHOD("snap_all_pathnodes_to_ground"), &FlowAIEditorInspector::snap_all_pathnodes_to_ground);
        ClassDB::bind_method(D_METHOD("snap_to_ground"), &FlowAIEditorInspector::snap_to_ground);
    }

    bool FlowAIEditorInspector::_can_handle(Object* p_object) const {
        if (p_object == nullptr) return false;
        bool is_manager = Object::cast_to<FlowAIManager>(p_object) != nullptr;
        bool is_layer = Object::cast_to<FlowAILayer>(p_object) != nullptr;
        bool is_pathnode = Object::cast_to<FlowAIPathnode>(p_object) != nullptr;

        TypedArray<Node> selected = EditorInterface::get_singleton()->get_selection()->get_selected_nodes();
        if (selected.size() > 0) {
            if (Object::cast_to<FlowAIPathnode>(selected[0]) != nullptr) {
                return true;
            }
        }

        return is_manager || is_pathnode || is_layer;
    }

    void FlowAIEditorInspector::_parse_begin(Object* p_object) {
        if (p_object == nullptr) return;

        // Check if have 2 pathnodes selected
        TypedArray<Node> selected_nodes = EditorInterface::get_singleton()->get_selection()->get_selected_nodes();

        if (selected_nodes.size() == 2) {
            FlowAIPathnode* node_A = Object::cast_to<FlowAIPathnode>(selected_nodes[0]);
            FlowAIPathnode* node_B = Object::cast_to<FlowAIPathnode>(selected_nodes[1]);

            if (node_A && node_B) {
                _parser_two_pathnodes(node_A, node_B);
            }
        }

        target_node = p_object;
        if (FlowAIManager* manager = Object::cast_to<FlowAIManager>(p_object)) {
            _parse_manager(manager);
            return;
        }

        if (FlowAILayer* layer = Object::cast_to<FlowAILayer>(p_object)) {
            _parse_layer(layer);
            return;
        }

        if (FlowAIPathnode* pathnode = Object::cast_to<FlowAIPathnode>(p_object)) {
            _parse_pathnode(pathnode);
            return;
        }
    }

    // ----------------------- //
    // When user select two pathnodes on SceneTree
    // ----------------------- //
    void FlowAIEditorInspector::_parser_two_pathnodes(FlowAIPathnode* node_a, FlowAIPathnode* node_b) {
        pathnode_selected_01 = node_a; pathnode_selected_02 = node_b;

        Button* btn_connect_pathnodes = memnew(Button);
        Button* btn_disconnect_pathnodes = memnew(Button);

        String btn_title_text = "Pathnodes: '" + node_a->get_name() + "' to '" + node_b->get_name() + "'";

        btn_connect_pathnodes->set_text("Connect " + btn_title_text);
        btn_disconnect_pathnodes->set_text("Disconnect " + btn_title_text);

        btn_connect_pathnodes->connect("pressed", Callable(this, "connect_pathnodes"));
        btn_disconnect_pathnodes->connect("pressed", Callable(this, "disconnect_pathnodes"));

        add_custom_control(btn_connect_pathnodes);
        add_custom_control(btn_disconnect_pathnodes);
    }

    // ----------------------- //
    //      FlowAIManager
    // ----------------------- //
    void FlowAIEditorInspector::_parse_manager(Object* target_node) {
        FlowAIManager* target_manager = Object::cast_to<FlowAIManager>(target_node);
        if (!target_manager) return;

        Button* btn_bake = memnew(Button);
        btn_bake->set_text("Bake");
        btn_bake->connect("pressed", Callable(this, "bake"));

        add_custom_control(btn_bake);
    }

    // ----------------------- //
    //      FlowAILayer
    // ----------------------- //
    void FlowAIEditorInspector::_parse_layer(Object* target_node) {
        Button* btn_add_new_pathnode = memnew(Button);
        Button* btn_snap_all_pathnodes_to_ground = memnew(Button);

        btn_add_new_pathnode->set_text("Add New Pathnode");
        btn_snap_all_pathnodes_to_ground->set_text("Snap All Pathnodes to Ground");

        btn_add_new_pathnode->connect("pressed", Callable(this, "create_pathnode"));
        btn_snap_all_pathnodes_to_ground->connect("pressed", Callable(this, "snap_all_pathnodes_to_ground"));

        add_custom_control(btn_add_new_pathnode);
        add_custom_control(btn_snap_all_pathnodes_to_ground);
    }

    // ----------------------- //
    //      FlowAIPathnode
    // ----------------------- //
    void FlowAIEditorInspector::_parse_pathnode(Object* target_node) {
        FlowAIPathnode* target_pathnode = Object::cast_to<FlowAIPathnode>(target_node);
        if (!target_pathnode) return;

        // Get the pathnode manager
        //FlowAIManager* manager = FlowAIManager::get_singleton();
        FlowAILayer* layer = target_pathnode->get_layer_onwer();

        if (!layer) return;

        String prev_node_id_text = (target_pathnode->get_prev_node_id() == -1) ? "Nill" : String::num_int64(target_pathnode->get_prev_node_id());
        PackedInt32Array pathnode_links_id = target_pathnode->get_links();
        std::vector<FlowAIPathnode*> pathnodes_list = layer->get_pathnode_list();

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
        lbl_section_id->set_text("Section ID: " + String::num_int64(target_pathnode->get_sector_id()));
        lbl_section_coord->set_text("Section Coord: " + target_pathnode->get_sector_coord());
        lbl_links_title->set_text("Links: [Array] - " + String::num_int64(target_pathnode->get_links().size()));
        btn_add->set_text("Add Next Pathnode");
        btn_snap->set_text("Snap to Ground");

        // Show all nodes linked to the selected pathnode
        for (auto link_id : target_pathnode->get_links()) {
            FlowAIPathnode* link_pathnode_ref = pathnodes_list[link_id - 1]; // An array always start in index 0. HA!
            if (link_pathnode_ref) {
                Label* lbl_link = memnew(Label);
                String item_text = String("  >  [") + String::num_int64(link_id) + "]: " + String(link_pathnode_ref->get_name());
                lbl_link->set_text(item_text);
                lbl_links_vbox_container->add_child(lbl_link);
            }
        }

        btn_add->connect("pressed", Callable(this, "create_next_pathnode"));
        btn_snap->connect("pressed", Callable(this, "snap_to_ground"));

        add_custom_control(lbl_id);
        add_custom_control(lbl_prev_node_id);
        add_custom_control(lbl_section_id);
        add_custom_control(lbl_section_coord);
        add_custom_control(lbl_links_title);
        if (!target_pathnode->get_links().is_empty()) add_custom_control(lbl_links_vbox_container);
        add_custom_control(btn_add);
        add_custom_control(btn_snap);
    }

    void FlowAIEditorInspector::_parse_agent_3d(Object* target_node) {
        FlowAIAgent3D* agent_3d = Object::cast_to<FlowAIAgent3D>(target_node);
        if (!agent_3d) return;
    }

    /// ------------------------------------------
    /// SIGNALS
    /// ------------------------------------------
    void FlowAIEditorInspector::bake() {
        if (FlowAIManager* manager = Object::cast_to<FlowAIManager>(target_node)) {
            manager->bake_sections();
        }
    }

    void FlowAIEditorInspector::connect_pathnodes() {
        if (pathnode_selected_01 && pathnode_selected_02) {
            FlowAIManager::get_singleton()->connect_pathnodes(pathnode_selected_01, pathnode_selected_02);
        }
    }

    void FlowAIEditorInspector::disconnect_pathnodes() {
        if (pathnode_selected_01 && pathnode_selected_02) {
            FlowAIManager::get_singleton()->disconnect_pathnodes(pathnode_selected_01, pathnode_selected_02);
        }
    }

    void FlowAIEditorInspector::create_pathnode() {
        if (FlowAILayer* layer = Object::cast_to<FlowAILayer>(target_node)) {
            layer->create_pathnode();
        }
    }

    void FlowAIEditorInspector::create_next_pathnode() {
        if (FlowAIPathnode* pathnode = Object::cast_to<FlowAIPathnode>(target_node)) {
            pathnode->create_next_pathnode();
        }
    }

    void FlowAIEditorInspector::snap_all_pathnodes_to_ground() {
        if (FlowAIManager* manager = Object::cast_to<FlowAIManager>(target_node)) {
            std::vector<FlowAIPathnode*> all_pathnodes = manager->get_pathnode_list();
            for (auto child : all_pathnodes) {
                child->snap_to_ground();
            }
        }
    }


    void FlowAIEditorInspector::snap_to_ground() {
        if (FlowAIPathnode* pathnode = Object::cast_to<FlowAIPathnode>(target_node)) {
            pathnode->snap_to_ground();
        }
    }
}