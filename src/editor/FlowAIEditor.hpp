#ifndef FLOW_AI_EDITOR_PLUGIN_H
#define FLOW_AI_EDITOR_PLUGIN_H

#include <godot_cpp/classes/editor_inspector.hpp>
#include <godot_cpp/classes/editor_inspector_plugin.hpp>
#include <godot_cpp/classes/editor_interface.hpp>
#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/label.hpp>
#include "FlowAI.hpp"

using namespace godot;

namespace FlowAI {
	// Nodes Inspector
	class FlowAIPathnodeInspector : public EditorInspectorPlugin {
		GDCLASS(FlowAIPathnodeInspector, EditorInspectorPlugin)
	public:
		bool _can_handle(Object* p_object) const override;
		void _parse_begin(Object* p_object) override;
		void _on_add_next_pressed();
		void _on_snap_ground_pressed();
	private:
		FlowAIPathnode* target_pathnode = nullptr;
	protected:
		static void _bind_methods();
	};

	// FlowAI Editor
	class FlowAIEditorPlugin : public EditorPlugin {
		GDCLASS(FlowAIEditorPlugin, EditorPlugin)
	private:
		Button* btn_new_pathnode = nullptr;

		FlowAIManager* manager_selected = nullptr;
		Ref<FlowAIPathnodeInspector> pathnode_inspector;
	protected:
		static void _bind_methods();
	public:
		FlowAIEditorPlugin();
		~FlowAIEditorPlugin();

		virtual bool _handles(Object* p_object) const override;
		virtual void _make_visible(bool p_visible) override;
		virtual void _edit(Object* p_object) override;

		void _on_create_new_pathnode_pressed();
	};

}

#endif