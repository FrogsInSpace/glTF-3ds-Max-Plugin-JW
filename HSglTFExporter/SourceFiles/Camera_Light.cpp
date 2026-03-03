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

#include "HSglTFExporter.h"
#include <lslights.h>

#define ArnoldLightClassID Class_ID(0x6705f00d, 0xca131d05)


BOOL IsIESLight(const TSTR& name)
{
	return FALSE;
}

//======================================================================
//======================================================================
tinygltf::Camera glTFExporter_Core::CreateCamera(INode *pNode)
{
	tinygltf::Camera camera;// = Create_glTFCamera(pNode->GetName());
	SetName(&camera, pNode->GetName());
	camera.name = WStringToString(pNode->GetName());

	GenCamera *pCamera = (GenCamera*)pNode->GetObjectRef();
	if (pCamera->IsOrtho()) {
		camera.type = "orthographic";
		camera.orthographic.xmag = 1.0f;
		camera.orthographic.ymag = 1.0f;
		camera.orthographic.zfar = pCamera->GetClipDist(m_time, CAM_YON_CLIP);;
		float n = pCamera->GetClipDist(m_time, CAM_HITHER_CLIP);
		camera.orthographic.znear = n == 0.0f ? 0.1f : n;
	}
	else {
		camera.type = "perspective";
		camera.perspective.aspectRatio = GetCOREInterface()->GetRendImageAspect();
		camera.perspective.yfov = pCamera->GetFOV(m_time);
		camera.perspective.zfar = pCamera->GetClipDist(m_time, CAM_YON_CLIP);;
		float n = pCamera->GetClipDist(m_time, CAM_HITHER_CLIP);
		camera.perspective.znear = n == 0.0f ? 0.1f : n;
	}

	m_model.cameras.push_back(camera);

	return camera;
}

//======================================================================
//======================================================================
tinygltf::Light glTFExporter_Core::CreateLight(INode *pNode)
{
	tinygltf::Light light;// = Create_glTFLight(pNode->GetName());
	SetName(&light, pNode->GetName());
	light.name = WStringToString(pNode->GetName());

	GenLight *pLight = (GenLight*)pNode->GetObjectRef();

	/*
	if (pLight->ClassID() == ArnoldLightClassID) {
		IParamBlock2* pBlock = pLight->GetParamBlock(0);
		TSTR fName = pBlock->GetStr(43);
		if (IsIESLight(fName)) {
			m_LightIESMap.insert(std::make_pair(pLight, m_model.lights.size() - 1));
			m_LlightsIES_Used = TRUE;
			return light;
		}
	}
	*/

	Point3 col = pLight->GetRGBColor(m_time);
	light.color.clear();
	light.color.push_back(col.x > 1.0f ? 1.0f : col.x);
	light.color.push_back(col.y > 1.0f ? 1.0f : col.y);
	light.color.push_back(col.z > 1.0f ? 1.0f : col.z);
	light.intensity = pLight->GetIntensity(m_time);

	int xx = pLight->Type();
	if (pLight->ClassID() == LS_POINT_LIGHT_ID || pLight->ClassID() == LS_POINT_LIGHT_TARGET_ID) {
		LightscapeLight* plsLight = (LightscapeLight*)pLight;
		int xx = plsLight->GetDistribution();
		if (plsLight->GetDistribution()== LightscapeLight::SPOTLIGHT_DIST) {
			light.type = "spot";
			IParamBlock2* pBlock = plsLight->GetParamBlock(4);
			light.range = pBlock->GetFloat(7, m_time);
			pBlock = plsLight->GetParamBlock(2);
			light.spot.innerConeAngle = DegToRad(pBlock->GetFloat(0, m_time));
			light.spot.outerConeAngle = DegToRad(pBlock->GetFloat(1, m_time));
		}
		else {
			light.type = "point";
		}
		Point3 col = plsLight->GetRGBFilter(m_time);
		light.color.clear();
		light.color.push_back(col.x > 1.0f ? 1.0f : col.x);
		light.color.push_back(col.y > 1.0f ? 1.0f : col.y);
		light.color.push_back(col.z > 1.0f ? 1.0f : col.z);
	}
	else if (pLight->Type() == OMNI_LIGHT || pLight->Type() == LightscapeLight::LightTypes::POINT_TYPE) {
		light.type = "point";
		if (pLight->GetDecayType()!=0) {
			light.range = pLight->GetDecayRadius(m_time);
		}
	}
	else if (pLight->Type() == DIR_LIGHT || pLight->Type() == TDIR_LIGHT) {
		light.type = "directional";
	}
	else if (pLight->Type() == FSPOT_LIGHT || pLight->Type() == TSPOT_LIGHT) {
		light.type = "spot";
		light.range = pLight->GetTDist(m_time);
		light.spot.innerConeAngle = DegToRad(pLight->GetHotspot(m_time));
		light.spot.outerConeAngle = DegToRad(pLight->GetFallsize(m_time));
	}

	m_model.lights.push_back(light);

	m_LightMap.insert(std::make_pair(pLight, m_model.lights.size() - 1));
	m_LlightsPunctual_Used = TRUE;

	return light;
}

