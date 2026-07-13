import os
import sys
import shutil

SOURCE_ASSETS_DIR = "#assets"
OUTPUT_ADDON_ASSETS_DIR = "#flow_ai_project/addons/FlowAI/assets"

def copy_assets(target, source, env):
    origin_folder_src = env.GetLaunchDir() + "/assets"
    destination = env.GetLaunchDir() + "/flow_ai_project/addons/FlowAI/assets"

    if os.path.exists(origin_folder_src):
        if os.path.exists(destination):
            shutil.rmtree(destination)
        
        shutil.copytree(origin_folder_src, destination)
        print("[FlowAI] Assets copied with sucess!")
    return None


projectdir = "flow_ai_project"
env = SConscript("godot-cpp/SConstruct")
env.Append(CPPPATH=["src/"])
sources = [
    "src/register_types.cpp",
    "src/nodes/FlowAIManager.cpp",
    "src/nodes/FlowAIPathnode.cpp",
    "src/nodes/FlowAIAgent3D.cpp",
    "src/debug/FlowAIAgentDebug.cpp",
    "src/debug/FlowAIManagerDebug.cpp",
    "src/editor/FlowAIEditor.cpp",
    "src/editor/FlowAIEditorInspector.cpp",
    "src/resources/FlowAIBakeData.cpp",
]

lib_filename = "{}flow_ai{}{}".format(env.subst('$SHLIBPREFIX'), env["suffix"], env.subst('$SHLIBSUFFIX'))

library = env.SharedLibrary(
    "{}/addons/FlowAI/bin/{}".format(projectdir, lib_filename),
    source=sources,
)
env.AddPostAction(library, Action(copy_assets, "[FlowAI] Copying assets to output directory..."))

Default(library)