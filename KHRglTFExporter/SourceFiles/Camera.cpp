//======================================================================
//======================================================================

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

	if (camera->type == cgltf_camera_type_perspective) {
		cgltf_camera_perspective *perspective = &camera->data.perspective;
		pCamObj->SetOrtho(FALSE);

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
		pCamObj->SetManualClip(TRUE);
		pCamObj->SetClipDist(m_time, CAM_HITHER_CLIP, orthographic->znear);
		pCamObj->SetClipDist(m_time, CAM_YON_CLIP, orthographic->zfar);
	}

	INode *pNode = GetCOREInterface()->CreateObjectNode(pCamObj);
	pNode->SetName(name.c_str()); //StringToWString(name.C_Str()).c_str());

	return pNode;
}

//======================================================================
//======================================================================
INode* glTFImporter_Core::CreateLight(cgltf_node *node)
{
	cgltf_light *light = node->light;

	tstring name;
	const char *pname = node->name;
	if (!pname) name = StringToWString(light->name);

	GenLight  *pLightObj = (GenLight*)GetCOREInterface()->CreateInstance(LIGHT_CLASS_ID, Class_ID(OMNI_LIGHT_CLASS_ID, 0));
	pLightObj->Enable(1);

	INode *pNode = GetCOREInterface()->CreateObjectNode(pLightObj);
	pNode->SetName(name.c_str());

	return pNode;
}