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

#include "HSglTFImporter.h"
#include "define.h"


//=============================================================================
//=============================================================================
void glTFImporter_Core::CreatePBRMetalMaterial(void)
{
	for (int i = 0; i < m_glTF_data->materials_count; i++) {
		cgltf_material *mtl = &m_glTF_data->materials[i];
		if (mtl->has_pbr_specular_glossiness) {
			Mtl* pSmat = CreatePBRSpecGlossMtl(mtl);
			m_MaterialMap.insert(std::make_pair(mtl, pSmat));
			SetMtlImportStatus(i + 1);
			continue;
		}

		Mtl *pSmat = (Mtl*)GetCOREInterface()->CreateInstance(MATERIAL_CLASS_ID, PBRMetalMtlID);
		pSmat->SetName(StringToWString(mtl->name).c_str());
		IParamBlock2 *pBlock0 = pSmat->GetParamBlockByID(0);
		IParamBlock2 *pBlock1 = pSmat->GetParamBlockByID(1);
		//const TCHAR *ptr = pBlock->GetLocalName();

		//cgltf_texture *tex;
		//cgltf_image *image;
		BitmapTex *pBmpTex;

		cgltf_pbr_metallic_roughness *metalRgh = &mtl->pbr_metallic_roughness;

		pBlock0->SetValue(pbr_normal_flip_red, m_time, m_FlipNormalRed);
		pBlock0->SetValue(pbr_normal_flip_green, m_time, m_FlipNormalGrn);

		float *col = metalRgh->base_color_factor;
		if (col) {
			AColor c(col[0], col[1], col[2], col[3]);
			pBlock1->SetValue(pbr_base_color, m_time, c);
		}

		float metal = metalRgh->metallic_factor;
		pBlock1->SetValue(pbr_metalness, m_time, metal);
		float rough = metalRgh->roughness_factor;
		pBlock1->SetValue(pbr_roughness, m_time, rough);

		if (metalRgh->base_color_texture.texture) {
			pBmpTex = GetBitmapTexFromglTexture(metalRgh->base_color_texture.texture);
			SetTextureUVoffset(pBmpTex, &metalRgh->base_color_texture);
			pBlock1->SetValue(pbr_base_color_map, m_time, pBmpTex);

			if (mtl->alpha_mode == cgltf_alpha_mode_mask || mtl->alpha_mode == cgltf_alpha_mode_blend) {
/*
				BitmapTex *pAlphaBmp = NewDefaultBitmapTex();
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
					Texmap* pCCTex = CreateAlphaFilterMap(NULL, AColor(col));
					IParamBlock2* pBlock = pCCTex->GetParamBlock(0);

					Texmap *pOSLMap = CreateCutOffOSLNode(pAlphaBmp, mtl->alpha_cutoff, pCCTex);
					if (pOSLMap)
						pBlock1->SetValue(pbr_opacity_map, m_time, pOSLMap);

					Control* pClr = (Control*)GetCOREInterface()->CreateInstance(CTRL_POINT4_CLASS_ID, Class_ID(0x2013, 0x0));
					pBlock1->SetControllerByID(pbr_base_color, 0, pClr);
					pBlock->SetControllerByID(0, 0, pClr);
					if (col) {
						pBlock1->SetValue(pbr_base_color, m_time, AColor(col[0], col[1], col[2], col[3]));
					}
				}
				else {
					pBlock1->SetValue(pbr_opacity_map, m_time, pAlphaBmp);
				}
				//pBlock1->SetValue(pbr_opacity_map, m_time, pAlphaBmp);
			}
		}

		BOOL Roughness = FALSE;
		BOOL Metalness = FALSE;
		BOOL Occlusion = FALSE;
		if (metalRgh->metallic_roughness_texture.texture) {
			pBmpTex = GetBitmapTexFromglTexture(metalRgh->metallic_roughness_texture.texture);
			CorrectBitmapGamma(pBmpTex, 1.0f);
			SetTextureUVoffset(pBmpTex, &metalRgh->metallic_roughness_texture);
			//Bitmap *pOriginalBmp = pBmpTex->GetBitmap(0);
			//BitmapInfo bi = pOriginalBmp->GetBitmapInfo();
			std::filesystem::path fname = pBmpTex->GetMapName();
			//tstring orgFilePath = fname;
			//std::filesystem::path fname = bi.Filename();
			//tstring orgFilePath = tstring(m_fullpath.parent_path()) + tstring(_T("\\")) + tstring(fname);

			tstring texFilePathR = tstring(m_WorkImageFolder) + tstring(fname.stem()) + tstring(_T("_R")) + tstring(fname.extension());
			tstring texFilePathG = tstring(m_WorkImageFolder) + tstring(fname.stem()) + tstring(_T("_G")) + tstring(fname.extension());
			tstring texFilePathB = tstring(m_WorkImageFolder) + tstring(fname.stem()) + tstring(_T("_B")) + tstring(fname.extension());

			tstring name = tstring(fname.stem());
			std::transform(name.cbegin(), name.cend(), name.begin(), tolower);
			Roughness = (metalRgh->roughness_factor > 0.0) ? TRUE : FALSE;
			Metalness = (metalRgh->metallic_factor > 0.0) ? TRUE : FALSE;
			if (mtl->occlusion_texture.texture) {
				BitmapTex *p = GetBitmapTexFromglTexture(mtl->occlusion_texture.texture);
				TSTR f1 = p->GetMapName();
				TSTR f2 = pBmpTex->GetMapName();
				if (f1 == f2) Occlusion = TRUE;
			}

			if (m_MapUnpackMode == 1) {
				Texmap *pMetalTex = NULL;
				Texmap *pRoughTex = NULL;
				Texmap *pOccTex = NULL;
				SetMetalRoughOccOSLMap(pBmpTex, &pMetalTex, &pRoughTex, &pOccTex, Metalness, Roughness, Occlusion, &metalRgh->metallic_roughness_texture);
				pBlock1->SetValue(pbr_metalness_map, m_time, pMetalTex);
				pBlock1->SetValue(pbr_roughness_map, m_time, pRoughTex);
				pBlock1->SetValue(pbr_ao_map, m_time, pOccTex);
			}
			else if (m_MapUnpackMode == 2) {
				Texmap* pMetalTex = NULL;
				Texmap* pRoughTex = NULL;
				Texmap* pOccTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, &pMetalTex, &pRoughTex, &pOccTex, Metalness, Roughness, Occlusion);
				pBlock1->SetValue(pbr_metalness_map, m_time, pMetalTex);
				pBlock1->SetValue(pbr_roughness_map, m_time, pRoughTex);
				pBlock1->SetValue(pbr_ao_map, m_time, pOccTex);
			}
			else {
				if (Occlusion) {
					BitmapTex *pBmpRTex = SplitOcclusionTexture(pBmpTex);
					if (pBmpRTex) {
						CorrectBitmapGamma(pBmpRTex, 1.0f);
						if (&mtl->occlusion_texture) {
							SetTextureUVoffset(pBmpRTex, &mtl->occlusion_texture);
						}
						else {
							SetTextureUVoffset(pBmpTex, &metalRgh->metallic_roughness_texture);
						}
						pBlock1->SetValue(pbr_ao_map, m_time, pBmpRTex);
					}
				}
				if (Roughness) {
					BitmapTex *pBmpGTex = SplitRoughnessTexture(pBmpTex);
					if (pBmpGTex) {
						CorrectBitmapGamma(pBmpGTex, 1.0f);
						SetTextureUVoffset(pBmpGTex, &metalRgh->metallic_roughness_texture);
						pBlock1->SetValue(pbr_roughness_map, m_time, pBmpGTex);
					}
				}
				if (Metalness) {
					BitmapTex *pBmpBTex = SplitMetalnessTexture(pBmpTex);
					if (pBmpBTex) {
						CorrectBitmapGamma(pBmpBTex, 1.0f);
						SetTextureUVoffset(pBmpBTex, &metalRgh->metallic_roughness_texture);
						pBlock1->SetValue(pbr_metalness_map, m_time, pBmpBTex);
					}
				}
			}
		}

		cgltf_texture_view *occTexInfo = &mtl->occlusion_texture;
		if (occTexInfo->texture && Occlusion==FALSE) {
			pBmpTex = GetBitmapTexFromglTexture(occTexInfo->texture);
			CorrectBitmapGamma(pBmpTex, 1.0f);

			SetTextureUVoffset(pBmpTex, occTexInfo);

			pBmpTex->GetUVGen()->SetTextureTiling(0);		/***********************/

			if (m_MapUnpackMode == 1) {
				Texmap* pMetalTex = NULL;
				Texmap* pRoughTex = NULL;
				Texmap* pOccTex = NULL;
				SetMetalRoughOccOSLMap(pBmpTex, &pMetalTex, &pRoughTex, &pOccTex, FALSE, FALSE, TRUE, occTexInfo);
				pBlock1->SetValue(pbr_ao_map, m_time, pOccTex);
			}
			else if (m_MapUnpackMode == 2) {
				Texmap* pMetalTex = NULL;
				Texmap* pRoughTex = NULL;
				Texmap* pOccTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, &pMetalTex, &pRoughTex, &pOccTex, FALSE, FALSE, TRUE);
				pBlock1->SetValue(pbr_ao_map, m_time, pOccTex);
			}
			else {
				pBmpTex = SplitOcclusionTexture(pBmpTex);
				pBlock1->SetValue(pbr_ao_map, m_time, pBmpTex);
			}
		}

		cgltf_texture_view *nrmTexInfo = &mtl->normal_texture;
		if (nrmTexInfo->texture) {
			pBmpTex = GetBitmapTexFromglTexture(nrmTexInfo->texture);
			CorrectBitmapGamma(pBmpTex, 1.0f);

			SetTextureUVoffset(pBmpTex, nrmTexInfo);
			pBlock1->SetValue(pbr_norm_map, m_time, pBmpTex);
			pBlock1->SetValue(pbr_bump_map_amt, m_time, nrmTexInfo->scale);
		}

		Color EmissiveColor(0.0f);
		float* emissive = mtl->emissive_factor;
		if (emissive) {
			EmissiveColor = Color(emissive[0], emissive[1], emissive[2]);
			pBlock1->SetValue(pbr_emit_color, m_time, EmissiveColor);
		}

		cgltf_texture_view *emissTexInfo = &mtl->emissive_texture;
		if (emissTexInfo->texture) {
			pBmpTex = GetBitmapTexFromglTexture(emissTexInfo->texture);
			SetTextureUVoffset(pBmpTex, emissTexInfo);

			Texmap* pColMulTex = CreateColorMultiplyOSLNode(pBmpTex, EmissiveColor);
			pBlock1->SetValue(pbr_emit_color_map, m_time, pColMulTex);

			Control* pColorCtrl = (Control*)GetCOREInterface()->CreateInstance(CTRL_POINT4_CLASS_ID, Class_ID(0x2012, 0x0));
			pBlock1->SetControllerByIndex(10, 0, pColorCtrl);
			pColMulTex->GetParamBlock(1)->SetControllerByIndex(1, 0, pColorCtrl);
		}

		if (mtl->has_emissive_strength) {
			cgltf_emissive_strength *strength = &mtl->emissive_strength;
		}

		if (mtl->has_clearcoat) {
		}

		/*
		if (mtl->has_transmission) {
			cgltf_transmission *transmission = &mtl->transmission;
			pBmpTex = GetBitmapTexFromglTexture(transmission->transmission_texture.texture);
			if (pBmpTex) {
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, &transmission->transmission_texture);
				pBlock1->SetValue(pbr_opacity_map, m_time, pBmpTex);
			}
			else {
				float f = transmission->transmission_factor;
				if (f > 0.0f) {
					Color c(col);
					c *= 0.1f;
					Texmap *pColTex = CreateColorMap(AColor(c));
					pBlock1->SetValue(pbr_opacity_map, m_time, pColTex);
				}
			}
		}
		*/
		if (mtl->has_volume) {
			cgltf_volume volume;
		}

		if (mtl->has_specular) {
			cgltf_specular specular;
		}
		if (mtl->has_sheen) {
			cgltf_sheen sheen;
		}
/*
		cgltf_extension *ext = mtl->extensions;
		for (int cnt = 0; cnt < mtl->extensions_count; cnt++, ext++) {
			char* name = ext->name;
			char* data = ext->data;
		}

		//gltf2::Extension ext = mtl.extensions();
		for (int j = 0; j < mtl->extensions_count; j++) {
			cgltf_extension *ext = &mtl->extensions[j];
		}
		float alphaCutoff = mtl->alpha_cutoff;
		bool doubleSided = mtl->double_sided;
*/

		AttacheAlphaModeCustAttr(pSmat, mtl->alpha_mode);

		CreateIORAttr(pSmat, &mtl->ior, mtl->has_ior);
		CreateTransmissionAttr(pSmat, &mtl->transmission, mtl->has_transmission);
		CreateVolumeAttr(pSmat, &mtl->volume, mtl->has_volume);
		CreateIridescenceAttr(pSmat, &mtl->iridescence, mtl->has_iridescence);
		CreateSheenAttr(pSmat, &mtl->sheen, mtl->has_sheen);
		CreateClearcoatAttr(pSmat, &mtl->clearcoat, mtl->has_clearcoat);
		CreateUnlitAttr(pSmat, mtl->unlit);
		CreateEmissiveStrengthAttr(pSmat, &mtl->emissive_strength, mtl->has_emissive_strength);
		CreateAnisotropyAttr(pSmat, &mtl->anisotropy, mtl->has_anisotropy);
		CreateSpecularAttr(pSmat, &mtl->specular, mtl->has_specular);
		CreateDispersionAttr(pSmat, &mtl->dispersion, mtl->has_dispersion);
		CreateDiffuseTransmissionAttr(pSmat, &mtl->diffuse_transmission, mtl->has_diffuse_transmission);

		m_MaterialMap.insert(std::make_pair(mtl, pSmat));
		SetMtlImportStatus(i + 1);
	}
}
