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

#include "KHRglTFImporter.h"

//======================================================================
//======================================================================
INode* glTFImporter_Core::CreateCamera(cgltf_node *node)
{
	cgltf_camera *camera = node->camera;

	tstring name;
	const char *pname = node->name;
	if (!pname) name = StringToWString(camera->name);

	GenCamera  *pCamObj = (GenCamera *)GetCOREInterface()->CreateInstance(CAMERA_CLASS_ID, Class_ID(SIMPLE_CAM_CLASS_ID, 0));
	pCamObj->Enable(1);
	pCamObj->SetType(FREE_CAMERA);
	pCamObj->SetManualClip(FALSE);

	if (camera->type == cgltf_camera_type::cgltf_camera_type_perspective) {
		cgltf_camera_perspective *perspective = &camera->data.perspective;
		pCamObj->SetOrtho(FALSE);

		if (perspective->has_zfar) {
			pCamObj->SetManualClip(TRUE);
			pCamObj->SetClipDist(m_time, CAM_HITHER_CLIP, perspective->znear * m_scale);
			pCamObj->SetClipDist(m_time, CAM_YON_CLIP, perspective->zfar * m_scale);
		}
		float f = perspective->yfov;
		float asp = 1.0;
		if (perspective->has_aspect_ratio) asp = perspective->aspect_ratio;
		pCamObj->SetFOV(m_time, f);
	}
	else if (camera->type == cgltf_camera_type_orthographic) {
		cgltf_camera_orthographic *orthographic = &camera->data.orthographic;
		pCamObj->SetOrtho(TRUE);

		float cl = 0.0f;//orthographic->;
		float ct = 0.0f;//orthographic->;
		float cr = 0.0f;//orthographic->;
		float cb = 0.0f;//orthographic->;
		pCamObj->SetManualClip(FALSE);
		pCamObj->SetClipDist(m_time, CAM_HITHER_CLIP, orthographic->znear * m_scale);
		pCamObj->SetClipDist(m_time, CAM_YON_CLIP, orthographic->zfar * m_scale);
	}

	INode *pNode = GetCOREInterface()->CreateObjectNode(pCamObj);
	pNode->SetName(name.c_str()); //StringToWString(name.C_Str()).c_str());

	m_CameraMap.insert(std::make_pair(camera, pCamObj));

	return pNode;
}

//======================================================================
//======================================================================
INode* glTFImporter_Core::CreateLight(cgltf_node *node)
{
	cgltf_light *light = node->light;

	tstring name = StringToWString(light->name);
	if (name.length()==0) name = StringToWString(node->name);

	IParamBlock* pBlock = NULL;
	GenLight  *pLightObj = NULL;
	float intensity = light->intensity;
	switch ((cgltf_light_type)light->type) {
	case cgltf_light_type_directional:
		pLightObj = (GenLight*)GetCOREInterface()->CreateInstance(LIGHT_CLASS_ID, Class_ID(DIR_LIGHT_CLASS_ID, 0));
		break;
	case cgltf_light_type_point:
		pLightObj = (GenLight*)GetCOREInterface()->CreateInstance(LIGHT_CLASS_ID, Class_ID(OMNI_LIGHT_CLASS_ID, 0));
		if (light->range > 0.0) pLightObj->SetTDist(m_time, light->range * m_scale);
		intensity = intensity * m_LiteIntensityScale;
		break;
	case cgltf_light_type_spot:
		pLightObj = (GenLight*)GetCOREInterface()->CreateInstance(LIGHT_CLASS_ID, Class_ID(FSPOT_LIGHT_CLASS_ID, 0));
		if (light->range > 0.0) pLightObj->SetTDist(m_time, light->range * m_scale);
		intensity = intensity * m_LiteIntensityScale;
		break;
	default:
		pLightObj = (GenLight*)GetCOREInterface()->CreateInstance(LIGHT_CLASS_ID, Class_ID(OMNI_LIGHT_CLASS_ID, 0));
		break;

	}

	pLightObj->SetRGBColor(m_time, Point3(light->color));
	pLightObj->SetIntensity(m_time, intensity);

	if (light->type == cgltf_light_type_spot) {
		pLightObj->SetHotspot(m_time, light->spot_inner_cone_angle * (180.0f / PI));
		pLightObj->SetFallsize(m_time, light->spot_outer_cone_angle * (180.0f / PI));
	}

	pLightObj->Enable(1);

	INode *pNode = GetCOREInterface()->CreateObjectNode(pLightObj);
	pNode->SetName(name.c_str());

	m_LightMap.insert(std::make_pair(light, pLightObj));

	return pNode;
}


