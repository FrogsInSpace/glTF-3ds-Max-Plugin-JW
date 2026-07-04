#include "KHRglTFImporter.h"
#include <iostream>
#include <json/json.h>

#pragma comment(lib, "jsoncpp_static.lib")

extern void GetJSONRoot(char* data, Json::Value& root);

void glTFImporter_Core::SetInteractivity(void)
{
    cgltf_extension* ext = m_glTF_data->data_extensions;
    for (int cnt = 0; cnt < m_glTF_data->data_extensions_count; cnt++, ext++) {
        char* name = ext->name;
        char* data = ext->data;
        if (!stricmp(name, "KHR_interactivity"))
            CreateInteractivityTable(data);
    }
}

void glTFImporter_Core::CreateInteractivityTable(char* data)
{
    Json::Value root;
    GetJSONRoot(data, root);

    auto types = root["types"];
    for (auto t : types) {
        std::string type = t["signature"].asString();
        if (type == "bool") {}
        if (type == "float") {}
        if (type == "float2") {}
        if (type == "float3") {}
        if (type == "bool") {}
    }
    auto events = root["events"];
    auto variables = root["variables"];
    auto nodes = root["nodes"];
}
