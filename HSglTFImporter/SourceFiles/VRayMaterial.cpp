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
void glTFImporter_Core::CreateVRayMaterial(void)
{
	for (int i = 0; i < m_glTF_data->materials_count; i++) {
		//cgltf_texture *tex;
		//cgltf_image *image;
		BitmapTex *pBmpTex;

		cgltf_material *mtl = &m_glTF_data->materials[i];
		Mtl *pSmat = (Mtl*)GetCOREInterface()->CreateInstance(MATERIAL_CLASS_ID, VRayMaterialID);
		pSmat->SetName(StringToWString(mtl->name).c_str());
		IParamBlock2 *pBlock0 = pSmat->GetParamBlock(0);
		IParamBlock2 *pBlock1 = pSmat->GetParamBlockByID(1);
		IParamBlock2 *pBlock2 = pSmat->GetParamBlockByID(2);
		IParamBlock2 *pBlock3 = pSmat->GetParamBlockByID(4);

		pBlock2->SetValue(vr_brdf_type, m_time, 1);
		pBlock2->SetValue(vr_brdf_useRoughness, m_time, 1);

		pSmat->GetParamBlockByID(3)->SetValue(vr_option_doubleSided, m_time, mtl->double_sided);

		cgltf_pbr_metallic_roughness *metalRgh = &mtl->pbr_metallic_roughness;

		BOOL ColorFound = FALSE;
		float *col = metalRgh->base_color_factor;
		if (col)
			if (col[0] < 1.0f || col[1] < 1.0f || col[2] < 1.0f) ColorFound = TRUE;

		Texmap *pBaseColorMap = NULL;
		if (m_UseColorComposite && metalRgh->base_color_texture.texture && ColorFound) {
			pBaseColorMap = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, VRayCompTexID);
			pBaseColorMap->GetParamBlock(0)->SetValue(2, m_time, 3);
			pBlock0->SetValueByName(vr_texmap_diffuse, pBaseColorMap, m_time);
			pBlock3->SetValueByName(vr_texmap_diffuse, pBaseColorMap, m_time);
		}

		if (ColorFound) {
			//AColor c(col[0], col[1], col[2], col[3]);
			Color c(col[0], col[1], col[2]);
			if (pBaseColorMap && metalRgh->base_color_texture.texture) {
				Texmap *pColTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, VRayColorID);
				pColTex->GetParamBlock(0)->SetValue(5, m_time, c);
				pBaseColorMap->GetParamBlock(0)->SetValue(0, m_time, pColTex);

				Control* pClr = (Control*)GetCOREInterface()->CreateInstance(CTRL_POINT4_CLASS_ID, Class_ID(0x2013, 0x0));
				pBlock0->SetControllerByID(vr_diffuse, 0, pClr);
				pColTex->GetParamBlock(0)->SetControllerByID(5, 0, pClr);
			}
			pBlock0->SetValue(vr_diffuse, m_time, c);
		}
		else {
			pBlock0->SetValue(vr_diffuse, m_time, Color(1.0f, 1.0f, 1.0f));
		}

		pBlock0->SetValue(vr_reflection, m_time, Color(1.0f, 1.0f, 1.0f));

		if (metalRgh->base_color_texture.texture) {
			pBmpTex = GetBitmapTexFromglTexture(metalRgh->base_color_texture.texture);
			SetTextureUVoffset(pBmpTex, &metalRgh->base_color_texture);
			Texmap *pTex = BitmapTexToVRayBitmap(pBmpTex);
			if (pBaseColorMap) {
				pBaseColorMap->GetParamBlock(0)->SetValue(1, m_time, pTex);
			}
			else {
				pBlock0->SetValueByName(vr_texmap_diffuse, pTex, m_time);
				pBlock3->SetValueByName(vr_texmap_diffuse, pTex, m_time);
			}

			if (mtl->alpha_mode == cgltf_alpha_mode_mask || mtl->alpha_mode == cgltf_alpha_mode_blend) {
				//BitmapTex *pAlphaBmp = NewDefaultBitmapTex();
				/*
				Texmap *pAlphaBmp = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, VRayBitmapID);
				pAlphaBmp->GetParamBlock(0)->SetValue(0, 0, pBmpTex->GetMapName());
				pAlphaBmp->GetParamBlock(0)->SetValue(22, 0, 1);
				pAlphaBmp->GetParamBlock(0)->SetValue(23, 0, 1);

				pAlphaBmp->SetName(pBmpTex->GetName() + TSTR(_T("_Alpha")));
				pAlphaBmp->GetUVGen()->SetCoordMapping(UVMAP_SCREEN_ENV);
				pAlphaBmp->GetUVGen()->SetTextureTiling(U_WRAP | V_WRAP);
				pAlphaBmp->GetUVGen()->InitSlotType(MAPSLOT_TEXTURE);
				pAlphaBmp->SetMapName(((BitmapTex*)pBmpTex)->GetMapName());
				pAlphaBmp->SetMtlFlag(MTL_TEX_DISPLAY_ENABLED, TRUE);
				pAlphaBmp->SetAlphaAsMono(TRUE);
				*/
				Texmap* pAlphaBmp = CreateAlphaFilterMap(pTex);

				if (mtl->alpha_mode == cgltf_alpha_mode_mask) {
					Texmap *pCCTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, ColorCorrectTexID);
					IParamBlock2* pBlock = pCCTex->GetParamBlock(0);	
					AColor c(col);
					pBlock->SetValue(0, m_time, c);	
					pBlock->SetValue(2, m_time, 3);
					pBlock->SetValue(3, m_time, 3);
					pBlock->SetValue(4, m_time, 3);
					pBlock->SetValue(5, m_time, 3);

					Texmap* pOSLMap = CreateCutOffOSLNode(pAlphaBmp, mtl->alpha_cutoff, pCCTex);
					if (pOSLMap)
						pBlock3->SetValue(vr_texmap_opacity, m_time, pOSLMap);
					else
						pBlock3->SetValue(vr_texmap_opacity, m_time, pAlphaBmp);

				}
				else {
					pBlock3->SetValue(vr_texmap_opacity, m_time, pAlphaBmp);
				}
			}
		}

		float metal = metalRgh->metallic_factor;
		pBlock0->SetValue(vr_reflection_metalness, m_time, metal);
		if (metal==0.0f) {
			pBlock0->SetValue(vr_reflection, m_time, Color(1.0f, 1.0f, 1.0f));
		}

		pBlock0->SetValue(vr_diffuse_roughness, m_time, 0.0f);
		//pBlock0->SetValue(vr_reflection_glossiness, m_time, 1.0f-rough);

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
				if (f1 == f2)Occlusion = TRUE;
			}

			if (m_MapUnpackMode == 1) {
				Texmap* pOrgTex = BitmapTexToVRayBitmap(pBmpTex, 1.0f);

				Texmap *pMetalTex = NULL;
				Texmap *pRoughTex = NULL;
				Texmap *pOccTex = NULL;
				SetMetalRoughOccOSLMap(pBmpTex, &pMetalTex, &pRoughTex, &pOccTex, Metalness, Roughness, Occlusion, &metalRgh->metallic_roughness_texture);
				pBlock0->SetValueByName(vr_texmap_metalness_Str, pMetalTex, m_time);
				pBlock3->SetValueByName(vr_texmap_metalness_Str, pMetalTex, m_time);
				pBlock0->SetValueByName(vr_texmap_reflectionGlossiness_Str, pRoughTex, m_time);
				pBlock3->SetValueByName(vr_texmap_reflectionGlossiness_Str, pRoughTex, m_time);
			}
			else if (m_MapUnpackMode == 2) {
				Texmap* pOrgTex = BitmapTexToVRayBitmap(pBmpTex, 1.0f);

				Texmap* pMetalTex = NULL;
				Texmap* pRoughTex = NULL;
				Texmap* pOccTex = NULL;
				SetMetalRoughOccClrCorrectMap(pOrgTex, &pMetalTex, &pRoughTex, &pOccTex, Metalness, Roughness, Occlusion);
				pBlock0->SetValueByName(vr_texmap_metalness_Str, pMetalTex, m_time);
				pBlock3->SetValueByName(vr_texmap_metalness_Str, pMetalTex, m_time);
				pBlock0->SetValueByName(vr_texmap_reflectionGlossiness_Str, pRoughTex, m_time);
				pBlock3->SetValueByName(vr_texmap_reflectionGlossiness_Str, pRoughTex, m_time);
			}
			else {
				//if (Occlusion) {
				//	BitmapTex *pBmpRTex = SplitOcclusionTexture(pBmpTex);
				//	SetTextureUVoffset(pBmpTex, &metalRgh->metallic_roughness_texture);
				//	pBlock3->SetValue(pbr_ao_map, m_time, pBmpRTex);
				//}
				if (Roughness) {
					//pBlock2->SetValue(vr_brdf_useRoughness, m_time, 1);
					BitmapTex *pBmpGTex = SplitRoughnessTexture(pBmpTex);
					SetTextureUVoffset(pBmpGTex, &metalRgh->metallic_roughness_texture);
					Texmap *pTex = BitmapTexToVRayBitmap(pBmpGTex, 1.0f);
					pBlock0->SetValueByName(vr_texmap_reflectionGlossiness_Str, pTex, m_time);
					pBlock3->SetValueByName(vr_texmap_reflectionGlossiness_Str, pTex, m_time);
				}
				if (Metalness) {
					BitmapTex *pBmpBTex = SplitMetalnessTexture(pBmpTex);
					SetTextureUVoffset(pBmpBTex, &metalRgh->metallic_roughness_texture);
					Texmap *pTex = BitmapTexToVRayBitmap(pBmpBTex, 1.0f);
					pBlock0->SetValueByName(vr_texmap_metalness_Str, pTex, m_time);
					pBlock3->SetValueByName(vr_texmap_metalness_Str, pTex, m_time);
				}
			}
		}

		//if(!pBlock3->GetTexmap(vr_texmap_reflectionGlossiness)){
		//	float rough = metalRgh->roughness_factor*100.0f;
		//	pBlock3->SetValue(vr_texmap_reflectionGlossiness_multiplier, m_time, rough);
		//}

		cgltf_texture_view *nrmTexInfo = &mtl->normal_texture;
		if (nrmTexInfo->texture) {
			pBmpTex = GetBitmapTexFromglTexture(nrmTexInfo->texture);
			CorrectBitmapGamma(pBmpTex, 1.0f);
			SetTextureUVoffset(pBmpTex, nrmTexInfo);

			Texmap *pNormlMap = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, VRayNormalMapID);
			pNormlMap->GetParamBlock(0)->SetValue(vr_nrm_normal_map, m_time, BitmapTexToVRayBitmap(pBmpTex, 1.0f));
			//pNormlMap->GetParamBlock(0)->SetValue(crn_nrm_multplier, m_time, nrmTexInfo->scale);
			pNormlMap->GetParamBlock(0)->SetValue(vr_nrm_flip_red, m_time, m_FlipNormalRed);
			pNormlMap->GetParamBlock(0)->SetValue(vr_nrm_flip_green, m_time, m_FlipNormalGrn);
			pBlock0->SetValueByName(vr_texmap_bump_Str, pNormlMap, m_time);
			pBlock3->SetValueByName(vr_texmap_bump_Str, pNormlMap, m_time);
			pBlock0->SetValue(vr_texmap_bump_multiplier, m_time, nrmTexInfo->scale*100.0f);
		}

		if (mtl->has_bump) {
			cgltf_material_bump* bump = &mtl->material_bump;
			cgltf_texture_view* bumpTexInfo = &bump->bumpTexture;
			if (bumpTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(bumpTexInfo->texture);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, bumpTexInfo);
				pBlock0->SetValueByName(vr_texmap_bump_Str, pBmpTex, m_time);
				pBlock0->SetValue(vr_texmap_bump_multiplier, m_time, bump->bump_factor*100.0f);
			}
		}

		Color EmissiveColor(0.0f, 0.0f, 0.0f);
		float* emissive = mtl->emissive_factor;
		if (emissive) {
			EmissiveColor = Color(emissive[0], emissive[1], emissive[2]);
			pBlock0->SetValue(vr_selfIllumination, m_time, EmissiveColor);
		}

		cgltf_texture_view *emissTexInfo = &mtl->emissive_texture;
		if (emissTexInfo->texture) {
			pBmpTex = GetBitmapTexFromglTexture(emissTexInfo->texture);
			SetTextureUVoffset(pBmpTex, emissTexInfo);
			Texmap *pTex = BitmapTexToVRayBitmap(pBmpTex);

			Texmap* pColMulTex = CreateColorMultiplyOSLNode(pTex, EmissiveColor);
			pBlock0->SetValueByName(vr_texmap_self_illumination_Str, pColMulTex, m_time);
			pBlock3->SetValueByName(vr_texmap_self_illumination_Str, pColMulTex, m_time);

			Control* pColorCtrl = (Control*)GetCOREInterface()->CreateInstance(CTRL_POINT4_CLASS_ID, Class_ID(0x2012, 0x0));
			pBlock0->SetControllerByIndex(5, 0, pColorCtrl);
			pColMulTex->GetParamBlock(1)->SetControllerByIndex(1, 0, pColorCtrl);
		}

		if (mtl->has_emissive_strength) {
			cgltf_emissive_strength* strength = &mtl->emissive_strength;
			pBlock0->SetValue(vr_selfIllumination_multiplier, m_time, strength->emissive_strength);
		}

		if (mtl->has_clearcoat) {
			cgltf_clearcoat *clearcoat = &mtl->clearcoat;

			pSmat->GetParamBlockByID(0)->SetValue(vr_coat_amount, m_time, clearcoat->clearcoat_factor);
			pSmat->GetParamBlockByID(0)->SetValue(vr_coat_glossiness, m_time, clearcoat->clearcoat_roughness_factor);

			//pBlock3->SetValueByName(vr_coat_amount, clearcoat->clearcoat_factor, m_time);
			//pBlock3->SetValueByName(vr_coat_glossiness, clearcoat->clearcoat_roughness_factor, m_time);

			pBmpTex = GetBitmapTexFromglTexture(clearcoat->clearcoat_texture.texture);
			if (pBmpTex) {
				pBlock3->SetValueByName(vr_texmap_coat_amount_on_Str, 1, m_time);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, &clearcoat->clearcoat_texture);
				Texmap* pOrgTex = BitmapTexToVRayBitmap(pBmpTex, 1.0f);

				Texmap* pRTex = NULL;
				SetMetalRoughOccClrCorrectMap(pOrgTex, NULL, NULL, &pRTex, FALSE, FALSE, TRUE);
				//Texmap *pTex = BitmapTexToVRayBitmap(pBmpTex, 1.0f);

				pBlock0->SetValueByName(vr_texmap_coat_amount_Str, pRTex, m_time);
				pBlock3->SetValueByName(vr_texmap_coat_amount_Str, pRTex, m_time);
				pBlock0->SetValue(vr_coat_amount, m_time, clearcoat->clearcoat_factor);
			}
			pBmpTex = GetBitmapTexFromglTexture(clearcoat->clearcoat_roughness_texture.texture);
			if (pBmpTex) {
				pBlock3->SetValueByName(vr_texmap_coat_glossiness_on_Str, 1, m_time);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, &clearcoat->clearcoat_roughness_texture);
				Texmap* pOrgTex = BitmapTexToVRayBitmap(pBmpTex, 1.0f);

				Texmap* pGTex = NULL;
				SetMetalRoughOccClrCorrectMap(pOrgTex, NULL, &pGTex, NULL, FALSE, TRUE, FALSE);
				//Texmap *pTex = BitmapTexToVRayBitmap(pBmpTex, 1.0f);

				pBlock0->SetValueByName(vr_texmap_coat_glossiness_Str, pGTex, m_time);
				pBlock3->SetValueByName(vr_texmap_coat_glossiness_Str, pGTex, m_time);
				pBlock3->SetValueByName(vr_texmap_coat_glossiness_multiplier_Str, clearcoat->clearcoat_roughness_factor*100.0f, m_time);
			}
			pBmpTex = GetBitmapTexFromglTexture(clearcoat->clearcoat_normal_texture.texture);
			if (pBmpTex) {
				pBlock3->SetValueByName(vr_texmap_coat_bump_on_Str, 1, m_time);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, &clearcoat->clearcoat_normal_texture);

				Texmap* pNormlMap = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, VRayNormalMapID);
				pNormlMap->GetParamBlock(0)->SetValue(vr_nrm_normal_map, m_time, BitmapTexToVRayBitmap(pBmpTex, 1.0f));
				pNormlMap->GetParamBlock(0)->SetValue(vr_nrm_flip_red, m_time, m_FlipNormalRed);
				pNormlMap->GetParamBlock(0)->SetValue(vr_nrm_flip_green, m_time, m_FlipNormalGrn);

				pBlock0->SetValueByName(vr_texmap_coat_bump_Str, pNormlMap, m_time);
				pBlock3->SetValueByName(vr_texmap_coat_bump_Str, pNormlMap, m_time);
				pBlock0->SetValue(vr_texmap_coat_bump_multiplier, m_time, 100.0f);
			}
		}

		if (mtl->has_sheen) {
			cgltf_sheen *sheen = &mtl->sheen;
			pBlock0->SetValueByName(vr_sheen_glossiness_Str, sheen->sheen_roughness_factor, m_time);
			float* col = sheen->sheen_color_factor;
			if (col) {
				Color c(col[0], col[1], col[2]);
				pBlock0->SetValueByName(vr_sheen_color_Str, c, m_time);
			}
			cgltf_texture_view* sheenTexInfo = &sheen->sheen_color_texture;
			if (sheenTexInfo->texture) {
				pBlock3->SetValueByName(vr_texmap_sheen_on_Str, 1, m_time);
				pBmpTex = GetBitmapTexFromglTexture(sheenTexInfo->texture);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, sheenTexInfo);
				Texmap *pTex = BitmapTexToVRayBitmap(pBmpTex, 1.0f);
				pBlock3->SetValueByName(vr_texmap_sheen_Str, pTex, m_time);
				pBlock0->SetValueByName(vr_texmap_sheen_Str, pTex, m_time);

				Texmap* pAlphaTex = NULL;
				SetAlphaClrCorrectMap(pBmpTex, &pAlphaTex);
				pBlock0->SetValueByName(vr_texmap_sheen_glossiness_Str, pAlphaTex, m_time);
				//pBlock0->SetValueByName(vr_texmap_sheen_glossiness_on_Str, 1, m_time);
			}
		}
		else {
			pBlock0->SetValueByName(vr_sheen_glossiness_Str, 0.0f, m_time);
		}


		if (mtl->has_transmission) {
			cgltf_transmission* transmission = &mtl->transmission;
			float f = transmission->transmission_factor;
			pBlock0->SetValue(vr_refraction, m_time, Color(f, f, f));
			//pBlock0->SetValue(vr_refraction, m_time, Color(c, c, c));
			//pBlock0->SetValue(vr_diffuse, m_time, Color(0.0f,0.0f, 0.0f));
			//if (f > 0.0f) {
			//	pBlock0->SetValue(vr_diffuse_roughness, m_time, metalRgh->roughness_factor);
			//	pBlock0->SetValue(vr_reflection_glossiness, m_time, 1.0f - metalRgh->roughness_factor);
			//}
			pBmpTex = GetBitmapTexFromglTexture(transmission->transmission_texture.texture);
			if (pBmpTex) {
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, &transmission->transmission_texture);
				Texmap* pOrgTex = BitmapTexToVRayBitmap(pBmpTex, 1.0f);
				Texmap* pRTex = NULL;
				SetMetalRoughOccClrCorrectMap(pOrgTex, NULL, NULL, &pRTex, FALSE, FALSE, TRUE);
				pBlock0->SetValueByName(vr_texmap_refraction_Str, pRTex, m_time);
				pBlock3->SetValueByName(vr_texmap_refraction_Str, pRTex, m_time);
			}
		}

		if (mtl->has_volume) {
			cgltf_volume* volume = &mtl->volume;
			pBlock0->SetValue(vr_translucency_on, m_time, 5);
			float* col = volume->attenuation_color;
			if (col) {
				Color c(col[0], col[1], col[2]);
				pBlock0->SetValue(vr_translucency_color, m_time, c);
			}
			pBlock0->SetValue(vr_translucency_fbCoeff, m_time, volume->attenuation_distance);
			pBlock0->SetValue(vr_translucency_thickness, m_time, volume->thickness_factor);

			pBmpTex = GetBitmapTexFromglTexture(volume->thickness_texture.texture);
			if (pBmpTex) {
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, &volume->thickness_texture);
				Texmap *pTex = BitmapTexToVRayBitmap(pBmpTex, 1.0f);
				pBlock3->SetValue(vr_texmap_translucent, m_time, pTex);
			}
		}

		if (mtl->has_ior) {
			cgltf_ior* ior = &mtl->ior;
			float f = ior->ior;
			pBlock0->SetValue(vr_reflection_ior, m_time, f);
			pBlock0->SetValue(vr_refraction_ior, m_time, f);
		}
		else {
			pBlock0->SetValue(vr_reflection_ior, m_time, 1.5f);
			pBlock0->SetValue(vr_refraction_ior, m_time, 1.5f);
		}

		if (mtl->has_anisotropy) {
			pBlock3->SetValue(vr_texmap_anisotropy_on, m_time, 1);
			pBlock3->SetValue(vr_texmap_anisotropy_rotation_on, m_time, 1);

			cgltf_anisotropy *anisotropy = &mtl->anisotropy;

			cgltf_texture_view* anisoTexInfo = &anisotropy->anisotropy_texture;
			if (anisoTexInfo->texture) {
				pBlock3->SetValue(vr_texmap_anisotropy_on, m_time, 1);
				pBlock3->SetValue(vr_texmap_anisotropy_multiplier, m_time, anisotropy->anisotropy_strength * 75.0f);
				pBmpTex = GetBitmapTexFromglTexture(anisoTexInfo->texture);
				Texmap* pOrgTex = BitmapTexToVRayBitmap(pBmpTex, 1.0f);

				{
					Texmap* pMetalTex = NULL;
					Texmap* pRoughTex = NULL;
					Texmap* pOccTex = NULL;
					SetMetalRoughOccClrCorrectMap(pOrgTex, &pMetalTex, &pRoughTex, &pOccTex, TRUE, TRUE, TRUE);// , & metalRgh->metallic_roughness_texture);

					pBlock3->SetValue(vr_texmap_anisotropy, m_time, pMetalTex);

					Texmap *pGainTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, ColorCorrectTexID);
					IParamBlock2* p1 = pGainTex->GetParamBlock(0);
					p1->SetValue(1, m_time, pOccTex);
					p1->SetValue(11, m_time, 1);
					p1->SetValue(18, m_time, 50.0f);
					p1->SetValue(2, m_time, 2);
					p1->SetValue(3, m_time, 4);
					p1->SetValue(4, m_time, 5);
					p1->SetValue(5, m_time, 6);

					Texmap *pLiftTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, ColorCorrectTexID);
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

					pBlock3->SetValue(vr_texmap_anisotropy_rotation, m_time, pMixTex);
				}

			}
			else {
				pBlock2->SetValue(vr_anisotropy, m_time, anisotropy->anisotropy_strength * 0.75f);

			}
		}
		else {
			pBlock3->SetValue(vr_texmap_anisotropy_on, m_time, 0);
			pBlock3->SetValue(vr_texmap_anisotropy_rotation_on, m_time, 0);
		}

		if (mtl->has_iridescence) {
			cgltf_iridescence* iridescence = &mtl->iridescence;

			if (col[0] == 0.0f && col[1] == 0.0f && col[2] == 0.0f) {
				pBlock0->SetValue(vr_diffuse, m_time, Color(0.01f, 0.01f, 0.01f));
			}
			//pBlock0->SetValue(vr_reflection, m_time, Color(0.0f, 0.0f, 0.0f));
			pBlock2->SetValue(vr_brdf_type, m_time, 4);

			pBlock1->SetValue(vr_thinfilm_on, m_time, 1);
			pBlock1->SetValue(vr_thinfilm_thickness_min, m_time, iridescence->iridescence_thickness_min);
			pBlock1->SetValue(vr_thinfilm_thickness_max, m_time, iridescence->iridescence_thickness_max);
			pBlock1->SetValue(vr_thinfilm_ior, m_time, iridescence->iridescence_ior);

			cgltf_texture_view* texInfo1 = &iridescence->iridescence_texture;
			if (texInfo1->texture) {
				BitmapTex* pBmpTex1 = GetBitmapTexFromglTexture(texInfo1->texture);
				SetTextureUVoffset(pBmpTex1, texInfo1);
				Texmap* pOrgTex = BitmapTexToVRayBitmap(pBmpTex1, 1.0f);
				SetTextureOutputScale(pOrgTex, iridescence->iridescence_factor);

				Texmap* pRTex = NULL;
				SetMetalRoughOccClrCorrectMap(pOrgTex, NULL, NULL, &pRTex, FALSE, FALSE, TRUE);
				pBlock0->SetValue(vr_texmap_thinfilm_ior, m_time, pRTex);
			}
			cgltf_texture_view* texInfo2 = &iridescence->iridescence_thickness_texture;
			if (texInfo2->texture) {
				BitmapTex* pBmpTex2 = GetBitmapTexFromglTexture(texInfo2->texture);
				SetTextureUVoffset(pBmpTex2, texInfo2);
				Texmap* pOrgTex = BitmapTexToVRayBitmap(pBmpTex2, 1.0f);

				Texmap* pGTex = NULL;
				SetMetalRoughOccClrCorrectMap(pOrgTex, NULL, &pGTex, NULL, FALSE, TRUE, FALSE);
				pBlock0->SetValue(vr_texmap_thinFilm_thickness, m_time, pGTex);
			}
			else {
				pBlock1->SetValue(vr_thinfilm_thickness_min, m_time, iridescence->iridescence_thickness_max);
			}

			//pBlock0->SetValue(fm_thin_film_weight, m_time, iridescence->iridescence_factor);
			//pBlock0->SetValue(fm_thin_film_thickness, m_time, iridescence->iridescence_thickness_max);
			//pBlock0->SetValue(fm_thin_film_ior, m_time, iridescence->iridescence_ior);
		}

		AttacheAlphaModeCustAttr(pSmat, mtl->alpha_mode);
		CreateSpecularAttr(pSmat, &mtl->specular, mtl->has_specular);
		CreateDiffuseTransmissionAttr(pSmat, &mtl->diffuse_transmission, mtl->has_diffuse_transmission);

		{
			vrayExtStruct str;
			str.roughness = metalRgh->roughness_factor;
			CreateVRayExtAttr(pSmat, str);
		}

		m_MaterialMap.insert(std::make_pair(mtl, pSmat));
		SetMtlImportStatus(i + 1);
	}
}

