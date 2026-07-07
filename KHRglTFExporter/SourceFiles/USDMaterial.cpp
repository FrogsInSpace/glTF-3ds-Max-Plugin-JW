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


#include "KHRglTFExporter.h"

//----------------------------------------------------------
//----------------------------------------------------------
void glTFExporter_Core::USDMaterial(MtlBase *pMtl, tinygltf::Material &material)
{
	IParamBlock2 *pBlock0 = pMtl->GetParamBlockByID(0);
	IParamBlock2 *pBlock1 = pMtl->GetParamBlockByID(1);
	Point4 c;
	pBlock1->GetValue(usd_diffuseColor, m_time, c, FOREVER);
	material.pbrMetallicRoughness.baseColorFactor[0] = c.x < 0.0 ? 0.0f : c.x;
	material.pbrMetallicRoughness.baseColorFactor[1] = c.y < 0.0 ? 0.0f : c.y;
	material.pbrMetallicRoughness.baseColorFactor[2] = c.z < 0.0 ? 0.0f : c.z;
	material.pbrMetallicRoughness.baseColorFactor[3] = c.w < 0.0 ? 0.0f : c.w;

	Texmap *pTex = NULL;
	pBlock1->GetValue(usd_diffuseColor_map, m_time, pTex, FOREVER);
	if (pTex) {
		CreateBaseColorTexture(material, pTex);
	}
	else {
		material.pbrMetallicRoughness.baseColorTexture.index = -1;
		//material.pbrMetallicRoughness.baseColorTexture.texCoord = 0;
	}
	{
		float val;
		pBlock1->GetValue(usd_metallic, m_time, val, FOREVER);
		material.pbrMetallicRoughness.metallicFactor = val;
		pBlock1->GetValue(usd_roughness, m_time, val, FOREVER);
		material.pbrMetallicRoughness.roughnessFactor = val;
	}
	//material->pbr_metallic_roughness.metallic_roughness_texture = 0;
	float scale = 1.0f;
	pBlock1->GetValue(usd_normal, m_time, scale, FOREVER);
	pBlock1->GetValue(usd_normal_map, m_time, pTex, FOREVER);
	if (pTex) {
		CreateNormalTexture(material, pTex, scale);
	}
	else {
		material.normalTexture.index = -1;
	}

	pTex = GetBitmapTextureRec(pBlock1->GetTexmap(usd_emissiveColor_map));
	if (pTex) {
		CreateEmitTexture(material, pTex);
	}
	else {
		material.emissiveTexture.index = -1;
		//material.emissiveTexture.texCoord = 0;
	}
	Point3 col;
	pBlock1->GetValue(usd_emissiveColor, m_time, col, FOREVER);
	if (col.x > 0.0f && col.y > 0.0f && col.z > 0.0f) {
		material.emissiveFactor.push_back(col.x);
		material.emissiveFactor.push_back(col.y);
		material.emissiveFactor.push_back(col.z);
	}
	/*
	pBlock1->GetValue(pbr_ao_map, m_time, pTex, FOREVER);
	if (pTex) {
		CreateOcclusionTexture(material, pTex);
	}
	else {
		material.occlusionTexture.index = -1;
	}
	*/
	pBlock1->GetValue(usd_opacity_map, m_time, pTex, FOREVER);
	if (pTex) {
		float f;
		pBlock1->GetValue(usd_opacityThreshold, m_time, f, FOREVER);
		material.alphaCutoff = f;
		material.alphaMode = GetAlphaMode(pMtl);
	}

	material.alphaMode = GetAlphaMode(pMtl);

	Texmap *pTex1, *pTex2, *pTex3;
	pBlock1->GetValue(usd_roughness_map, m_time, pTex1, FOREVER);
	pBlock1->GetValue(usd_metallic_map, m_time, pTex2, FOREVER);
	pBlock1->GetValue(usd_occlusion_map, m_time, pTex3, FOREVER);
	if (pTex1 || pTex2 || pTex3) {
		int mapCh1 = 0;
		int mapCh2 = 1;
		CreateMetalRoughTexture(material, pTex1, pTex2, &mapCh1, pTex3, &mapCh2);
		if (pTex1 || pTex2)
			material.pbrMetallicRoughness.baseColorTexture.texCoord = mapCh1;
		if (pTex3)
			material.occlusionTexture.texCoord = mapCh2;
	}
	else {
	}
}