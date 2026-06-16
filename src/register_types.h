#pragma once

#include <godot_cpp/core/class_db.hpp>

// Funções que o Godot vai chamar ao carregar e descarregar o plugin
void initialize_flow_ai_module(godot::ModuleInitializationLevel p_level);
void uninitialize_flow_ai_module(godot::ModuleInitializationLevel p_level);