//=============================================================================
//=============================================================================
Texmap *glTFImporter_Core::BitmapTexToVRayBitmap(BitmapTex *pBmpTex, float gamma)
{
	if (!pBmpTex) return NULL;

	TimeValue time = 0;

	int mapCh = pBmpTex->GetUVGen()->GetMapChannel();
	float px = pBmpTex->GetUVGen()->GetUOffs(time);
	float py = pBmpTex->GetUVGen()->GetVOffs(time);
	float sx = pBmpTex->GetUVGen()->GetUScl(time);
	float sy = pBmpTex->GetUVGen()->GetVScl(time);
	float rot = pBmpTex->GetUVGen()->GetWAng(time);
	UINT tiling = pBmpTex->GetUVGen()->GetTextureTiling();

	TSTR str = pBmpTex->GetMapName();
	Texmap *pNewTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, VRayBitmapID);
	pNewTex->GetParamBlock(0)->SetValueByName(_T("HDRIMapName"), str, 0);
	pNewTex->GetParamBlock(0)->SetValueByName(_T("color_space"), 3, 0);
	if (gamma != 0.0f) {
		pNewTex->GetParamBlock(0)->SetValueByName(_T("gamma"), gamma, 0);
		pNewTex->GetParamBlock(0)->SetValueByName(_T("color_space"), 0, 0);
	}

	StdUVGen* pUVGen = GetUVGen(pNewTex);
	if (pUVGen) {
		pUVGen->SetMapChannel(mapCh);
		pUVGen->SetUOffs(px, time);
		pUVGen->SetVOffs(py, time);
		pUVGen->SetUScl(sx, time);
		pUVGen->SetVScl(sy, time);
		pUVGen->SetWAng(rot, time);
		pUVGen->SetTextureTiling(tiling);
		pUVGen->SetFlag(U_MIRROR, 0);
		pUVGen->SetFlag(V_MIRROR, 0);
	}

	IParamBlock2* pBlock = NULL;
	GetCustAttrPBlock(pBmpTex, tstring(_T("Webp Encode")), pBlock);
	if (pBlock) {
		tstring str = pBlock->GetStr(2, m_time);
		CreateWebpEncodingAttr(pNewTex, str, str.size() > 0);
	}

	cgltf_texture_view* textureview = m_TextureViewMap[pBmpTex];
	m_TextureViewMap[pNewTex] = textureview;

	return pNewTex;
}
