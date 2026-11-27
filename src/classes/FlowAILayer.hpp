#pragma once

#include "../FlowAI.hpp"

namespace FlowAI {
	class FlowAIManager;
	class FlowAIPathnode;

	class FlowAILayer : public Node {
		GDCLASS(FlowAILayer, Node)
	public:
		FlowAILayer();
		~FlowAILayer();

		void create_pathnode(int32_t _prevPathnodeID = -1);

		void set_layer_color(Color _color) { m_layerColor = _color; }

		FlowAIPathnode* get_pathnode(uint32_t _id);
		std::vector<FlowAIPathnode*> get_pathnode_list();
		TypedArray<FlowAIPathnode> get_pathnode_list_gd();
		Color get_layer_color() const { return m_layerColor; }

	protected:
		static void _bind_methods();
		void _notification(int p_what);
	private:
		Color m_layerColor = Color(1.0f, 1.0f, 1.0f, 1.0f);

		uint32_t get_available_pathnode_id();
	};
}