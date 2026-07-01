import os
import sys

projectdir = "flow_ai_project"
env = SConscript("godot-cpp/SConstruct")
env.Append(CPPPATH=["src/"])
sources = [
    "src/register_types.cpp",
    "src/editor/FlowAIEditor.cpp",
    "src/editor/FlowAIEditorInspector.cpp",
    "src/nodes/FlowAIManager.cpp",
    "src/nodes/FlowAIPathnode.cpp",
    "src/nodes/FlowAIAgent3D.cpp",
    "src/resources/FlowAIBakeData.cpp",
    "src/systems/draw_sections_grid.cpp",
    "src/systems/draw_path.cpp",
    "src/systems/draw_pathnode_connections.cpp",
]

lib_filename = "{}flow_ai{}{}".format(env.subst('$SHLIBPREFIX'), env["suffix"], env.subst('$SHLIBSUFFIX'))

library = env.SharedLibrary(
    "{}/addons/FlowAI/bin/{}".format(projectdir, lib_filename),
    source=sources,
)

Default(library)