import os
import sys

projectdir = "flow_ai_project"
env = SConscript("godot-cpp/SConstruct")
env.Append(CPPPATH=["src/"])
sources = Glob("src/*.cpp")

lib_filename = "{}flow_ai{}{}".format(env.subst('$SHLIBPREFIX'), env["suffix"], env.subst('$SHLIBSUFFIX'))

library = env.SharedLibrary(
    "{}/addons/FlowAI/bin/{}".format(projectdir, lib_filename),
    source=sources,
)

Default(library)