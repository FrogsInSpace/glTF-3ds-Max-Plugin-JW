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
void glTFExporter_Core::CoronaMaterial(MtlBase *pMtl, tinygltf::Material &material)
{
	BOOL animated = FALSE;

	IParamBlock2 *pBlock0 = pMtl->GetParamBlockByID(0);

	Color c;
	pBlock0->GetValue(crn_baseColor, m_time, c, FOREVER);
	material.pbrMetallicRoughness.baseColorFactor[0] = c.r;
	material.pbrMetallicRoughness.baseColorFactor[1] = c.g;
	material.pbrMetallicRoughness.baseColorFactor[2] = c.b;
	material.pbrMetallicRoughness.baseColorFactor[3] = 1.0f;

	Texmap *pTex = NULL;
	pBlock0->GetValue(crn_baseTexmap, m_time, pTex, FOREVER);
	if (pTex) {
		if (pTex->ClassID() == CoronaMixID) {
			IParamBlock2 *p = pTex->GetParamBlock(0);
			p->GetValue(1, m_time, pTex, FOREVER, 0);
		}
		if(pTex) CreateBaseColorTexture(material, pTex);
	}
	else {
		material.pbrMetallicRoughness.baseColorTexture.index = -1;
	}
	//material.has_pbr_metallic_roughness = 1;
	pBlock0->GetValue(crn_baseBumpTexmap, m_time, pTex, FOREVER);
	if (pTex) {
		if (pTex->ClassID() == CoronaNormalMapID) {
			IParamBlock2 *p = pTex->GetParamBlock(0);
			p->GetValue(crn_nrm_normalMap, m_time, pTex, FOREVER);
		}

		if (pTex) CreateNormalTexture(material, pTex);
	}
	else {
		material.normalTexture.index = -1;
	}

	pTex = GetBitmapTextureRec(pBlock0->GetTexmap(crn_selfIllumTexmap));
	if (pTex) {
		CreateEmitTexture(material, pTex);
		Point4 col;
		//pBlock1->GetValue(an_sf_emission_color, m_time, col, FOREVER);
		material.emissiveFactor.push_back(1.0f);
		material.emissiveFactor.push_back(1.0f);
		material.emissiveFactor.push_back(1.0f);
	}
	else {
		material.emissiveTexture.index = -1;
	}
	//pBlock0->GetValue(glTF_ambientOcclusionMap, m_time, pTex, FOREVER);
	pTex = NULL;
	if (pTex) {
		CreateOcclusionTexture(material, pTex);
	}
	else {
		material.occlusionTexture.index = -1;
	}

	pBlock0->GetValue(crn_opacityTexmap, m_time, pTex, FOREVER);
	if (pTex) {
		if (pTex->ClassID() == OSLTex_CLASS_ID) {
			float value;
			IParamBlock2 *pPBlock1 = pTex->GetParamBlock(1);
			pPBlock1->GetValue(0, m_time, value, FOREVER);
			pPBlock1->GetValue(4, m_time, pTex, FOREVER);
			material.alphaMode = "MASK";
			material.alphaCutoff = value;
		}
		else {
			material.alphaMode = "BLEND";
		}
	}
	else {
	}

	{
		TransmissionStruct str;
		pBlock0->GetValue(crn_refractionAmountTexmap, m_time, str.pTex, FOREVER);
		if (str.pTex) {
			if (str.pTex->ClassID() == OSLTex_CLASS_ID) {
				str.pTex->GetParamBlock(1)->GetValueByName(_T("B_map"), m_time, str.pTex, FOREVER);
			}
		}
		pBlock0->GetValue(crn_refractionAmount, m_time, str.factor, FOREVER);
		if (str.pTex || str.factor > 0.0f) {
			CreateTransmissionTexture(material, str, animated);
			material.alphaMode = "BLEND";
		}
	}

	Texmap *pTex1, *pTex2;
	pBlock0->GetValue(crn_baseRoughnessTexmap, m_time, pTex1, FOREVER);
	pBlock0->GetValue(crn_metalnessTexmap, m_time, pTex2, FOREVER);
	if (pTex1 || pTex2) {
		int mapCh1 = 0;
		CreateMetalRoughTexture(material, pTex1, pTex2, &mapCh1);
		material.pbrMetallicRoughness.baseColorTexture.texCoord = mapCh1;
	}
	else {
	}

	{
		float val;
		pBlock0->GetValue(crn_baseRoughness, m_time, val, FOREVER);
		material.pbrMetallicRoughness.roughnessFactor = val;
		if(!pTex2)
			material.pbrMetallicRoughness.metallicFactor = 0.0f;
	}

	int enable;
	pBlock0->GetValue(crn_sheenColorTexmapOn, m_time, enable, FOREVER);
	{
		SheenStruct str;
		pBlock0->GetValue(crn_sheenColorTexmap, m_time, str.pColMap, FOREVER);
		pBlock0->GetValue(crn_sheenRoughnessTexmap, m_time, str.pRoughnessMap, FOREVER);
		pBlock0->GetValue(crn_sheenColor, m_time, str.color, FOREVER);
		pBlock0->GetValue(crn_sheenRoughness, m_time, str.roughness, FOREVER);
		if(str.pColMap|| str.pRoughnessMap)
			CreateSheenTexture(material, str, animated);
	}
	//pBlock0->GetValue(fm_coat_map, m_time, pTex, FOREVER);
	//pBlock0->GetValue(fm_coat_rough_map, m_time, pTex, FOREVER);
	//pBlock0->GetValue(glTF_clearcoatNormalMap, m_time, pTex, FOREVER);
}


