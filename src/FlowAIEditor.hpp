#ifndef FLOW_AI_EDITOR_PLUGIN_H
#define FLOW_AI_EDITOR_PLUGIN_H

#include <godot_cpp/classes/editor_plugin.hpp>
#include <godot_cpp/classes/button.hpp>
#include "FlowAI.hpp"

using namespace godot;

class FlowAIEditorPlugin : public EditorPlugin {
	GDCLASS(FlowAIEditorPlugin, EditorPlugin)
private:
	Button* btn_new_pathnode = nullptr;
	FlowAIManager* manager_selected = nullptr;
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

#endif