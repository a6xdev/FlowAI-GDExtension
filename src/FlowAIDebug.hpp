#include <iostream>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/transform3d.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/classes/ref.hpp>
#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/classes/immediate_mesh.hpp>
#include <godot_cpp/classes/standard_material3d.hpp>

using namespace godot;

namespace FlowAIDebug {
	static Color agent_path_line_color = Color(1.0f, 0.0f, 0.0f, 0.5f);
	static Color pathnode_connections_color = Color(0.157f, 0.157f, 0.769f, 0.8f);
	static Color grid_main_color = Color(0.0f, 1.0f, 0.5f, 0.4f);
	static Color active_sector_color = Color(0.0f, 0.5f, 0.5f, 0.4f);

	static bool pathnode_debug = true;
	static bool section_debug = true;
	static bool draw_agent_path = true;
	static bool draw_sections_grid = true;
	static bool draw_x_in_sections_that_have_pathnode = true;
	static bool draw_pathnode_connections_grid = true;
}