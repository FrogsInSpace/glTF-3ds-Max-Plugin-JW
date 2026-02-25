

#include "HSglTFExporter.h"


//----------------------------------------------------------
//----------------------------------------------------------
void glTFExporter_Core::PhysicalMaterial(MtlBase *pMtl, tinygltf::Material &material)
{
	BOOL animated = FALSE;

	IParamBlock2 *pBlock0 = pMtl->GetParamBlockByID(0);
	//IParamBlock2 *pBlock1 = pMtl->GetParamBlockByID(1);
	Point4 baseColor;
	pBlock0->GetValue(fm_base_color, m_time, baseColor, FOREVER);
	material.pbrMetallicRoughness.baseColorFactor[0] = baseColor.x < 0.0 ? 0.0f : baseColor.x;
	material.pbrMetallicRoughness.baseColorFactor[1] = baseColor.y < 0.0 ? 0.0f : baseColor.y;
	material.pbrMetallicRoughness.baseColorFactor[2] = baseColor.z < 0.0 ? 0.0f : baseColor.z;
	material.pbrMetallicRoughness.baseColorFactor[3] = baseColor.w < 0.0 ? 0.0f : baseColor.w;

	Texmap *pTex = NULL;
	int mapCh = 0;

	Texmap* pAlphaMap = NULL;
	pBlock0->GetValue(fm_base_color_map, m_time, pTex, FOREVER);
	if (pTex) {
		if (pTex->ClassID() == CompositeTexClassID) {
			IParamBlock2 *p = pTex->GetParamBlockByID(0);
			p->GetValue(9, m_time, pTex, FOREVER, 0);
		}
		if (pTex) {
			Texmap *pTemp;
			pBlock0->GetValue(fm_cutout_map, m_time, pTemp, FOREVER);
			if (pTemp){
				if (GetOSLMapType(pTemp) == OSL_CutOff) {
					float val;
					if (GetCutOffValue(pTemp, pAlphaMap, val)) {
						if(pAlphaMap->ClassID() == bmptexClassID)
							pTex = pAlphaMap;
						material.alphaCutoff = truncateDecimal(val);
					}
				}
				material.alphaMode = "MASK";
			}
			// pTex とｐAlphaTexが異なる場合、アルファマップを置き換えたテクスチャを作る
			// ※まだ作ってません
			if(pTex != pAlphaMap && pAlphaMap)
				pTex = ReplaceAlphamap(pTex, pAlphaMap);
			CreateBaseColorTexture(material, pTex);
		}
	}
	else {
		if (baseColor.w < 1.0f)
			material.alphaMode = "BLEND";
		else {
			Control* pC = pBlock0->GetControllerByID(fm_base_color);
			if (pC)
				material.alphaMode = "BLEND";
		}

		material.pbrMetallicRoughness.baseColorTexture.index = -1;
	}
	//material.has_pbr_metallic_roughness = 1;
	material.pbrMetallicRoughness.metallicFactor = 0.0f;
	material.pbrMetallicRoughness.roughnessFactor = 0.0f;
	//material->pbr_metallic_roughness.metallic_roughness_texture = 0;
	pBlock0->GetValue(fm_bump_map, m_time, pTex, FOREVER);
	if (pTex) {
		float scale = 1.0f;
		pBlock0->GetValue(fm_bump_map_amt, m_time, scale, FOREVER);
		if (pTex->ClassID() == NormalBumpMapClassID) {
			IParamBlock2* p = pTex->GetParamBlockByID(0);
			p->GetValue(2, m_time, pTex, FOREVER);
			if (pTex) CreateNormalTexture(material, pTex, scale);
		}
		else {
			MaterialBumpStruct str;
			str.bumpTexture = pTex;
			str.bumpFactor = scale / 10.0f;
			CreateMaterialBumpTexture(material, str, FALSE);
		}
	}
	else {
		material.normalTexture.index = -1;
	}

	pTex = GetBitmapTextureRec(pBlock0->GetTexmap(fm_emit_color_map));
	if (pTex) {
		CreateEmitTexture(material, pTex);
	}
	else {
		material.emissiveTexture.index = -1;
	}
	Point4 col;
	{
		pBlock0->GetValue(fm_emit_color, m_time, col, FOREVER);
		material.emissiveFactor.push_back(col.x);
		material.emissiveFactor.push_back(col.y);
		material.emissiveFactor.push_back(col.z);
	}
	if (pTex || col.x > 0.0f || col.y > 0.0f || col.z > 0.0f) {
		float lum;
		pBlock0->GetValue(fm_emit_luminance, m_time, lum, FOREVER);
		CreateEmitStrength(material, lum / 300.0f);
	}

//pBlock0->GetValue(glTF_ambientOcclusionMap, m_time, pTex, FOREVER);
	pTex = NULL;
	if (pTex) {
		CreateOcclusionTexture(material, pTex);
	}
	else {
		material.occlusionTexture.index = -1;
	}

	{
		TransmissionStruct str;
		pBlock0->GetValue(fm_transparency_map, m_time, str.pTex, FOREVER);
		if (str.pTex) {
			if (str.pTex->ClassID() == OSLTex_CLASS_ID) {
				str.pTex->GetParamBlock(1)->GetValueByName(_T("B_map"), m_time, str.pTex, FOREVER);
			}
			if (str.pTex->ClassID() == ColorCorrectTexID) {
				str.pTex = str.pTex->GetParamBlock(0)->GetTexmap(1);
			}
			if (str.pTex->ClassID() == RGBMultiTexID) {
				Texmap *p = str.pTex->GetParamBlock(0)->GetTexmap(2);
				if (!p)
					p = str.pTex->GetParamBlock(0)->GetTexmap(3);
				str.pTex = p;
			}
		}
		pBlock0->GetValue(fm_transparency, m_time, str.factor, FOREVER);
		//if (str.pTex || str.factor > 0.0f) {
		CreateTransmissionTexture(material, str, FALSE);
			//material.alphaMode = "BLEND";
		//}
	}

	Texmap *pTex1, *pTex2;
	pBlock0->GetValue(fm_roughness_map, m_time, pTex1, FOREVER);
	pBlock0->GetValue(fm_metalness_map, m_time, pTex2, FOREVER);
	if (pTex1 || pTex2) {
		int mapCh1 = 0;
		CreateMetalRoughTexture(material, pTex1, pTex2, &mapCh1);
		material.pbrMetallicRoughness.baseColorTexture.texCoord = mapCh1;
	}
	else {
	}

	{
		float val;
		pBlock0->GetValue(fm_roughness, m_time, val, FOREVER);
		material.pbrMetallicRoughness.roughnessFactor = val;
		pBlock0->GetValue(fm_metalness, m_time, val, FOREVER);
		material.pbrMetallicRoughness.metallicFactor = val;
	}

	{
		IORStruct str;
		pBlock0->GetValue(fm_trans_ior, m_time, str.ior, FOREVER);
		Control* pC = pBlock0->GetControllerByID(fm_trans_ior);
		if (pC) animated = pC->IsAnimated();
		CreateIORTexture(material, str, animated);
	}

	{
		ClearCoatStruct str;
		str.pMap = NULL;
		str.pNormalMap = NULL;
		str.pRoughnessMap = NULL;

		pBlock0->GetValue(fm_coating, m_time, str.factor, FOREVER);
		pBlock0->GetValue(fm_coat_roughness, m_time, str.roughness, FOREVER);
		pBlock0->GetValue(fm_coat_map, m_time, str.pMap, FOREVER);
		pBlock0->GetValue(fm_coat_rough_map, m_time, str.pRoughnessMap, FOREVER);
		//pBlock0->GetValue(glTF_clearcoatNormalMap, m_time, pTex, FOREVER);
		CreateClearCoatTexture(material, str, animated);
	}

	{
		AnisotropyStruct str;
		str.strength = 0.0f;
		str.rotation = 0.0f;
		str.texture = NULL;

		pBlock0->GetValue(fm_anisotropy, m_time, str.strength, FOREVER);
		pBlock0->GetValue(fm_anisoangle, m_time, str.rotation, FOREVER);
		pBlock0->GetValue(fm_anisotropy_map, m_time, str.texture, FOREVER);
		CreateAnisotropyTexture(material, str, animated);
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

#if MAX_RELEASE>=25000
	{
		DispersionStruct str;
		pBlock0->GetValue(fm_dispersion, m_time, str.dispersion, FOREVER);
		Control* pC = pBlock0->GetControllerByID(fm_dispersion);
		if (pC) animated = pC->IsAnimated();
		CreateDispersionTexture(material, str, animated);
	}
	{
		SheenStruct str;
		str.pColMap = NULL;
		str.pRoughnessMap = NULL;
		pBlock0->GetValue(fm_sheen_color, m_time, str.color, FOREVER);
		Control* pC = pBlock0->GetControllerByID(fm_sheen_color);
		pBlock0->GetValue(fm_sheen_roughness, m_time, str.roughness, FOREVER);
		pC = pBlock0->GetControllerByID(fm_sheen_roughness);
		pBlock0->GetValue(fm_sheen_color_map, m_time, str.pColMap, FOREVER);
		pBlock0->GetValue(fm_sheen_rough_map, m_time, str.pRoughnessMap, FOREVER);
		CreateSheenTexture(material, str, animated);
	}
#endif
	material.alphaMode = GetAlphaMode(pMtl);

	{
		UnlitStruct str;
		if (SetUnlitParams(pMtl, str)) {
			CreateUnlitTexture(material, str, animated);
		}
	}
#if MAX_RELEASE>=25000
	{
		IridescenceStruct str;
		SetIridescenceParams(pMtl, str, animated);
			pBlock0->GetValue(fm_thin_film_weight, m_time, str.factor, FOREVER);
			pBlock0->GetValue(fm_thin_film_thickness, m_time, str.maximum, FOREVER);
			pBlock0->GetValue(fm_thin_film_ior, m_time, str.ior, FOREVER);
			pBlock0->GetValue(fm_thin_film_map, m_time, str.texture, FOREVER);
			pBlock0->GetValue(fm_thin_film_ior_map, m_time, str.thicknessTexture, FOREVER);

			if (pBlock0->GetControllerByID(fm_thin_film_weight)) animated = TRUE;
			if (pBlock0->GetControllerByID(fm_thin_film_thickness)) animated = TRUE;
			if (pBlock0->GetControllerByID(fm_thin_film_ior)) animated = TRUE;

			CreateIridescenceTexture(material, str, animated);
	}
#endif
	{
		VolumeStruct str;
		if (SetVolumeParams(pMtl, str, animated)) {
			CreateVolumeTexture(material, str, animated);
		}
	}
}
