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

#pragma warning( disable : 4101 )

//=============================================================================
//=============================================================================
void glTFImporter_Core::CreateOpenPBRMaterial(void)
{
	for (int i = 0; i < m_glTF_data->materials_count; i++) {
		cgltf_material *mtl = &m_glTF_data->materials[i];

		Mtl *pSmat = (Mtl*)GetCOREInterface()->CreateInstance(MATERIAL_CLASS_ID, OpenPBRMaterialID);
		pSmat->SetName(StringToWString(mtl->name).c_str());
		IParamBlock2 *pBlock0 = pSmat->GetParamBlockByID(0);

		BitmapTex *pBmpTex;

		cgltf_pbr_metallic_roughness *metalRgh = &mtl->pbr_metallic_roughness;
		pBlock0->SetValue(opbr_base_weight, m_time, 1.0f);

		//pBlock0->SetValue(pbr_normal_flip_red, m_time, m_FlipNormalRed);
		//pBlock0->SetValue(pbr_normal_flip_green, m_time, m_FlipNormalGrn);

		float *col = metalRgh->base_color_factor;
		if (col) {
			AColor c(col[0], col[1], col[2], col[3]);
			pBlock0->SetValue(opbr_base_color, m_time, c);
			pBlock0->SetValue(opbr_transmission_color, m_time, c);
		}

		float metal = metalRgh->metallic_factor;
		pBlock0->SetValue(opbr_base_metalness, m_time, metal);
		float rough = metalRgh->roughness_factor;
		pBlock0->SetValue(opbr_specular_roughness, m_time, rough);

		if (metalRgh->base_color_texture.texture) {
			pBmpTex = GetBitmapTexFromglTexture(metalRgh->base_color_texture.texture);
			SetTextureUVoffset(pBmpTex, &metalRgh->base_color_texture);
			pBlock0->SetValue(opbr_base_color_map, m_time, pBmpTex);
			pBlock0->SetValue(opbr_base_color_map_on, m_time, 1);
			pBlock0->SetValue(opbr_transmission_color_map, m_time, pBmpTex);
			pBlock0->SetValue(opbr_transmission_color_map_on, m_time, 1);

			if (mtl->alpha_mode == cgltf_alpha_mode_blend) {
				Texmap* pAlphaBmp = CreateAlphaFilterMap(pBmpTex);
				pBlock0->SetValue(opbr_geometry_opacity_map, m_time, pAlphaBmp);
				pBlock0->SetValue(opbr_geometry_opacity_map_on, m_time, 1);
			}
			else if (mtl->alpha_mode == cgltf_alpha_mode_mask) {
				Texmap* pAlphaBmp = CreateAlphaFilterMap(pBmpTex);
				Texmap *pOSLMap = CreateCutOffOSLNode(pAlphaBmp, mtl->alpha_cutoff);
				//if (pOSLMap)
				//	pBlock1->SetValue(pbr_opacity_map, m_time, pOSLMap);
				//Control* pClr = (Control*)GetCOREInterface()->CreateInstance(CTRL_POINT4_CLASS_ID, Class_ID(0x2013, 0x0));
				//pBlock0->SetControllerByID(pbr_base_color, 0, pClr);
				//pBlock->SetControllerByID(0, 0, pClr);
				//if (col) {
				//	pBlock1->SetValue(pbr_base_color, m_time, AColor(col[0], col[1], col[2], col[3]));
				pBlock0->SetValue(opbr_geometry_opacity_map, m_time, pOSLMap);
				pBlock0->SetValue(opbr_geometry_opacity_map_on, m_time, 1);
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
				pBlock0->SetValue(opbr_base_metalness_map, m_time, pMetalTex);
				pBlock0->SetValue(opbr_base_metalness_map_on, m_time, 1);
				pBlock0->SetValue(opbr_specular_roughness_map, m_time, pRoughTex);
				pBlock0->SetValue(opbr_specular_roughness_map_on, m_time, 1);
				//pBlock0->SetValue(pbr_ao_map, m_time, pOccTex);
			}
			else if (m_MapUnpackMode == 2) {
				Texmap* pMetalTex = NULL;
				Texmap* pRoughTex = NULL;
				Texmap* pOccTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, &pMetalTex, &pRoughTex, &pOccTex, Metalness, Roughness, Occlusion);
				pBlock0->SetValue(opbr_base_metalness_map, m_time, pMetalTex);
				pBlock0->SetValue(opbr_base_metalness_map_on, m_time, 1);
				pBlock0->SetValue(opbr_specular_roughness_map, m_time, pRoughTex);
				pBlock0->SetValue(opbr_specular_roughness_map_on, m_time, 1);
				//pBlock1->SetValue(pbr_ao_map, m_time, pOccTex);
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
						//pBlock0->SetValue(pbr_ao_map, m_time, pBmpRTex);
					}
				}
				if (Roughness) {
					BitmapTex *pBmpGTex = SplitRoughnessTexture(pBmpTex);
					if (pBmpGTex) {
						CorrectBitmapGamma(pBmpGTex, 1.0f);
						SetTextureUVoffset(pBmpGTex, &metalRgh->metallic_roughness_texture);
						pBlock0->SetValue(opbr_specular_roughness_map, m_time, pBmpGTex);
						pBlock0->SetValue(opbr_specular_roughness_map_on, m_time, 1);
					}
				}
				if (Metalness) {
					BitmapTex *pBmpBTex = SplitMetalnessTexture(pBmpTex);
					if (pBmpBTex) {
						CorrectBitmapGamma(pBmpBTex, 1.0f);
						SetTextureUVoffset(pBmpBTex, &metalRgh->metallic_roughness_texture);
						pBlock0->SetValue(opbr_specular_roughness_map, m_time, pBmpBTex);
						pBlock0->SetValue(opbr_specular_roughness_map_on, m_time, 1);
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
				//pBlock0->SetValue(pbr_ao_map, m_time, pOccTex);
			}
			else if (m_MapUnpackMode == 2) {
				Texmap* pMetalTex = NULL;
				Texmap* pRoughTex = NULL;
				Texmap* pOccTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, &pMetalTex, &pRoughTex, &pOccTex, FALSE, FALSE, TRUE);
				//pBlock0->SetValue(pbr_ao_map, m_time, pOccTex);
			}
			else {
				pBmpTex = SplitOcclusionTexture(pBmpTex);
				//pBlock0->SetValue(pbr_ao_map, m_time, pBmpTex);
			}
		}

		cgltf_texture_view *nrmTexInfo = &mtl->normal_texture;
		if (nrmTexInfo->texture) {
			pBmpTex = GetBitmapTexFromglTexture(nrmTexInfo->texture);
			CorrectBitmapGamma(pBmpTex, 1.0f);

			pBmpTex = GetBitmapTexFromglTexture(nrmTexInfo->texture);
			CorrectBitmapGamma(pBmpTex, 1.0f);
			SetTextureUVoffset(pBmpTex, nrmTexInfo);

			Texmap* pNormlMap = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, GNORMAL_CLASS_ID);
			pNormlMap->GetParamBlock(0)->SetValue(2, m_time, pBmpTex);
			pNormlMap->GetParamBlock(0)->SetValue(7, m_time, m_FlipNormalRed);
			pNormlMap->GetParamBlock(0)->SetValue(8, m_time, m_FlipNormalGrn);

			pBlock0->SetValue(opbr_bump_map, m_time, pNormlMap);
			pBlock0->SetValue(opbr_bump_map_on, m_time, 1);
			pBlock0->SetValue(opbr_bump_map_amt, m_time, nrmTexInfo->scale);
		}

		Color EmissiveColor(0.0f, 0.0f, 0.0f);
		float* emissive = mtl->emissive_factor;
		if (emissive) {
			EmissiveColor = Color(emissive[0], emissive[1], emissive[2]);
			pBlock0->SetValue(opbr_emission_color, m_time, EmissiveColor);
		}

		cgltf_texture_view *emissTexInfo = &mtl->emissive_texture;
		if (emissTexInfo->texture) {
			pBmpTex = GetBitmapTexFromglTexture(emissTexInfo->texture);
			SetTextureUVoffset(pBmpTex, emissTexInfo);
			pBlock0->SetValue(opbr_emission_weight_map, m_time, pBmpTex);
			pBlock0->SetValue(opbr_emission_weight_map_on, m_time, 1);

			Texmap* pColMulTex = CreateColorMultiplyOSLNode(pBmpTex, EmissiveColor);
			pBlock0->SetValue(opbr_emission_color_map, m_time, pColMulTex);
			pBlock0->SetValue(opbr_emission_color_map_on, m_time, 1);

			//Control* pColorCtrl = (Control*)GetCOREInterface()->CreateInstance(CTRL_POINT4_CLASS_ID, Class_ID(0x2012, 0x0));
			//pBlock1->SetControllerByIndex(10, 0, pColorCtrl);
			//pColMulTex->GetParamBlock(1)->SetControllerByIndex(1, 0, pColorCtrl);
		}

		if (mtl->has_emissive_strength) {
			cgltf_emissive_strength *strength = &mtl->emissive_strength;
		}

		if (mtl->has_clearcoat) {
			cgltf_clearcoat* clearCoat = &mtl->clearcoat;
			float f = clearCoat->clearcoat_factor;
			pBlock0->SetValue(opbr_coat_weight, m_time, f);
			cgltf_texture_view* clrTexInfo = &clearCoat->clearcoat_texture;
			if (clrTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(clrTexInfo->texture);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, clrTexInfo);
				Texmap* pRTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, NULL, NULL, &pRTex, FALSE, FALSE, TRUE);
				pBlock0->SetValue(opbr_coat_weight_map, m_time, pRTex);
				pBlock0->SetValue(opbr_coat_weight_map_on, m_time, 1);
			}
			cgltf_texture_view* clrRghTexInfo = &clearCoat->clearcoat_roughness_texture;
			if (clrRghTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(clrRghTexInfo->texture);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, clrRghTexInfo);
				Texmap* pGTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, NULL, &pGTex, NULL, FALSE, TRUE, FALSE);
				pBlock0->SetValue(opbr_coat_roughness_map, m_time, pGTex);
				pBlock0->SetValue(opbr_coat_roughness_map_on, m_time, 1);
				pBlock0->SetValue(opbr_coat_roughness, m_time, clrRghTexInfo->scale);
			}
			f = clearCoat->clearcoat_roughness_factor;
			pBlock0->SetValue(opbr_coat_roughness, m_time, f);
		}

		if (mtl->has_transmission) {
			cgltf_transmission *transmission = &mtl->transmission;
			float f = transmission->transmission_factor;
			pBlock0->SetValue(opbr_transmission_weight, m_time, f);
			pBmpTex = GetBitmapTexFromglTexture(transmission->transmission_texture.texture);
			if (pBmpTex) {
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, &transmission->transmission_texture);
				pBlock0->SetValue(opbr_transmission_weight_map, m_time, pBmpTex);
				pBlock0->SetValue(opbr_transmission_weight_map_on, m_time, 1);
			}
		}

		if (mtl->has_volume) {
			cgltf_volume* volume = &mtl->volume;
		}

		if (mtl->has_specular) {
			cgltf_specular specular;
		}

		if (mtl->has_sheen) {
			cgltf_sheen* sheen = &mtl->sheen;
			float* col = sheen->sheen_color_factor;
			if (col) {
				Color c(col[0], col[1], col[2]);
				pBlock0->SetValue(opbr_fuzz_color, m_time, c);
			}
			cgltf_texture_view* sheenTexInfo = &sheen->sheen_color_texture;
			if (sheenTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(sheenTexInfo->texture);
				SetTextureUVoffset(pBmpTex, sheenTexInfo);
				pBlock0->SetValue(opbr_fuzz_color_map, m_time, pBmpTex);
				pBlock0->SetValue(opbr_fuzz_color_map_on, m_time, 1);
			}
			pBlock0->SetValue(opbr_fuzz_roughness, m_time, sheen->sheen_roughness_factor);
			sheenTexInfo = &sheen->sheen_roughness_texture;
			if (sheenTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(sheenTexInfo->texture);
				SetTextureUVoffset(pBmpTex, sheenTexInfo);

				Texmap* pAlphaTex = NULL;
				SetAlphaClrCorrectMap(pBmpTex, &pAlphaTex);
				pBlock0->SetValue(opbr_fuzz_roughness_map, m_time, pAlphaTex);
				pBlock0->SetValue(opbr_fuzz_roughness_map_on, m_time, 1);
				pBlock0->SetValue(opbr_fuzz_roughness, m_time, sheen->sheen_roughness_factor);
			}
		}

		if (mtl->has_iridescence) {
			cgltf_iridescence* iridescence = &mtl->iridescence;
			
			pBlock0->SetValue(opbr_thin_film_weight, m_time, iridescence->iridescence_factor);
			pBlock0->SetValue(opbr_thin_film_ior, m_time, iridescence->iridescence_ior);

			cgltf_texture_view* texInfo1 = &iridescence->iridescence_texture;
			if (texInfo1->texture) {
				BitmapTex* pBmpTex1 = GetBitmapTexFromglTexture(texInfo1->texture);
				SetTextureUVoffset(pBmpTex1, texInfo1);
				Texmap* pRTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex1, NULL, NULL, &pRTex, FALSE, FALSE, TRUE);
				pBlock0->SetValue(opbr_thin_film_weight_map, m_time, pRTex);
				pBlock0->SetValue(opbr_thin_film_weight_map_on, m_time, 1);
			}
			cgltf_texture_view* texInfo2 = &iridescence->iridescence_thickness_texture;
			if (texInfo2->texture) {
				BitmapTex* pBmpTex2 = GetBitmapTexFromglTexture(texInfo2->texture);
				SetTextureUVoffset(pBmpTex2, texInfo2);
				Texmap* pGTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex2, NULL, &pGTex, NULL, FALSE, TRUE, FALSE);
				pBlock0->SetValue(opbr_thin_film_thickness_map, m_time, pGTex);
				pBlock0->SetValue(opbr_thin_film_thickness_map_on, m_time, 1);
			}
		}
		else {
		}

		if (mtl->has_anisotropy) {
			cgltf_anisotropy* anisotropy = &mtl->anisotropy;
			pBlock0->SetValue(opbr_specular_roughness_anisotropy, m_time, anisotropy->anisotropy_strength);
			cgltf_texture_view* anisoTexInfo = &anisotropy->anisotropy_texture;
			pBmpTex = GetBitmapTexFromglTexture(anisoTexInfo->texture);
			if (pBmpTex) {
				SetTextureUVoffset(pBmpTex, anisoTexInfo);

				Texmap* pMetalTex = NULL;
				Texmap* pRoughTex = NULL;
				Texmap* pOccTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, &pMetalTex, &pRoughTex, &pOccTex, TRUE, FALSE, TRUE);// , & metalRgh->metallic_roughness_texture);

				pBlock0->SetValue(opbr_specular_roughness_anisotropy_map, m_time, pMetalTex);
				pBlock0->SetValue(opbr_specular_roughness_anisotropy_map_on, m_time, 1);

				Texmap* pGainTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, ColorCorrectTexID);
				IParamBlock2* p1 = pGainTex->GetParamBlock(0);
				p1->SetValue(1, m_time, pOccTex);
				p1->SetValue(11, m_time, 1);
				p1->SetValue(18, m_time, 50.0f);
				p1->SetValue(2, m_time, 2);
				p1->SetValue(3, m_time, 4);
				p1->SetValue(4, m_time, 5);
				p1->SetValue(5, m_time, 6);

				Texmap* pLiftTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, ColorCorrectTexID);
				IParamBlock2* p2 = pLiftTex->GetParamBlock(0);
				p2->SetValue(1, m_time, pOccTex);
				p2->SetValue(11, m_time, 1);
				p2->SetValue(18, m_time, 50.0f);
				p2->SetValue(30, m_time, 0.5f);

				Texmap* pCutOffTex = CreateCutOffOSLNode(pRoughTex, 0.5f);

				Texmap* pMixTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, MixTexID);
				IParamBlock2* p3 = pMixTex->GetParamBlock(0);
				p3->SetValue(6, m_time, pLiftTex);
				p3->SetValue(7, m_time, pGainTex);
				p3->SetValue(8, m_time, pCutOffTex);

				pBlock0->SetValue(opbr_geometry_tangent_map, m_time, pMixTex);
				pBlock0->SetValue(opbr_geometry_tangent_map_on, m_time, 1);
			}
		}

		if (mtl->has_ior) {
			cgltf_ior* ior = &mtl->ior;
			pBlock0->SetValue(opbr_specular_ior, m_time, ior->ior);
		}






		if (mtl->has_dispersion) {
			cgltf_dispersion* dispersion = &mtl->dispersion;
			if (dispersion->dispersion != 0.0f) {
				pBlock0->SetValue(opbr_transmission_dispersion_scale, m_time, 1.0f);
				pBlock0->SetValue(opbr_transmission_dispersion_abbe_number, m_time, (20.0f/ dispersion->dispersion));
			}
			else
				pBlock0->SetValue(opbr_transmission_dispersion_scale, m_time, 0.0f);
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

		AttachAlphaModeCustAttr(pSmat, mtl->alpha_mode);

		CreateUnlitAttr(pSmat, mtl->unlit);
		CreateVolumeAttr(pSmat, &mtl->volume, mtl->has_volume);
		CreateDiffuseTransmissionAttr(pSmat, &mtl->diffuse_transmission, mtl->has_diffuse_transmission);
//		CreateIORAttr(pSmat, &mtl->ior, mtl->has_ior);
	//		CreateIridescenceAttr(pSmat, &mtl->iridescence, mtl->has_iridescence);
//		CreateSheenAttr(pSmat, &mtl->sheen, mtl->has_sheen);
//		CreateEmissiveStrengthAttr(pSmat, &mtl->emissive_strength, mtl->has_emissive_strength);
//		CreateAnisotropyAttr(pSmat, &mtl->anisotropy, mtl->has_anisotropy);
//		CreateSpecularAttr(pSmat, &mtl->specular, mtl->has_specular);
//		CreateDispersionAttr(pSmat, &mtl->dispersion, mtl->has_dispersion);

		m_MaterialMap.insert(std::make_pair(mtl, pSmat));
		SetMtlImportStatus(i + 1);
	}
}