//======================================================================
//======================================================================
void glTFImporter_Core::SetCamPZnearController(GenCamera* pCamera, Control* pCtrl, cgltf_interpolation_type InterpType, TimeValue start)
{
	pCamera->SetManualClip(TRUE);
	IParamBlock* pBlock = GetParamBlock(pCamera, 0);
	if (!pBlock) return;
	pBlock->SetController(2, pCtrl);
}

//======================================================================
//======================================================================
void glTFImporter_Core::SetCamPZfarController(GenCamera* pCamera, Control* pCtrl, cgltf_interpolation_type InterpType, TimeValue start)
{
	pCamera->SetManualClip(TRUE);
	IParamBlock* pBlock = GetParamBlock(pCamera, 0);
	if (!pBlock) return;
	pBlock->SetController(3, pCtrl);
}

//======================================================================
//======================================================================
void glTFImporter_Core::SetCamPYfovController(GenCamera* pCamera, Control* pCtrl, cgltf_interpolation_type InterpType, TimeValue start)
{
	pCamera->SetFOVControl(pCtrl);
}

//======================================================================
//======================================================================
void glTFImporter_Core::SetCamOYmagController(GenCamera* pCamera, Control* pCtrl, cgltf_interpolation_type InterpType, TimeValue start)
{
}

//======================================================================
//======================================================================
void glTFImporter_Core::SetCamOXmagController(GenCamera* pCamera, Control* pCtrl, cgltf_interpolation_type InterpType, TimeValue start)
{
}

//======================================================================
//======================================================================
void glTFImporter_Core::SetCamOZnearController(GenCamera* pCamera, Control* pCtrl, cgltf_interpolation_type InterpType, TimeValue start)
{
	IParamBlock* pBlock = GetParamBlock(pCamera, 0);
	if (!pBlock) return;
	pBlock->SetController(2, pCtrl);
}

//======================================================================
//======================================================================
void glTFImporter_Core::SetCamOZfarController(GenCamera* pCamera, Control* pCtrl, cgltf_interpolation_type InterpType, TimeValue start)
{
	IParamBlock* pBlock = GetParamBlock(pCamera, 0);
	if (!pBlock) return;
	pBlock->SetController(3, pCtrl);
}



//======================================================================
//======================================================================
void glTFImporter_Core::SetLightIntensController(GenLight* pLight, Control* pCtrl, cgltf_interpolation_type InterpType, TimeValue start)
{
	IParamBlock* pBlock = GetParamBlock(pLight, 0);
	if (!pBlock) return;
	pBlock->SetController(1, pCtrl);
}

//======================================================================
//======================================================================
void glTFImporter_Core::SetLightRangeController(GenLight* pLight, Control* pCtrl, cgltf_interpolation_type InterpType, TimeValue start)
{
	IParamBlock* pBlock = GetParamBlock(pLight, 0);
	if (!pBlock) return;
	pBlock->SetController(18, pCtrl);
}

//======================================================================
//======================================================================
void glTFImporter_Core::SetLightColorController(GenLight* pLight, Control* pCtrl, cgltf_interpolation_type InterpType, TimeValue start)
{
	pLight->SetColorControl(pCtrl);
	return;

	IParamBlock* pBlock = GetParamBlock(pLight, 0);
	if (!pBlock) return;
	pBlock->SetController(0, pCtrl);
}

//======================================================================
//======================================================================
void glTFImporter_Core::SetLightOutAngleController(GenLight* pLight, Control* pCtrl, cgltf_interpolation_type InterpType, TimeValue start)
{
	pLight->SetFalloffControl(pCtrl);
	return;


	IParamBlock* pBlock = GetParamBlock(pLight, 0);
	if (!pBlock) return;
	pBlock->SetController(4, pCtrl);
}

//======================================================================
//======================================================================
void glTFImporter_Core::SetLightInAngleController(GenLight* pLight, Control* pCtrl, cgltf_interpolation_type InterpType, TimeValue start)
{
	pLight->SetHotSpotControl(pCtrl);
	return;

	IParamBlock* pBlock = GetParamBlock(pLight, 0);
	if (!pBlock) return;
	pBlock->SetController(5, pCtrl);
}



