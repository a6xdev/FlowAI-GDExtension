#include "FlowAIEditor.hpp"
#include <godot_cpp/classes/editor_interface.hpp>

using namespace godot;

namespace FlowAI {
    void FlowAIEditorPlugin::_bind_methods() {}

    FlowAIEditorPlugin::FlowAIEditorPlugin() {
        flowai_inspector.instantiate();
        add_inspector_plugin(flowai_inspector);
    }

    FlowAIEditorPlugin::~FlowAIEditorPlugin() {
        if (flowai_inspector.is_valid()) {
            remove_inspector_plugin(flowai_inspector);
            flowai_inspector.unref();
        }
    }

    bool FlowAIEditorPlugin::_handles(Object* p_object) const { return false; }

    void FlowAIEditorPlugin::_edit(Object* p_object) { node_selected = p_object; }

    void FlowAIEditorPlugin::_make_visible(bool p_visible) {}
}