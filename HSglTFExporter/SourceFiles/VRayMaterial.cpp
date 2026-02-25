

#include "HSglTFExporter.h"

//----------------------------------------------------------
//----------------------------------------------------------
void glTFExporter_Core::VRayMaterial(MtlBase *pMtl, tinygltf::Material &material)
{
	BOOL animated = FALSE;

	IParamBlock2 *pBlock0 = pMtl->GetParamBlockByID(0);
	IParamBlock2 *pBlock1 = pMtl->GetParamBlockByID(1);
	IParamBlock2 *pBlock2 = pMtl->GetParamBlockByID(2);
	IParamBlock2 *pBlock3 = pMtl->GetParamBlockByID(4);

	Color c;
	pBlock0->GetValue(vr_diffuse, m_time, c, FOREVER);
	material.pbrMetallicRoughness.baseColorFactor[0] = c.r < 0.0 ? 0.0f : c.r;
	material.pbrMetallicRoughness.baseColorFactor[1] = c.g < 0.0 ? 0.0f : c.g;
	material.pbrMetallicRoughness.baseColorFactor[2] = c.b < 0.0 ? 0.0f : c.b;
	material.pbrMetallicRoughness.baseColorFactor[3] = 1.0f;

	Texmap * pBaseColorMap = NULL;
	pBlock1->GetValueByName(vr_texmap_diffuse, m_time, pBaseColorMap, FOREVER);
	if (pBaseColorMap) {
		if (pBaseColorMap->ClassID() == VRayCompTexID) {
			IParamBlock2 *p = pBaseColorMap->GetParamBlock(0);
			p->GetValue(1, m_time, pBaseColorMap, FOREVER, 0);
		}
		if(pBaseColorMap) CreateBaseColorTexture(material, pBaseColorMap);
	}
	else {
		material.pbrMetallicRoughness.baseColorTexture.index = -1;
	}
	//material.has_pbr_metallic_roughness = 1;
	Texmap* pTex = NULL;
	pBlock0->GetValueByName(vr_texmap_bump, m_time, pTex, FOREVER);
	if (pTex) {
		float scale = pMtl->GetParamBlock(0)->GetFloat(vr_texmap_bump_multiplier, m_time);
		IParamBlock2 *p = pTex->GetParamBlockByID(0);
		if (pTex->ClassID() == VRayNormalMapID) {
			p->GetValue(vr_nrm_normal_map, m_time, pTex, FOREVER);
			if (pTex) CreateNormalTexture(material, pTex, scale / 100.0f);
		}
		else {
			MaterialBumpStruct str;
			str.bumpTexture = pTex;
			str.bumpFactor = scale / 100.0f;
			CreateMaterialBumpTexture(material, str, FALSE);
		}
	}
	else {
		material.normalTexture.index = -1;
	}

	Texmap* pAlphaMap = NULL;
	pBlock3->GetValue(vr_texmap_opacity, m_time, pAlphaMap, FOREVER);
	if (pAlphaMap) {
		if (pAlphaMap->ClassID() == ColorCorrectTexID)
			pAlphaMap = pAlphaMap->GetParamBlock(0)->GetTexmap(1);
		else if (pAlphaMap->ClassID() == RGBMultiTexID) {
			Texmap* p = pAlphaMap->GetParamBlock(0)->GetTexmap(2);
			if (!p)
				p = pAlphaMap->GetParamBlock(0)->GetTexmap(3);
			pAlphaMap = p;
		}
		TransmissionStruct str;
		str.pTex = pAlphaMap;
		str.factor = 0.5f;
		if (GetOSLMapType(str.pTex) == OSL_CutOff) {
			float val;
			if (GetCutOffValue(str.pTex, pAlphaMap, val)) {
				str.pTex = pAlphaMap;
				material.alphaCutoff = val;
			}
			material.alphaMode = "MASK";
		}
		if (pBaseColorMap != pAlphaMap)
			CreateTransmissionTexture(material, str, animated);
	}

	pBlock0->GetValueByName(vr_texmap_self_illumination, m_time, pTex, FOREVER);
	pTex = GetBitmapTextureRec(pTex);
	if (pTex) {
		CreateEmitTexture(material, pTex);
	}
	else {
		material.emissiveTexture.index = -1;
	}

	{
		Color col;
		pBlock0->GetValue(vr_selfIllumination, m_time, col, FOREVER);
		material.emissiveFactor.push_back(col.r);
		material.emissiveFactor.push_back(col.g);
		material.emissiveFactor.push_back(col.b);
	}
	{
		EmissiveStrengthStruct str;
		str.strength = pBlock0->GetFloat(vr_selfIllumination_multiplier);
		CreateEmissiveStrengthTexture(material, str, FALSE);
	}

	//pBlock0->GetValue(glTF_ambientOcclusionMap, m_time, pTex, FOREVER);
	pTex = NULL;
	if (pTex) {
		CreateOcclusionTexture(material, pTex);
	}
	else {
		material.occlusionTexture.index = -1;
	}

	pBlock0->GetValueByName(vr_texmap_refraction, m_time, pTex, FOREVER);
	if (pTex) {
		CreateOpacityTexture(material, pTex);
		material.alphaMode = "MASK";
	}
	else {
	}

	Texmap *pTex1, *pTex2;
	pBlock0->GetValueByName(vr_texmap_reflectionGlossiness, m_time, pTex1, FOREVER);
	pBlock0->GetValueByName(vr_texmap_metalness, m_time, pTex2, FOREVER);
	if (pTex1 || pTex2) {
		int mapCh1 = 0;
		CreateMetalRoughTexture(material, pTex1, pTex2, &mapCh1);
		material.pbrMetallicRoughness.baseColorTexture.texCoord = mapCh1;
	}
	else {
	}

	{
		TransmissionStruct str;
		pBlock0->GetValueByName(vr_texmap_refraction, m_time, str.pTex, FOREVER);
		Color c;
		pBlock1->GetValue(vr_refraction, m_time, c, FOREVER);
		str.factor = c.r;
		if (str.pTex || str.factor > 0.0f) {
			CreateTransmissionTexture(material, str, animated);
			material.alphaMode = "BLEND";
		}
	}

	{
		float val;
		pBlock0->GetValue(vr_reflection_metalness, m_time, val, FOREVER);
		material.pbrMetallicRoughness.metallicFactor = val;// / 100.0f;
		//pBlock0->GetValue(vr_diffuse_roughness, m_time, val, FOREVER);
		//val = pBlock3->GetFloat(vr_texmap_reflectionGlossiness_multiplier);
		//material.pbrMetallicRoughness.roughnessFactor = val/100.0f;
	}

	if (pBlock3->GetInt(vr_texmap_anisotropy_on)) {
		AnisotropyStruct str;
		str.rotation = 0.0f;
		float f = pBlock3->GetFloat(vr_texmap_anisotropy_multiplier);
		str.strength = f / 75.0f;
		Texmap* pTex = pBlock3->GetTexmap(vr_texmap_anisotropy);
		str.texture = GetBitmapTextureRec(pTex);
		CreateAnisotropyTexture(material, str, FALSE);
	}

	{
		TransmissionStruct str;
		Color c;
		pBlock0->GetValue(vr_refraction, m_time, c, FOREVER);
		str.factor = 0.299f * c.r + 0.587f * c.g + 0.114f * c.b;
		pBlock0->GetValueByName(vr_texmap_refraction, m_time, str.pTex, FOREVER);
		if (str.pTex || str.factor > 0.0f) {
			CreateTransmissionTexture(material, str, animated);
			material.alphaMode = "BLEND";
		}
		else if (pBlock0->GetInt(vr_translucency_on, m_time, FOREVER) == 5) {
			CreateTransmissionTexture(material, str, animated);
			material.alphaMode = "BLEND";
		}
	}

	if (pBlock0->GetInt(vr_translucency_on, m_time, FOREVER)==5) {
		VolumeStruct str;
		pBlock0->GetValue(vr_translucency_thickness, m_time, str.thickness, FOREVER);
		pBlock0->GetValue(vr_translucency_fbCoeff, m_time, str.distance, FOREVER);
		pBlock0->GetValue(vr_translucency_color, m_time, str.color, FOREVER);
		str.pThicknessMap = pBlock3->GetTexmap(vr_texmap_translucent);

		CreateVolumeTexture(material, str, TRUE);
	}

//	int enable;
//	pBlock3->GetValueByName(vr_texmap_sheen_on, m_time, enable, FOREVER);
	{
		SheenStruct str;
		pBlock3->GetValueByName(vr_texmap_sheen, m_time, str.pColMap, FOREVER);
		pBlock3->GetValueByName(vr_texmap_sheen_glossiness, m_time, str.pRoughnessMap, FOREVER);
		pBlock1->GetValueByName(vr_sheen_color, m_time, str.color, FOREVER);
		pBlock1->GetValueByName(vr_sheen_glossiness, m_time, str.roughness, FOREVER);
		if(str.pColMap || str.pRoughnessMap || str.roughness || (str.color!=Color(0.0f, 0.0f, 0.0f)))
			CreateSheenTexture(material, str, animated);
	}

	{
		SpecularStruct str;
		if (SetSpecularParams(pMtl, str, animated)) {
			CreateSpecularTexture(material, str, animated);
		}
	}

	{
		DiffuseTransmissionStruct str;
		if (SetDiffuseTransmissionParams(pMtl, str, animated)) {
			CreateDiffuseTransmissionTexture(material, str, animated);
		}
	}

	{
		IORStruct str;
		pBlock0->GetValue(vr_refraction_ior, m_time, str.ior, FOREVER);
		BOOL animated = FALSE;
		Control* pC = pBlock0->GetControllerByID(vr_refraction_ior);
		if (pC) animated = pC->IsAnimated();
		CreateIORTexture(material, str, animated);
	}

	if(pBlock1->GetInt(vr_thinfilm_on)){
		IridescenceStruct str;
		str.factor = 1.0;
		str.minimum = pBlock1->GetFloat(vr_thinfilm_thickness_min);
		str.maximum = pBlock1->GetFloat(vr_thinfilm_thickness_max);
		str.ior = pBlock1->GetFloat(vr_thinfilm_ior);
		str.texture = pBlock0->GetTexmap(vr_texmap_thinfilm_ior);
		str.thicknessTexture = pBlock0->GetTexmap(vr_texmap_thinFilm_thickness);
		CreateIridescenceTexture(material, str, animated);
	}

	int doubleSide;
	pMtl->GetParamBlockByID(3)->GetValue(vr_option_doubleSided, m_time, doubleSide, FOREVER);
	material.doubleSided = doubleSide;

	{
		ClearCoatStruct str;
		str.factor = pBlock0->GetFloat(vr_coat_amount);
		if (str.factor > 0.0f) {
			str.roughness = pBlock0->GetFloat(vr_coat_glossiness);
			str.normalValue = pBlock0->GetFloat(vr_coat_glossiness);
			str.pMap = pBlock3->GetTexmap(vr_texmap_coat_amount);
			str.pRoughnessMap = pBlock3->GetTexmap(vr_texmap_coat_glossiness);
			str.pNormalMap = pBlock3->GetTexmap(vr_texmap_coat_bump);
			CreateClearCoatTexture(material, str, FALSE);
		}
	}

	{
		vrayExtStruct str;
		SetVRayExtParams(pMtl, str);
		material.pbrMetallicRoughness.roughnessFactor = str.roughness;
	}

	//pBlock0->GetValue(fm_coat_map, m_time, pTex, FOREVER);
	//pBlock0->GetValue(fm_coat_rough_map, m_time, pTex, FOREVER);
	//pBlock0->GetValue(glTF_clearcoatNormalMap, m_time, pTex, FOREVER);

	material.alphaMode = GetAlphaMode(pMtl);
}

