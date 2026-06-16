#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/core/utility_functions.hpp>

namespace godot {
	class FLowAIController : public Node {
		GDCLASS(FLowAIController, Node);
	public:
		bool show_pathnodes_lines = false;
		bool show_pathnodes_shape = false;
		bool show_pathnode_label = false;

		FlowAIController();
		~FlowAIController();

		void _process(double delta) override;

		void add_new_area();
	};
}