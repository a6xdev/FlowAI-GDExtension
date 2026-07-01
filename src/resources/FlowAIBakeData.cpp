#include "../FlowAI.hpp"

namespace FlowAI {
	FlowAIBakeData::FlowAIBakeData() {}
    FlowAIBakeData::~FlowAIBakeData() {}

    void FlowAIBakeData::_bind_methods() {
        ClassDB::bind_method(D_METHOD("set_sectors_payload", "p_payload"), &FlowAIBakeData::set_sectors_payload);
        ClassDB::bind_method(D_METHOD("get_sectors_payload"), &FlowAIBakeData::get_sectors_payload);

        ADD_PROPERTY(PropertyInfo(Variant::DICTIONARY, "sectors_payload"), "set_sectors_payload", "get_sectors_payload");
    }
}