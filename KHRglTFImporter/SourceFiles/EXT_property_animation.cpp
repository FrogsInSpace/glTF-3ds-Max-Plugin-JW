/// TODO: source file is not used anywhere in the project and should be removed from the public repo

#include "HSglTFImporter.h"

#undef is_array
#undef is_string

#include <nlohmann/json.hpp>

using json = nlohmann::json;

//======================================================================
//======================================================================
void glTFImporter_Core::SetPropertyAnimation(int animID)
{
	cgltf_animation* animation = &m_glTF_data->animations[animID];
	if (!animation) return;

	int num = animation->extensions_count;
	if (num == 0) return;

	for (int i = 0; i < num; i++) {
		cgltf_extension* extension = animation->extensions++;
		if (extension->name != "EXT_property_animation") continue;

		json j = json::parse(extension->data);
		for (auto& element : j) {
			std::cout << element << '\n';
		}



		//json j = R"({ "happy": true, "pi": 3.141 })"_json;

	}
}
