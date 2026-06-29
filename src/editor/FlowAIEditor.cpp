#include "FlowAIEditor.hpp"
#include <godot_cpp/classes/editor_interface.hpp>

using namespace godot;

namespace FlowAI {
    const int MAX_LAYERS = 32;

    void FlowAIEditorPlugin::_bind_methods() {}

    FlowAIEditorPlugin::FlowAIEditorPlugin() {
        flowai_inspector.instantiate();
        add_inspector_plugin(flowai_inspector);

        _setup_project_settings();
    }

    FlowAIEditorPlugin::~FlowAIEditorPlugin() {
        if (flowai_inspector.is_valid()) {
            remove_inspector_plugin(flowai_inspector);
            flowai_inspector.unref();
        }
    }

    void FlowAIEditorPlugin::_setup_project_settings() {
        ProjectSettings* settings = ProjectSettings::get_singleton();
        if (!settings) return;

        // Visualize Connections
        if (!settings->has_setting(d_visualize_connections)) { settings->set_setting(d_visualize_connections, true); }
        if (!settings->has_setting(d_visualize_sections_grid)) { settings->set_setting(d_visualize_sections_grid, true); }

        settings->set_initial_value(d_visualize_connections, true);
        settings->set_initial_value(d_visualize_sections_grid, true);

        PropertyInfo debug_info_visualize_connections(Variant::BOOL, d_visualize_connections);
        PropertyInfo debug_info_visualize_sections_grid(Variant::BOOL, d_visualize_sections_grid);

        settings->add_property_info(debug_info_visualize_connections);
        settings->add_property_info(debug_info_visualize_sections_grid);

        // Navigation Layers Colors
        Color default_layer_color = Color(0.0f, 0.6f, 1.0f, 0.7f);

        for (int i = 1; i <= MAX_LAYERS; ++i) {
            String color_path = "flow_ai/layer_colors/layer_" + String::num_int64(i);

            if (!settings->has_setting(color_path)) {
                settings->set_setting(color_path, default_layer_color);
            }

            settings->set_initial_value(color_path, default_layer_color);
            settings->add_property_info(PropertyInfo(Variant::COLOR, color_path));
        }



        // save this shit
        settings->save();
    }

    bool FlowAIEditorPlugin::_handles(Object* p_object) const { return false; }

    void FlowAIEditorPlugin::_edit(Object* p_object) { node_selected = p_object; }

    void FlowAIEditorPlugin::_make_visible(bool p_visible) {}
}