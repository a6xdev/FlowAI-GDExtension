#pragma once 

#include <godot_cpp/classes/node3d.hpp>

namespace godot {
	class FlowAIArea : public Node3D {
		GDCLASS(FlowAIArea, Node3D);
	public:
		FlowAIArea();
		~FlowAIArea();
	};
}