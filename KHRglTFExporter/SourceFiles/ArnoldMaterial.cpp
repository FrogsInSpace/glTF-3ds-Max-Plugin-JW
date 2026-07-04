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

//----------------------------------------------------------
//----------------------------------------------------------
void glTFExporter_Core::ArnoldMaterial(MtlBase *pMtl, tinygltf::Material &material)
{
	BOOL animated = FALSE;

	IParamBlock2 *pBlock0 = pMtl->GetParamBlockByID(0);
	IParamBlock2 *pBlock1 = pMtl->GetParamBlockByID(1);
	IParamBlock2 *pBlock2 = pMtl->GetParamBlockByID(2);

	Color c;
	pBlock1->GetValue(an_sf_base_color, m_time, c, FOREVER);
	material.pbrMetallicRoughness.baseColorFactor[0] = c.r < 0.0f ? 0.0f : c.r;
	material.pbrMetallicRoughness.baseColorFactor[1] = c.g < 0.0f ? 0.0f : c.g;
	material.pbrMetallicRoughness.baseColorFactor[2] = c.b < 0.0f ? 0.0f : c.b;
	material.pbrMetallicRoughness.baseColorFactor[3] = 1.0f;

	Texmap *pTex = NULL;
	Texmap* pBaseColorMap = pBlock1->GetTexmap(an_sf_base_color_shader);
	if (pBaseColorMap) {
		if (pBaseColorMap->ClassID() == CompositeTexClassID) {
			pBaseColorMap = pBaseColorMap->GetParamBlockByID(0)->GetTexmap(9);
		}
		if(pBaseColorMap) CreateBaseColorTexture(material, pBaseColorMap);
	}
	else {
		material.pbrMetallicRoughness.baseColorTexture.index = -1;
	}
	//material.has_pbr_metallic_roughness = 1;
	pBlock1->GetValue(an_sf_normal_shader, m_time, pTex, FOREVER);
	if (pTex) {
		float scale = 1.0f;
		if (pTex->ClassID() == ArnoldNormalMapID) {
			IParamBlock2 *pBlock = pTex->GetParamBlock(1);
			pBlock->GetValue(2, m_time, pTex, FOREVER);
			pBlock->GetValue(15, m_time, scale, FOREVER);
		}
		if (pTex) CreateNormalTexture(material, pTex, scale);
	}
	else {
		material.normalTexture.index = -1;
	}

	pTex = GetBitmapTextureRec(pBlock1->GetTexmap(an_sf_emission_color_shader));
	if (pTex) {
		CreateEmitTexture(material, pTex);
	}
	else {
		material.emissiveTexture.index = -1;
	}
	{
		Color col;
		pBlock1->GetValue(an_sf_emission_color, m_time, col, FOREVER);
		material.emissiveFactor.push_back(col.r);
		material.emissiveFactor.push_back(col.g);
		material.emissiveFactor.push_back(col.b);
	}

	//pBlock0->GetValue(glTF_ambientOcclusionMap, m_time, pTex, FOREVER);
	pTex = NULL;
	if (pTex) {
		CreateOcclusionTexture(material, pTex);
	}
	else {
		material.occlusionTexture.index = -1;
	}

	Texmap* pAlphaMap = NULL;
	pBlock1->GetValue(an_sf_opacity_shader, m_time, pAlphaMap, FOREVER);
	if (pAlphaMap) {
		if (pAlphaMap->ClassID() == ColorCorrectTexID)
			pAlphaMap = pAlphaMap->GetParamBlock(0)->GetTexmap(1);
		TransmissionStruct str;
		str.pTex= pAlphaMap;
		str.factor = 0.5f;
		if (GetOSLMapType(str.pTex) == OSL_CutOff) {
			float val;
			if (GetCutOffValue(str.pTex, pAlphaMap, val)) {
				str.pTex = pAlphaMap;
				material.alphaCutoff = val;
			}
			material.alphaMode = "MASK";
		}
		if(pBaseColorMap != pAlphaMap)
			CreateTransmissionTexture(material, str, animated);
	}

	pBlock1->GetValue(an_sf_transmission_shader, m_time, pTex, FOREVER);
	if (pTex) {
		TransmissionStruct str;
		str.pTex = pTex;
		//pBlock0->GetValue(fm_transparency, m_time, str.factor, FOREVER);
		CreateTransmissionTexture(material, str, animated);
		material.alphaMode = "BLEND";
	}
	Texmap *pTex1, *pTex2;
	pBlock1->GetValue(an_sf_specular_roughness_shader, m_time, pTex1, FOREVER);
	pBlock1->GetValue(an_sf_metalness_shader, m_time, pTex2, FOREVER);
	if (pTex1 || pTex2) {
		int mapCh1 = 0;
		CreateMetalRoughTexture(material, pTex1, pTex2, &mapCh1);
		material.pbrMetallicRoughness.baseColorTexture.texCoord = mapCh1;
	}
	else {
	}

	material.alphaMode = GetAlphaMode(pMtl);

	//pBlock0->GetValue(fm_coat_map, m_time, pTex, FOREVER);
	//pBlock0->GetValue(fm_coat_rough_map, m_time, pTex, FOREVER);
	//pBlock0->GetValue(glTF_clearcoatNormalMap, m_time, pTex, FOREVER);
	//pBlock0->GetValue(fm_sheen_color_map, m_time, pTex, FOREVER);
	//pBlock0->GetValue(fm_sheen_rough_map, m_time, pTex, FOREVER);
}
