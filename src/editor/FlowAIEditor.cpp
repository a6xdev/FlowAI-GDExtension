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

        String d_enable_logging = get_d_enable_logging();
        String d_visualize_pathnode_debug = get_d_visualize_pathnode_debug();
        String d_visualize_section_debug = get_d_visualize_section_debug();
        String d_visualize_connections = get_d_visualize_connections();
        String d_visualize_agent_path = get_d_visualize_agent_path();

        // Visualize Options
        if (!settings->has_setting(d_enable_logging)) { settings->set_setting(d_enable_logging, true); }
        if (!settings->has_setting(d_visualize_pathnode_debug)) { settings->set_setting(d_visualize_pathnode_debug, true); }
        if (!settings->has_setting(d_visualize_section_debug)) { settings->set_setting(d_visualize_section_debug, true); }
        if (!settings->has_setting(d_visualize_connections)) { settings->set_setting(d_visualize_connections, true); }
        if (!settings->has_setting(d_visualize_agent_path)) { settings->set_setting(d_visualize_agent_path, true); }

        settings->set_initial_value(d_enable_logging, false);
        settings->set_initial_value(d_visualize_pathnode_debug, true);
        settings->set_initial_value(d_visualize_section_debug, true);
        settings->set_initial_value(d_visualize_connections, true);
        settings->set_initial_value(d_visualize_agent_path, true);

        PropertyInfo debug_info_enable_logging(Variant::BOOL, d_enable_logging);
        PropertyInfo debug_info_visualize_pathnode_debug(Variant::BOOL, d_visualize_pathnode_debug);
        PropertyInfo debug_info_visualize_sections_debug(Variant::BOOL, d_visualize_section_debug);
        PropertyInfo debug_info_visualize_connections(Variant::BOOL, d_visualize_connections);
        PropertyInfo debug_info_visualize_agent_path(Variant::BOOL, d_visualize_agent_path);

        settings->add_property_info(debug_info_enable_logging);
        settings->add_property_info(debug_info_visualize_pathnode_debug);
        settings->add_property_info(debug_info_visualize_sections_debug);
        settings->add_property_info(debug_info_visualize_connections);
        settings->add_property_info(debug_info_visualize_agent_path);

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
        Error err = settings->save();
    }

    bool FlowAIEditorPlugin::_handles(Object* p_object) const { return false; }

    void FlowAIEditorPlugin::_edit(Object* p_object) { node_selected = p_object; }

    void FlowAIEditorPlugin::_make_visible(bool p_visible) {}

    // CALLS
    bool is_debug_enabled(DebugOption p_option) {
        godot::ProjectSettings* settings = godot::ProjectSettings::get_singleton();
        if (!settings) return false;

        String target_path = "";

        switch (p_option) {
        case DEBUG_ENABLE_LOGGING: target_path = get_d_enable_logging(); break;
        case DEBUG_VISUALIZE_PATHNODE: target_path = get_d_visualize_pathnode_debug(); break;
        case DEBUG_VISUALIZE_SECTION: target_path = get_d_visualize_section_debug(); break;
        case DEBUG_VISUALIZE_AGENT_PATH: target_path = get_d_visualize_agent_path(); break;
        case DEBUG_VISUALIZE_CONNECTIONS: target_path = get_d_visualize_connections(); break;
        }

        if (!target_path.is_empty() && settings->has_setting(target_path)) {
            return settings->get_setting(target_path);
        }

        return true;
    }

    Color get_color_from_navigation_layer_mask(uint32_t _layer_mask) {
        godot::ProjectSettings* settings = godot::ProjectSettings::get_singleton();
        Color default_color = Color(1.0f, 1.0f, 1.0f);
        int active_layer = 1;

        for (int i = 0; i < 32; ++i) {
            if (_layer_mask & (1 << i)) {
                active_layer = i + 1;
                break;
            }
        }

        String color_path = "flow_ai/layer_colors/layer_" + String::num_int64(active_layer);

        if (settings && settings->has_setting(color_path)) {
            return VariantCaster<Color>::cast(settings->get_setting(color_path));
        }

        return default_color;
    }
}