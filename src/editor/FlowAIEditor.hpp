#ifndef FLOW_AI_EDITOR_PLUGIN_H
#define FLOW_AI_EDITOR_PLUGIN_H

#include <godot_cpp/classes/editor_inspector.hpp>
#include <godot_cpp/classes/editor_property.hpp>
#include <godot_cpp/classes/editor_inspector_plugin.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/label.hpp>
#include "FlowAI.hpp"

using namespace godot;

namespace FlowAI {
	// Nodes Inspector
	class FlowAIEditorInspector : public EditorInspectorPlugin {
		GDCLASS(FlowAIEditorInspector, EditorInspectorPlugin)
	public:
		bool _can_handle(Object* p_object) const override;
		void _parse_begin(Object* p_object) override;

	protected:
		static void _bind_methods();
	private:
		Object* target_node = nullptr;

		void _parse_manager(Object* target_node);
		void _parse_pathnode(Object* target_node);
		void _manager_add_new_pathnode();
		void _pathnode_add_next_pathnode();
		void _pathnode_snap_to_ground();
	};

	// FlowAI Editor
	class FlowAIEditorPlugin : public EditorPlugin {
		GDCLASS(FlowAIEditorPlugin, EditorPlugin)
	public:
		FlowAIEditorPlugin();
		~FlowAIEditorPlugin();

		virtual bool _handles(Object* p_object) const override;
		virtual void _make_visible(bool p_visible) override;
		virtual void _edit(Object* p_object) override;
	protected:
		static void _bind_methods();
	private:
		Object* node_selected = nullptr;
		Ref<FlowAIEditorInspector> flowai_inspector;

		void _setup_project_settings();
	};
}

#endif