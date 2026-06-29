#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace FlowAI {
	class FlowAIBakeData : public Resource {
		GDCLASS(FlowAIBakeData, Resource)
	public:
		FlowAIBakeData();
		~FlowAIBakeData();

		void set_sectors_payload(const Dictionary& p_payload) { sectors_payload = p_payload; }
		Dictionary get_sectors_payload() const { return sectors_payload; }
	private:
		Dictionary sectors_payload;
	protected:
		static void _bind_methods();
	};
}