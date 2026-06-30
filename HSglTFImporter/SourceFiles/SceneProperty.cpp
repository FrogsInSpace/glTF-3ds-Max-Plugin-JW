/*
 * Copyright (c) 2024-2026 The Khronos Group Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 //**************************************************************************/
 // AUTHOR: Satoshi Hayashi 
 //***************************************************************************/

#include "HSglTFImporter.h"

//======================================================================
//======================================================================
void glTFImporter_Core::SetSceneProperties(void)
{
	cgltf_scene *scenes = m_glTF_data->scene;
	if (!scenes) return;

	cgltf_extras *extras = &scenes->extras;
	if (!extras) return;

	cgltf_size size;
	cgltf_result ret = cgltf_copy_extras_json(m_glTF_data, extras, NULL, &size);
	if (size > 0) {
		std::vector<custAttrParam> attrTbl;
		CreateParamTableFromExtras(*extras, size, attrTbl, TRUE);
		AttachSceneProp(attrTbl);
	}
}

//======================================================================
//======================================================================
void glTFImporter_Core::AttachSceneProp(std::vector<custAttrParam>& attrTbl)
{
	if (attrTbl.size() == 0) return;

	PROPSPEC PropSpec;
	PROPVARIANT PropVar;
	PropSpec.ulKind = PRSPEC_LPWSTR;
	PropSpec.propid = PIDSI_TITLE;

	TSTR string;

	for (auto param : attrTbl) {
		TSTR name(StringToWString(param.name.c_str()).c_str());
		PropSpec.lpwstr = (wchar_t*)name.data();
		switch (param.type) {
		case TYPE_INT:
			PropVar.vt = VT_INT;
			PropVar.intVal = param.iParam;
			break;
		case TYPE_FLOAT:
			PropVar.vt = VT_R4;
			PropVar.fltVal = param.fParam;
			break;

		case TYPE_STRING:
			PropVar.vt = VT_LPWSTR;
			string = StringToWString(param.sParam.c_str()).c_str();
			//_tcscpy(PropVar.pwszVal, string.data());
			PropVar.pwszVal = (wchar_t*)string.data();
			break;
		}

		GetCOREInterface()->AddProperty(PROPSET_USERDEFINED, &PropSpec, &PropVar);
	}
}

//======================================================================
//======================================================================
void glTFImporter_Core::SetSceneInfos(void)
{
	cgltf_asset* asset = &m_glTF_data->asset;
	if (!asset) return;

	PROPSPEC PropSpec;
	PropSpec.ulKind = PRSPEC_PROPID;
	PropSpec.propid = PIDSI_COMMENTS;

	PROPVARIANT PropVar;
	PropVar.vt = VT_LPWSTR;

	TSTR string = _T("Source FileName: ");
	string += TSTR(tstring(m_fullpath.filename()).c_str());
	string += _T("\r\n");
	string += _T("Copyright: ");
	string += StringToWString(asset->copyright).c_str();
	string += _T("\r\n");
	string += _T("Generator: ");
	string += StringToWString(asset->generator).c_str();
	string += _T("\r\n");
	string += _T("Importer: HSglTFImporter");
	PropVar.pwszVal = (wchar_t*)string.data();

	GetCOREInterface()->AddProperty(PROPSET_SUMMARYINFO, &PropSpec, &PropVar);

}
