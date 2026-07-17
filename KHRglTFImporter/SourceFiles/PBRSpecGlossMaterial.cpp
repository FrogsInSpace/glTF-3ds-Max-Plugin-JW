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
#include "define.h"

Mtl* glTFImporter_Core::CreatePBRSpecGlossMtl(cgltf_material* mtl)
{
	Mtl* pSmat = (Mtl*)GetCOREInterface()->CreateInstance(MATERIAL_CLASS_ID, PBRSpecGlossMtlID);
	pSmat->SetName(StringToWString(mtl->name).c_str());
	IParamBlock2* pBlock0 = pSmat->GetParamBlockByID(0);
	IParamBlock2* pBlock1 = pSmat->GetParamBlockByID(1);
	BitmapTex* pBmpTex = NULL;

//	if (mtl->has_pbr_specular_glossiness) {

	cgltf_pbr_specular_glossiness* spl_gls = &mtl->pbr_specular_glossiness;
	cgltf_texture_view* diffuseTexInfo = &spl_gls->diffuse_texture;
	if (diffuseTexInfo->texture) {
		pBmpTex = GetBitmapTexFromglTexture(diffuseTexInfo->texture);
		SetTextureUVoffset(pBmpTex, diffuseTexInfo);
		pBlock1->SetValue(pbr_sg_base_color_map, m_time, pBmpTex);
	}
	float* colDiff = spl_gls->diffuse_factor;
	if (colDiff) {
		Color c(colDiff[0], colDiff[1], colDiff[2]);
		pBlock1->SetValue(pbr_sg_base_color, m_time, c);
	}

	if (spl_gls->diffuse_texture.texture) {
		pBmpTex = GetBitmapTexFromglTexture(spl_gls->diffuse_texture.texture);
		SetTextureUVoffset(pBmpTex, &spl_gls->diffuse_texture);
		pBlock1->SetValue(pbr_base_color_map, m_time, pBmpTex);

		if (mtl->alpha_mode == cgltf_alpha_mode_mask || mtl->alpha_mode == cgltf_alpha_mode_blend) {
/*
			BitmapTex* pAlphaBmp = NewDefaultBitmapTex();
			pAlphaBmp->SetName(pBmpTex->GetName() + TSTR(_T("_Alpha")));
			pAlphaBmp->GetUVGen()->SetCoordMapping(UVMAP_SCREEN_ENV);
			pAlphaBmp->GetUVGen()->SetTextureTiling(U_WRAP | V_WRAP);
			pAlphaBmp->GetUVGen()->InitSlotType(MAPSLOT_TEXTURE);
			pAlphaBmp->SetMapName(pBmpTex->GetMapName());
			pAlphaBmp->SetMtlFlag(MTL_TEX_DISPLAY_ENABLED, TRUE);
			pAlphaBmp->SetAlphaAsMono(TRUE);
			pAlphaBmp->SetAlphaAsRGB(FALSE);
			((IParamBlock2*)pAlphaBmp->GetReference(1))->SetValue(10, 0, 1);
*/
			Texmap* pAlphaBmp = CreateAlphaFilterMap(pBmpTex);

			if (mtl->alpha_mode == cgltf_alpha_mode_mask) {
				Texmap* pCCTex = CreateAlphaFilterMap(NULL, AColor(colDiff));
				IParamBlock2* pBlock = pCCTex->GetParamBlock(0);

				Texmap* pOSLMap = CreateCutOffOSLNode(pAlphaBmp, mtl->alpha_cutoff, pCCTex);
				if (pOSLMap)
					pBlock1->SetValue(pbr_opacity_map, m_time, pOSLMap);

				Control* pClr = (Control*)GetCOREInterface()->CreateInstance(CTRL_POINT4_CLASS_ID, Class_ID(0x2013, 0x0));
				pBlock1->SetControllerByID(pbr_sg_base_color, 0, pClr);
				pBlock->SetControllerByID(0, 0, pClr);
			}
			else {
				pBlock1->SetValue(pbr_opacity_map, m_time, pAlphaBmp);
			}
			//pBlock1->SetValue(pbr_opacity_map, m_time, pAlphaBmp);
		}
	}

	cgltf_texture_view* spglTexInfo = &spl_gls->specular_glossiness_texture;
	if (spglTexInfo->texture) {
		pBmpTex = GetBitmapTexFromglTexture(spglTexInfo->texture);
		SetTextureUVoffset(pBmpTex, diffuseTexInfo);
		CorrectBitmapGamma(pBmpTex, 1.0f);
		pBlock1->SetValue(pbr_sg_specular_map, m_time, pBmpTex);
		int aaa = pBmpTex->GetAlphaSource();
		if (pBmpTex->GetAlphaSource() == ALPHA_FILE) {
			BitmapTex* pAlphaBmp = NewDefaultBitmapTex();
			pAlphaBmp->SetName(pBmpTex->GetName() + TSTR(_T("_Roughness")));
			pAlphaBmp->GetUVGen()->SetCoordMapping(UVMAP_SCREEN_ENV);
			pAlphaBmp->GetUVGen()->SetTextureTiling(U_WRAP | V_WRAP);
			pAlphaBmp->GetUVGen()->InitSlotType(MAPSLOT_TEXTURE);
			pAlphaBmp->SetMapName(pBmpTex->GetMapName());
			pAlphaBmp->SetMtlFlag(MTL_TEX_DISPLAY_ENABLED, TRUE);
			pAlphaBmp->SetAlphaAsMono(TRUE);
			pAlphaBmp->SetAlphaSource(FALSE);
			((IParamBlock2*)pAlphaBmp->GetReference(1))->SetValue(10, 0, 1);
			TextureOutput* pTexOut = pAlphaBmp->GetTexout();
			pTexOut->SetInvert(TRUE);

			pBlock1->SetValue(pbr_sg_glossiness_map, m_time, pAlphaBmp);
		}

		float* colSpec = spl_gls->specular_factor;
		if (colSpec) {
			Color col(colSpec[0], colSpec[1], colSpec[2]);
			pBlock1->SetValue(pbr_sg_specular, m_time, col);
		}
		else {
		}
		float f = spl_gls->glossiness_factor;
		pBlock1->SetValue(pbr_sg_glossiness, m_time, f);
	}


	cgltf_texture_view* occTexInfo = &mtl->occlusion_texture;
	if (occTexInfo->texture) {
		pBmpTex = GetBitmapTexFromglTexture(occTexInfo->texture);
		CorrectBitmapGamma(pBmpTex, 1.0f);

		SetTextureUVoffset(pBmpTex, occTexInfo);

		pBmpTex->GetUVGen()->SetTextureTiling(0);		/***********************/

		if (m_MapUnpackMode == 1) {
			Texmap* pMetalTex = NULL;
			Texmap* pRoughTex = NULL;
			Texmap* pOccTex = NULL;
			SetMetalRoughOccOSLMap(pBmpTex, &pMetalTex, &pRoughTex, &pOccTex, FALSE, FALSE, TRUE, occTexInfo);
			pBlock1->SetValue(pbr_sg_ao_map, m_time, pOccTex);
		}
		else if (m_MapUnpackMode == 2) {
			Texmap* pMetalTex = NULL;
			Texmap* pRoughTex = NULL;
			Texmap* pOccTex = NULL;
			SetMetalRoughOccClrCorrectMap(pBmpTex, &pMetalTex, &pRoughTex, &pOccTex, FALSE, FALSE, TRUE);
			pBlock1->SetValue(pbr_sg_ao_map, m_time, pOccTex);
		}
		else {
			pBmpTex = SplitOcclusionTexture(pBmpTex);
			pBlock1->SetValue(pbr_sg_ao_map, m_time, pBmpTex);
		}
	}

	cgltf_texture_view* nrmTexInfo = &mtl->normal_texture;
	if (nrmTexInfo->texture) {
		pBmpTex = GetBitmapTexFromglTexture(nrmTexInfo->texture);
		CorrectBitmapGamma(pBmpTex, 1.0f);

		SetTextureUVoffset(pBmpTex, nrmTexInfo);
		pBlock1->SetValue(pbr_sg_norm_map, m_time, pBmpTex);
		pBlock1->SetValue(pbr_sg_bump_map_amt, m_time, nrmTexInfo->scale);
	}

	cgltf_texture_view* emissTexInfo = &mtl->emissive_texture;
	if (emissTexInfo->texture) {
		pBmpTex = GetBitmapTexFromglTexture(emissTexInfo->texture);
		SetTextureUVoffset(pBmpTex, emissTexInfo);
		pBlock1->SetValue(pbr_sg_emit_color_map, m_time, pBmpTex);
	}

	float* emissive = mtl->emissive_factor;
	if (emissive) {
		Color c(emissive[0], emissive[1], emissive[2]);
		pBlock1->SetValue(pbr_sg_emit_color, m_time, c);
	}

	if (mtl->has_emissive_strength) {
		cgltf_emissive_strength* strength = &mtl->emissive_strength;
	}
	if (mtl->unlit) {
		CreateUnlitAttr(pSmat, mtl->unlit);
	}
	if (mtl->has_iridescence) {
		CreateIridescenceAttr(pSmat, &mtl->iridescence, mtl->has_iridescence);
	}
	if (mtl->has_diffuse_transmission) {
		CreateDiffuseTransmissionAttr(pSmat, &mtl->diffuse_transmission, mtl->has_diffuse_transmission);
	}


	return pSmat;
}
