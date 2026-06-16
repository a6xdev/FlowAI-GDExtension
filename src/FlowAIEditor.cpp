#include "FlowAIEditor.hpp"
#include <godot_cpp/classes/editor_interface.hpp>

void FlowAIEditorPlugin::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_on_create_new_pathnode_pressed"), &FlowAIEditorPlugin::_on_create_new_pathnode_pressed);
}

FlowAIEditorPlugin::FlowAIEditorPlugin() {
    btn_new_pathnode = memnew(Button);
    btn_new_pathnode->set_text("Add Pathnode");

    btn_new_pathnode->connect("pressed", Callable(this, "_on_create_new_pathnode_pressed"));
}

FlowAIEditorPlugin::~FlowAIEditorPlugin() {
}

bool FlowAIEditorPlugin::_handles(Object* p_object) const {
    return p_object->is_class("FlowAIManager");
}

void FlowAIEditorPlugin::_edit(Object* p_object) {
    manager_selected = Object::cast_to<FlowAIManager>(p_object);
}

void FlowAIEditorPlugin::_make_visible(bool p_visible) {
    if (p_visible) {
        if (btn_new_pathnode && btn_new_pathnode->get_parent() == nullptr)
            add_control_to_container(CONTAINER_SPATIAL_EDITOR_MENU, btn_new_pathnode);
    }
    else {
        if (btn_new_pathnode && btn_new_pathnode->get_parent() != nullptr)
            remove_control_from_container(CONTAINER_SPATIAL_EDITOR_MENU, btn_new_pathnode);
    }
}

void FlowAIEditorPlugin::_on_create_new_pathnode_pressed() {
    if (manager_selected) {
        manager_selected->add_new_pathnode();
    }
}