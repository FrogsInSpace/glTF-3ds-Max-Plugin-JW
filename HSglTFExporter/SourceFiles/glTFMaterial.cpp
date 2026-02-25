

#include "HSglTFExporter.h"

//----------------------------------------------------------
//----------------------------------------------------------
void glTFExporter_Core::glTFMaterial(MtlBase* pMtl, tinygltf::Material& material)
{
	BOOL animated = FALSE;

	IParamBlock2* pBlock0 = pMtl->GetParamBlockByID(0);
	IParamBlock2* pBlock1 = pMtl->GetParamBlockByID(1);
	Point4 c;
	pBlock0->GetValue(glTF_baseColor, m_time, c, FOREVER);
	Control* pC = pBlock0->GetControllerByID(glTF_baseColor);
	if (pC && c == Point4(1.0f, 1.0f, 1.0f, 1.0f)) c.w-=0.00000001f;

	material.pbrMetallicRoughness.baseColorFactor[0] = c.x < 0.0 ? 0.0f : c.x;
	material.pbrMetallicRoughness.baseColorFactor[1] = c.y < 0.0 ? 0.0f : c.y;
	material.pbrMetallicRoughness.baseColorFactor[2] = c.z < 0.0 ? 0.0f : c.z;
	material.pbrMetallicRoughness.baseColorFactor[3] = c.w < 0.0 ? 0.0f : c.w;

	Texmap* pTex = NULL;
	pBlock0->GetValue(glTF_baseColorMap, m_time, pTex, FOREVER);
	if (pTex) {
		CreateBaseColorTexture(material, pTex);
	}
	else {
		material.pbrMetallicRoughness.baseColorTexture.index = -1;
	}
	//material.has_pbr_metallic_roughness = 1;
	material.pbrMetallicRoughness.metallicFactor = 0.0f;
	material.pbrMetallicRoughness.roughnessFactor = 0.0f;
	//material->pbr_metallic_roughness.metallic_roughness_texture = 0;
	float scale = 1.0f;
	pBlock0->GetValue(glTF_normalMap, m_time, pTex, FOREVER);
	pBlock0->GetValue(glTF_normal, m_time, scale, FOREVER);
	if (pTex) {
		if (pTex->ClassID() == OSLTex_CLASS_ID) {
			IParamBlock2* pPBlock1 = pTex->GetParamBlock(1);
			pPBlock1->GetValue(6, m_time, pTex, FOREVER);
		}
		CreateNormalTexture(material, pTex, scale);
	}
	else {
		material.normalTexture.index = -1;
	}
	pBlock0->GetValue(glTF_emissionMap, m_time, pTex, FOREVER);
	if (pTex) {
		CreateEmitTexture(material, pTex);
	}
	else {
		material.emissiveTexture.index = -1;
	}

	Point4 col;
	pBlock0->GetValue(glTF_emissionColor, m_time, col, FOREVER);
	if (col.x != 0.0f || col.y != 0.0f || col.z != 0.0f) {
		material.emissiveFactor.push_back(col.x);
		material.emissiveFactor.push_back(col.y);
		material.emissiveFactor.push_back(col.z);
	}

	int alphaMode;
	pBlock0->GetValue(glTF_alphaMode, m_time, alphaMode, FOREVER);
	switch (alphaMode) {
	case 1:material.alphaMode = "OPAQUE"; break;
	case 2:material.alphaMode = "MASK"; break;
	case 3:material.alphaMode = "BLEND"; break;
	}

	float alphaCutOff;
	pBlock0->GetValue(glTF_alphaCutoff, m_time, alphaCutOff, FOREVER);
	material.alphaCutoff = truncateDecimal(alphaCutOff);

	pBlock1->GetValue(glTF_transmissionMap, m_time, pTex, FOREVER);
	if (pTex) {
		//CreateOpacityTexture(material, pTex);
		material.alphaMode = "MASK";
	}
	else {
	}

	{
		float val;
		pBlock0->GetValue(glTF_roughness, m_time, val, FOREVER);
		material.pbrMetallicRoughness.roughnessFactor = truncateDecimal(val);
		pBlock0->GetValue(glTF_metalness, m_time, val, FOREVER);
		material.pbrMetallicRoughness.metallicFactor = truncateDecimal(val);
	}

	float occStrength = pBlock0->GetFloat(glTF_ambientOcclusion);
	Texmap *pTex1, *pTex2, *pTex3;
	pBlock0->GetValue(glTF_roughnessMap, m_time, pTex1, FOREVER);
	pBlock0->GetValue(glTF_metalnessMap, m_time, pTex2, FOREVER);
	pBlock0->GetValue(glTF_ambientOcclusionMap, m_time, pTex3, FOREVER);
	if (pTex1 || pTex2 || pTex3) {
		int mapCh1 = 0;
		int mapCh2 = 1;
		CreateMetalRoughTexture(material, pTex1, pTex2, &mapCh1, pTex3, &mapCh2);
		if (pTex1 || pTex2)
			material.pbrMetallicRoughness.baseColorTexture.texCoord = mapCh1;
		if (pTex3) {
			material.occlusionTexture.texCoord = mapCh2;
			CreateOcclusionTexture(material, pTex3, occStrength);
		}
	}
	else {
	}
	/*
	if (pTex3) {
		//CreateOcclusionTexture(material, pTex);
	}
	else {
		material.occlusionTexture.index = -1;
	}
	*/
	int enable;
	pBlock1->GetValue(glTF_enableTransmission, m_time, enable, FOREVER);
	if (enable) {
		TransmissionStruct str;
		pBlock1->GetValue(glTF_transmissionMap, m_time, pTex, FOREVER);
		str.pTex = pTex;
		pBlock1->GetValue(glTF_transmission, m_time, str.factor, FOREVER);
		Control* pC = NULL;
		BOOL animated = FALSE;
		pC = pBlock1->GetControllerByIndex(glTF_transmission);
		if (pC) if (pC->IsAnimated()) animated = TRUE;
		CreateTransmissionTexture(material, str, animated);
	}

	pBlock1->GetValue(glTF_enableSpecular, m_time, enable, FOREVER);
	if (enable) {
		SpecularStruct str;
		pBlock1->GetValue(glTF_specularMap, m_time, str.pMap, FOREVER);
		pBlock1->GetValue(glTF_specularColorMap, m_time, str.pColMap, FOREVER);
		pBlock1->GetValue(glTF_specular, m_time, str.factor, FOREVER);
		pBlock1->GetValue(glTF_specularcolor, m_time, str.color, FOREVER);
		Control* pC = NULL;
		BOOL animated = FALSE;
		pC = pBlock1->GetControllerByIndex(glTF_specular);
		if (pC) if (pC->IsAnimated()) animated = TRUE;
		pC = pBlock1->GetControllerByIndex(glTF_specularcolor);
		if (pC) if (pC->IsAnimated()) animated = TRUE;
		CreateSpecularTexture(material, str, animated);
	}

	pBlock1->GetValue(glTF_enableClearcoat, m_time, enable, FOREVER);
	if (enable) {
		ClearCoatStruct str;
		pBlock1->GetValue(glTF_clearcoatMap, m_time, str.pMap, FOREVER);
		pBlock1->GetValue(glTF_clearcoatRoughnessMap, m_time, str.pRoughnessMap, FOREVER);
		pBlock1->GetValue(glTF_clearcoatNormalMap, m_time, str.pNormalMap, FOREVER);
		pBlock1->GetValue(glTF_clearcoat, m_time, str.factor, FOREVER);
		pBlock1->GetValue(glTF_clearcoatRoughness, m_time, str.roughness, FOREVER);
		pBlock1->GetValue(glTF_clearcoatNormal, m_time, str.normalValue, FOREVER);
		Control* pC = NULL;
		BOOL animated = FALSE;
		pC = pBlock1->GetControllerByIndex(glTF_clearcoat);
		if (pC) if (pC->IsAnimated()) animated = TRUE;
		pC = pBlock1->GetControllerByIndex(glTF_clearcoatRoughness);
		if (pC) if (pC->IsAnimated()) animated = TRUE;
		CreateClearCoatTexture(material, str, animated);
	}

	pBlock1->GetValue(glTF_enableSheen, m_time, enable, FOREVER);
	if (enable) {
		SheenStruct str;
		pBlock1->GetValue(glTF_sheenColorMap, m_time, str.pColMap, FOREVER);
		pBlock1->GetValue(glTF_sheenRoughnessMap, m_time, str.pRoughnessMap, FOREVER);
		pBlock1->GetValue(glTF_sheenColor, m_time, str.color, FOREVER);
		pBlock1->GetValue(glTF_sheenRoughness, m_time, str.roughness, FOREVER);
		Control* pC = NULL;
		BOOL animated = FALSE;
		pC = pBlock1->GetControllerByIndex(glTF_sheenColor);
		if (pC) if (pC->IsAnimated()) animated = TRUE;
		pC = pBlock1->GetControllerByIndex(glTF_sheenRoughness);
		if (pC) if (pC->IsAnimated()) animated = TRUE;
		CreateSheenTexture(material, str, animated);
	}

	pBlock1->GetValue(glTF_enableVolume, m_time, enable, FOREVER);
	if (enable) {
		VolumeStruct str;
		pBlock1->GetValue(glTF_volumeThicknessMap, m_time, str.pThicknessMap, FOREVER);
		pBlock1->GetValue(glTF_volumeThickness, m_time, str.thickness, FOREVER);
		pBlock1->GetValue(glTF_volumeDistance, m_time, str.distance, FOREVER);
		pBlock1->GetValue(glTF_volumeColor, m_time, str.color, FOREVER);
		Control* pC = NULL;
		BOOL animated = FALSE;
		pC = pBlock1->GetControllerByIndex(glTF_volumeThickness);
		if (pC) if (pC->IsAnimated()) animated = TRUE;
		pC = pBlock1->GetControllerByIndex(glTF_volumeDistance);
		if (pC) if (pC->IsAnimated()) animated = TRUE;
		pC = pBlock1->GetControllerByIndex(glTF_volumeColor);
		if (pC) if (pC->IsAnimated()) animated = TRUE;
		CreateVolumeTexture(material, str, animated);
	}

	pBlock1->GetValue(glTF_enableIndexOfRefraction, m_time, enable, FOREVER);
	if (enable) {
		IORStruct str;
		pBlock1->GetValue(glTF_indexOfRefraction, m_time, str.ior, FOREVER);
		BOOL animated = FALSE;
		Control* pC = pBlock1->GetControllerByID(glTF_indexOfRefraction);
		if (pC) animated = pC->IsAnimated();
		CreateIORTexture(material, str, animated);
	}

	pBlock1->GetValue(glTF_unlit, m_time, enable, FOREVER);
	if (enable) {
		UnlitStruct str;
		CreateUnlitTexture(material, str, FALSE);
	}

	pBlock1->GetValue(glTF_enableIndexOfRefraction, m_time, enable, FOREVER);
	if (enable) {
		IORStruct str;
		pBlock1->GetValue(glTF_indexOfRefraction, m_time, str.ior, FOREVER);
		BOOL animated = FALSE;
		Control* pC = pBlock0->GetControllerByID(glTF_indexOfRefraction);
		if (pC) animated = pC->IsAnimated(); else animated = FALSE;
		CreateIORTexture(material, str, animated);
	}

	int doubleSide;
	pBlock0->GetValue(glTF_Doublesided, m_time, doubleSide, FOREVER);
	material.doubleSided = doubleSide;

	{
		IridescenceStruct str;
		if (SetIridescenceParams(pMtl, str, animated)) {
			CreateIridescenceTexture(material, str, animated);
		}
	}

	{
		EmissiveStrengthStruct str;
		if (SetEmissiveStrengthParams(pMtl, str, animated)) {
			CreateEmissiveStrengthTexture(material, str, animated);
		}
	}

	{
		DispersionStruct str;
		if (SetDispersionParams(pMtl, str, animated)) {
			CreateDispersionTexture(material, str, animated);
		}
	}

	{
		AnisotropyStruct str;
		if (SetAnisotropyParams(pMtl, str, animated)) {
			CreateAnisotropyTexture(material, str, animated);
		}
	}

	{
		DiffuseTransmissionStruct str;
		if (SetDiffuseTransmissionParams(pMtl, str, animated)) {
			CreateDiffuseTransmissionTexture(material, str, animated);
		}
	}
}
