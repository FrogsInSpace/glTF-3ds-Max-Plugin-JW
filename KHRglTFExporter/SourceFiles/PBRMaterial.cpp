

#include "HSglTFExporter.h"

//----------------------------------------------------------
//----------------------------------------------------------
void glTFExporter_Core::PBRMaterial(MtlBase *pMtl, tinygltf::Material &material)
{
	BOOL animated = FALSE;

	IParamBlock2 *pBlock0 = pMtl->GetParamBlockByID(0);
	IParamBlock2 *pBlock1 = pMtl->GetParamBlockByID(1);
	Point4 baseColor;
	pBlock1->GetValue(pbr_base_color, m_time, baseColor, FOREVER);
	material.pbrMetallicRoughness.baseColorFactor[0] = baseColor.x;
	material.pbrMetallicRoughness.baseColorFactor[1] = baseColor.y;
	material.pbrMetallicRoughness.baseColorFactor[2] = baseColor.z;
	material.pbrMetallicRoughness.baseColorFactor[3] = baseColor.w;

	Texmap *pTex = NULL;
	pBlock1->GetValue(pbr_base_color_map, m_time, pTex, FOREVER);
	if (pTex) {
		CreateBaseColorTexture(material, pTex);
	}
	else {
		material.pbrMetallicRoughness.baseColorTexture.index = -1;
		//material.pbrMetallicRoughness.baseColorTexture.texCoord = 0;
	}
	{
		float val;
		pBlock1->GetValue(pbr_metalness, m_time, val, FOREVER);
		material.pbrMetallicRoughness.metallicFactor = truncateDecimal(val);
		pBlock1->GetValue(pbr_roughness, m_time, val, FOREVER);
		material.pbrMetallicRoughness.roughnessFactor = truncateDecimal(val);
	}
	//material->pbr_metallic_roughness.metallic_roughness_texture = 0;
	float scale = 1.0f;
	pBlock1->GetValue(pbr_bump_map_amt, m_time, scale, FOREVER);
	pBlock1->GetValue(pbr_norm_map, m_time, pTex, FOREVER);
	if (pTex) {
		CreateNormalTexture(material, pTex, scale);
	}
	else {
		material.normalTexture.index = -1;
	}

	pTex = GetBitmapTextureRec(pBlock1->GetTexmap(pbr_emit_color_map));
	if (pTex) {
		CreateEmitTexture(material, pTex);
	}
	else {
		material.emissiveTexture.index = -1;
		//material.emissiveTexture.texCoord = 0;
	}
	Point4 col;
	pBlock1->GetValue(pbr_emit_color, m_time, col, FOREVER);
	if(col.x>0.0f && col.y > 0.0f && col.z > 0.0f){
		material.emissiveFactor.push_back(col.x);
		material.emissiveFactor.push_back(col.y);
		material.emissiveFactor.push_back(col.z);
	}

	pBlock1->GetValue(pbr_ao_map, m_time, pTex, FOREVER);
	if (pTex) {
		CreateOcclusionTexture(material, pTex);
	}
	else {
		material.occlusionTexture.index = -1;
	}

	pBlock1->GetValue(pbr_opacity_map, m_time, pTex, FOREVER);
	if (pTex) {
		if (GetOSLMapType(pTex) == OSL_CutOff) {
			IParamBlock2* pPBlock1 = pTex->GetParamBlock(1);
			float cutoff = 1.0f;
			pPBlock1->GetValue(0, m_time, cutoff, FOREVER);
			material.alphaMode = "MASK";
			material.alphaCutoff = truncateDecimal(cutoff);
		}
		else {
			material.alphaMode = "BLEND";
		}
		//CreateOpacityTexture(material, pTex);
	}
	else {
		if(baseColor.w<1.0f)
			material.alphaMode = "BLEND";
		else {
			Control *pC = pBlock1->GetControllerByID(pbr_base_color);
			if(pC)
				material.alphaMode = "BLEND";
		}
	}
	Texmap *pTex1, *pTex2, *pTex3;
	pBlock1->GetValue(pbr_roughness_map, m_time, pTex1, FOREVER);
	pBlock1->GetValue(pbr_metalness_map, m_time, pTex2, FOREVER);
	pBlock1->GetValue(pbr_ao_map, m_time, pTex3, FOREVER);
	if (pTex1 || pTex2 || pTex3) {
		int mapCh1 = 0;
		int mapCh2 = 1;
		CreateMetalRoughTexture(material, pTex1, pTex2, &mapCh1, pTex3, &mapCh2);
		if (pTex1 || pTex2)
			material.pbrMetallicRoughness.baseColorTexture.texCoord = mapCh1;
		if(pTex3)
			material.occlusionTexture.texCoord = mapCh2;
	}
	else {
	}

	material.alphaMode = GetAlphaMode(pMtl);

	{
		IORStruct str;
		if (SetIORParams(pMtl, str, animated)) {
			CreateIORTexture(material, str, animated);
		}
	}

	{
		VolumeStruct str;
		if (SetVolumeParams(pMtl, str, animated)) {
			CreateVolumeTexture(material, str, animated);
		}
	}

	{
		TransmissionStruct str;
		if (SetTransmissionParams(pMtl, str, animated)) {
			CreateTransmissionTexture(material, str, animated);
		}
	}

	{
		UnlitStruct str;
		if (SetUnlitParams(pMtl, str)) {
			CreateUnlitTexture(material, str, animated);
		}
	}

	{
		IridescenceStruct str;
		if (SetIridescenceParams(pMtl, str, animated)) {
			CreateIridescenceTexture(material, str, animated);
		}
	}

	{
		ClearCoatStruct str;
		if (SetClearCoatParams(pMtl, str, animated)) {
			CreateClearCoatTexture(material, str, animated);
		}
	}

	{
		SheenStruct str;
		if (SetSheenParams(pMtl, str, animated)) {
			CreateSheenTexture(material, str, animated);
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
}