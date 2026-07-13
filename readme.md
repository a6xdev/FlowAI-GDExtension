# FlowAI (v1.0)

## What is FlowAI
FlowAI is a hierarchical pathfinding system (HPA*) for Godot 4, designed for large-scale worlds with multiple simultaneous agents.

## Docs

- [FlowAI Concepts](docs/concepts.md)

## Installation
1. Download the latest release from the Releases page
2. Copy the `addons/flowai/` folder into your project's `addons/` directory

## Quick How-To
1. Adds a ``FlowAIManager`` to your scene.
2. Select your ``FlowAIManager`` and you will have access to some buttons on Inspector.
3. Click on ``Add New Pathnode``.
4. Select your new pathnode and you will have access to the "Add Next Pathnode" on Inspector. Now, you can just populate your scene with all pathnodes you need.
5. Select your ``FlowAIManager`` and create a new ``Bake Data``. When creating it, click on ``Bake`` button to create all sections.
6. Add a ``FlowAIAgent3D`` on your NPC scene and configure it:
```js
func _ready() -> void:
	agent.set_random_path(false)

func _physics_process(delta: float) -> void:
		if agent.is_path_complete():
			agent.set_random_path(false)
		
		var target = agent.get_next_pathnode_position()
		var direction:Vector3 = (target - global_position).normalized()
		velocity = direction * 10.0
		move_and_slide()

```
7. Now, execute your scene and check if everything is working well.

## Building From Source
### Requirements
- [SCons](https://scons.org/) 4.0+
- Python 3.6+
- C++17 compiler (MSVC 2019+, GCC 9+, Clang 10+)
- [godot-cpp](https://github.com/godotengine/godot-cpp) (included as submodule)

### Setup
```bash
git clone --recursive https://github.com/yourname/flowai
cd flowai
git submodule add https://github.com/godotengine/godot-cpp
git submodule update --init --recursive
```

### Compile
```bash
# Debug (Development)
scons target=template_debug

# Release
scons target=template_release

# Windows cross-compile no Linux
scons platform=windows target=template_release
```

### Output
The compiled DLL will to `flow_ai_project/addons/FlowAI/bin/`.
