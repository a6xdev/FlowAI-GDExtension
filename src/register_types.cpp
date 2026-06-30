#include "register_types.h"

#include "FlowAI.hpp"
#include "editor/FlowAIEditor.hpp"

#include <gdextension_interface.h>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/core/defs.hpp>

using namespace godot;
using namespace FlowAI;

void initialize_flow_ai_module(ModuleInitializationLevel p_level) {
    if (p_level == MODULE_INITIALIZATION_LEVEL_SCENE) {
        GDREGISTER_CLASS(FlowAIManager);
        GDREGISTER_CLASS(FlowAIPathnode);
        GDREGISTER_CLASS(FlowAIAgent3D);
        GDREGISTER_CLASS(FlowAIBakeData);
    }


    if (p_level == MODULE_INITIALIZATION_LEVEL_EDITOR) {
        GDREGISTER_CLASS(FlowAIEditorPlugin);
        GDREGISTER_CLASS(FlowAIEditorInspector);

        EditorPlugins::add_by_type<FlowAIEditorPlugin>();
    }
}

void uninitialize_flow_ai_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
}

extern "C" {
    GDExtensionBool GDE_EXPORT flow_ai_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization* r_initialization) {
        godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        init_obj.register_initializer(initialize_flow_ai_module);
        init_obj.register_terminator(uninitialize_flow_ai_module);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

        return init_obj.init();
    }
}