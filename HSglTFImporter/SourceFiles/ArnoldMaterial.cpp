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
void glTFImporter_Core::CreateArnoldMaterial(void)
{
	for (int i = 0; i < m_glTF_data->materials_count; i++) {
		//cgltf_texture *tex;
		//cgltf_image *image;
		BitmapTex *pBmpTex;

		cgltf_material *mtl = &m_glTF_data->materials[i];
		Mtl *pSmat = (Mtl*)GetCOREInterface()->CreateInstance(MATERIAL_CLASS_ID, Arnold_StandardSufaceID);
		pSmat->SetName(StringToWString(mtl->name).c_str());
		IParamBlock2 *pBlock0 = pSmat->GetParamBlockByID(0);
		IParamBlock2 *pBlock1 = pSmat->GetParamBlockByID(1);
		IParamBlock2 *pBlock2 = pSmat->GetParamBlockByID(2);

		cgltf_pbr_metallic_roughness *metalRgh = &mtl->pbr_metallic_roughness;

		Control* pBaseColorCtrl = (Control*)GetCOREInterface()->CreateInstance(CTRL_POINT4_CLASS_ID, Class_ID(0x2012, 0x0));
		//pBlock1->SetControllerByIndex(an_sf_base_color, 0, pBaseColorCtrl);

		BOOL ColorFound = FALSE;
		float *col = metalRgh->base_color_factor;
		if (col)
			if (col[0] < 1.0f || col[1] < 1.0f || col[2] < 1.0f) ColorFound = TRUE;

		Texmap *pBaseColorMap = NULL;
		if (m_UseColorComposite && metalRgh->base_color_texture.texture && ColorFound) {
			pBaseColorMap = CreateBaseColorMap();
			pBlock1->SetValue(an_sf_base_color_shader, m_time, pBaseColorMap);
		}

		if (ColorFound) {
			AColor c(col[0], col[1], col[2], col[3]);
			if (pBaseColorMap) {
				Texmap *pColTex = CreateColorMap(c);
				pColTex->GetParamBlock(0)->SetControllerByIndex(0, 0, pBaseColorCtrl);
				pBaseColorMap->GetParamBlock(0)->SetValue(9, m_time, pColTex, 1);
				pBaseColorMap->GetParamBlock(0)->SetValue(5, m_time, 5, 1);
			}
			pBlock1->SetValue(an_sf_base_color, m_time, Color(col[0], col[1], col[2]));
			pBlock1->SetValue(an_sf_transmission_color, m_time, Color(col[0], col[1], col[2]));
		}
		else {
			pBlock1->SetValue(an_sf_base_color, m_time, AColor(1.0f, 1.0f, 1.0f, 1.0f));
		}

		float metal = metalRgh->metallic_factor;
		pBlock1->SetValue(an_sf_metalness, m_time, metal);
		float rough = metalRgh->roughness_factor;
		pBlock1->SetValue(an_sf_diffuse_roughness, m_time, rough);

		if (metalRgh->base_color_texture.texture) {
			pBmpTex = GetBitmapTexFromglTexture(metalRgh->base_color_texture.texture);
			SetTextureUVoffset(pBmpTex, &metalRgh->base_color_texture);
			if (pBaseColorMap) {
				pBaseColorMap->GetParamBlock(0)->SetValue(9, m_time, pBmpTex, 0);
			}
			else {
				pBlock1->SetValue(an_sf_base_color_shader, m_time, pBmpTex);
			}
			pBlock1->SetValue(an_sf_transmission_color_shader, m_time, pBmpTex);

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
				pAlphaBmp->SetAlphaAsRGB(TRUE);
				//pAlphaBmp->SetPremultAlpha(FALSE);
*/
				Texmap* pAlphaBmp = CreateAlphaFilterMap(pBmpTex);

				if (mtl->alpha_mode == cgltf_alpha_mode_mask) {
					Texmap* pCCTex = CreateAlphaFilterMap(NULL, AColor(col));
					IParamBlock2* pBlock = pCCTex->GetParamBlock(0);
					pBlock->SetControllerByID(0, 0, pBaseColorCtrl);

					//((IParamBlock2*)pAlphaBmp->GetReference(1))->SetValue(10, 0, 1);
					Texmap *pOSLMap = CreateCutOffOSLNode(pAlphaBmp, mtl->alpha_cutoff, pCCTex);
					if (pOSLMap)
						pBlock1->SetValue(an_sf_opacity_shader, m_time, pOSLMap);
					else
						pBlock1->SetValue(an_sf_opacity_shader, m_time, pAlphaBmp);

					//Control* pClr = (Control*)GetCOREInterface()->CreateInstance(CTRL_POINT4_CLASS_ID, Class_ID(0x2013, 0x0));
					//pBlock1->SetControllerByID(an_sf_base_color, 0, pClr);
					//pBlock->SetControllerByID(0, 0, pClr);
				}
				else {
					pBlock1->SetValue(an_sf_opacity_shader, m_time, pAlphaBmp);
				}
			}
		}

		BOOL Roughness = FALSE;
		BOOL Metalness = FALSE;
		BOOL Occlusion = FALSE;
		if (metalRgh->metallic_roughness_texture.texture) {
			pBmpTex = GetBitmapTexFromglTexture(metalRgh->metallic_roughness_texture.texture);
			 CorrectBitmapGamma(pBmpTex, 1.0f);
			SetTextureUVoffset(pBmpTex, &metalRgh->metallic_roughness_texture);
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

			if (m_MapUnpackMode == 1) {
				Texmap *pMetalTex = NULL;
				Texmap *pRoughTex = NULL;
				Texmap *pOccTex = NULL;
				SetMetalRoughOccOSLMap(pBmpTex, &pMetalTex, &pRoughTex, &pOccTex, Metalness, Roughness, Occlusion, &metalRgh->metallic_roughness_texture);

				pBlock1->SetValue(an_sf_metalness_shader, m_time, pMetalTex);
				pBlock1->SetValue(an_sf_specular_roughness_shader, m_time, pRoughTex);
				//pBlock0->SetValue(pbr_ao_map, m_time, pOccTex);
			}
			else if (m_MapUnpackMode == 2) {
				Texmap* pMetalTex = NULL;
				Texmap* pRoughTex = NULL;
				Texmap* pOccTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, &pMetalTex, &pRoughTex, &pOccTex, Metalness, Roughness, Occlusion);
				pBlock1->SetValue(an_sf_metalness_shader, m_time, pMetalTex);
				pBlock1->SetValue(an_sf_specular_roughness_shader, m_time, pRoughTex);
			}
			else {
				//if (Occlusion) {
				//	BitmapTex *pBmpRTex = SplitOcclusionTexture(pBmpTex);
				//	SetTextureUVoffset(pBmpTex, &metalRgh->metallic_roughness_texture);
				//	pBlock1->SetValue(pbr_ao_map, m_time, pBmpRTex);
				//}
				pBlock1->SetValue(an_sf_specular, m_time, 1.0f);
				if (Roughness) {
					BitmapTex *pBmpGTex = SplitRoughnessTexture(pBmpTex);
					CorrectBitmapGamma(pBmpGTex, 1.0f);
					SetTextureUVoffset(pBmpGTex, &metalRgh->metallic_roughness_texture);
					pBlock1->SetValue(an_sf_specular_roughness_shader, m_time, pBmpGTex);
					pBlock1->SetValue(an_sf_specular_roughness, m_time, 1.0f);
				}
				if (Metalness) {
					BitmapTex *pBmpBTex = SplitMetalnessTexture(pBmpTex);
					CorrectBitmapGamma(pBmpBTex, 1.0f);
					SetTextureUVoffset(pBmpBTex, &metalRgh->metallic_roughness_texture);
					pBlock1->SetValue(an_sf_metalness_shader, m_time, pBmpBTex);
					pBlock1->SetValue(an_sf_metalness, m_time, 1.0f);
				}
			}
		}

		cgltf_texture_view *nrmTexInfo = &mtl->normal_texture;
		if (nrmTexInfo->texture) {
			pBmpTex = GetBitmapTexFromglTexture(nrmTexInfo->texture);
			CorrectBitmapGamma(pBmpTex, 1.0f);
			SetTextureUVoffset(pBmpTex, nrmTexInfo);

			Texmap *pNormlMap = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, ArnoldNormalMapID);
			pNormlMap->GetParamBlock(1)->SetValue(2, m_time, pBmpTex);
			pNormlMap->GetParamBlock(1)->SetValue(10, m_time, m_FlipNormalRed);
			pNormlMap->GetParamBlock(1)->SetValue(11, m_time, m_FlipNormalGrn);
			pNormlMap->GetParamBlock(1)->SetValue(15, m_time, nrmTexInfo->scale);
			pBlock1->SetValue(an_sf_normal_shader, m_time, pNormlMap);
		}

		Color EmissiveColor;
		float* emissive = mtl->emissive_factor;
		if (emissive) {
			EmissiveColor = Color(emissive[0], emissive[1], emissive[2]);
			pBlock1->SetValue(an_sf_emission_color, m_time, EmissiveColor);
		}

		cgltf_texture_view *emissTexInfo = &mtl->emissive_texture;
		if (emissTexInfo->texture) {
			pBmpTex = GetBitmapTexFromglTexture(emissTexInfo->texture);
			SetTextureUVoffset(pBmpTex, emissTexInfo);

			Texmap* pColMulTex = CreateColorMultiplyOSLNode(pBmpTex, EmissiveColor);
			pBlock1->SetValue(an_sf_emission_color_shader, m_time, pColMulTex);

			Control* pColorCtrl = (Control*)GetCOREInterface()->CreateInstance(CTRL_POINT4_CLASS_ID, Class_ID(0x2012, 0x0));
			pBlock1->SetControllerByIndex(120, 0, pColorCtrl);
			pColMulTex->GetParamBlock(1)->SetControllerByIndex(1, 0, pColorCtrl);

			//pBlock1->SetValue(an_sf_emission_shader, m_time, pBmpTex);
		}


		if (mtl->has_emissive_strength) {
			cgltf_emissive_strength *strength = &mtl->emissive_strength;
			pBlock1->SetValue(an_sf_emission, m_time, strength->emissive_strength);
		}
		else {
			pBlock1->SetValue(an_sf_emission, m_time, 1.0f);
		}

		if (mtl->has_clearcoat) {
			cgltf_clearcoat *clearcoat = &mtl->clearcoat;
			float f = clearcoat->clearcoat_factor;
			pBlock1->SetValue(an_sf_coat, m_time, f);
			float cf = clearcoat->clearcoat_roughness_factor;
			pBlock1->SetValue(an_sf_coat_roughness, m_time, cf);
			pBmpTex = GetBitmapTexFromglTexture(clearcoat->clearcoat_texture.texture);
			if (pBmpTex) {
				SetTextureUVoffset(pBmpTex, &clearcoat->clearcoat_texture);
				Texmap* pRTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, NULL, NULL, &pRTex, FALSE, FALSE, TRUE);

				pBlock1->SetValue(an_sf_coat_color_connected, m_time, 1);
				pBlock1->SetValue(an_sf_coat_color_shader, m_time, pRTex);
			}
			pBmpTex = GetBitmapTexFromglTexture(clearcoat->clearcoat_roughness_texture.texture);
			if (pBmpTex) {
				SetTextureUVoffset(pBmpTex, &clearcoat->clearcoat_roughness_texture);
				Texmap* pGTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, NULL, &pGTex, NULL, FALSE, TRUE, FALSE);

				pBlock1->SetValue(an_sf_coat_roughness_connected, m_time, 1);
				pBlock1->SetValue(an_sf_coat_roughness_shader, m_time, pGTex);
			}
			pBmpTex = GetBitmapTexFromglTexture(clearcoat->clearcoat_normal_texture.texture);
			if (pBmpTex) {
				SetTextureUVoffset(pBmpTex, &clearcoat->clearcoat_normal_texture);
				pBlock1->SetValue(an_sf_coat_normal_connected, m_time, 1);
				pBlock1->SetValue(an_sf_coat_normal_shader, m_time, pBmpTex);
			}
		}

		if (mtl->has_transmission) {
			cgltf_transmission *transmission = &mtl->transmission;
			float f = transmission->transmission_factor;
			pBlock1->SetValue(an_sf_transmission, m_time, f);
			pBmpTex = GetBitmapTexFromglTexture(transmission->transmission_texture.texture);
			if (pBmpTex) {
				if (m_CorrectGamma) CorrectBitmapGamma(pBmpTex, m_GammaValue);
				SetTextureUVoffset(pBmpTex, &transmission->transmission_texture);
				Texmap* pRTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, NULL, NULL, &pRTex, FALSE, FALSE, TRUE);
				pBlock1->SetValue(an_sf_transmission_shader, m_time, pRTex);
			}
			//	Color c(col);
			//	pBlock1->SetValue(an_sf_transmission_color, m_time, c);
		}
		if (mtl->has_volume) {
			cgltf_volume *volume = &mtl->volume;
			float* col = volume->attenuation_color;
			if (col) {
				Color c(col);
				pBlock1->SetValue(an_sf_transmission_color, m_time, c);

			}
			float f = volume->thickness_factor;
			//pBmpTex = GetBitmapTexFromglTexture(volume->thickness_texture.texture);
			//cgltf_float volume->attenuation_distance;
		}

		if (mtl->has_ior) {
			cgltf_ior *ior = &mtl->ior;
			pBlock1->SetValue(an_sf_specular_IOR, m_time, ior->ior);
		}

		if (mtl->has_specular) {
			cgltf_specular* specular = &mtl->specular;

			float f = specular->specular_factor;

			float* col = specular->specular_color_factor;
			if (col) {
				Color c(col[0], col[1], col[2]);
				pBlock1->SetValue(an_sf_specular_color, m_time, c);
			}

			cgltf_texture_view* colTexInfo = &specular->specular_color_texture;
			if (colTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(specular->specular_texture.texture);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, &specular->specular_texture);
				pBlock1->SetValue(an_sf_specular_color_shader, m_time, pBmpTex);
				pBlock1->SetValue(an_sf_specular_color_connected, m_time, 1);

				Texmap* pAlphaTex = NULL;
				SetAlphaClrCorrectMap(pBmpTex, &pAlphaTex);
				pBlock1->SetValue(an_sf_specular_shader, m_time, pAlphaTex);
				pBlock1->SetValue(an_sf_specular_connected, m_time, 1);
			}

		}

		if (mtl->has_iridescence) {
			cgltf_iridescence* iridescence = &mtl->iridescence;
			//float f = iridescence->iridescence_factor;
			pBlock1->SetValue(an_sf_thin_film_thickness, m_time, iridescence->iridescence_thickness_max);
			pBlock1->SetValue(an_sf_thin_film_IOR, m_time, iridescence->iridescence_ior);
			pBmpTex = GetBitmapTexFromglTexture(iridescence->iridescence_thickness_texture.texture);
			if (pBmpTex) {
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, &iridescence->iridescence_thickness_texture);
				Texmap* pGTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, NULL, &pGTex, NULL, FALSE, TRUE, FALSE);

				pBlock1->SetValue(an_sf_thin_film_thickness_connected, m_time, 1);
				pBlock1->SetValue(an_sf_thin_film_thickness_shader, m_time, pGTex);
			}
			pBmpTex = GetBitmapTexFromglTexture(iridescence->iridescence_texture.texture);
			if (pBmpTex) {
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, &iridescence->iridescence_texture);
				Texmap* pRTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, NULL, NULL, &pRTex, FALSE, FALSE, TRUE);

				pBlock1->SetValue(an_sf_thin_film_IOR_connected, m_time, 1);
				pBlock1->SetValue(an_sf_thin_film_IOR_shader, m_time, pRTex);
			}
		}

		if (mtl->has_sheen) {
			cgltf_sheen* sheen = &mtl->sheen;
			
			pBlock1->SetValue(an_sf_sheen, m_time, sheen->sheen_roughness_factor);
			pBlock1->SetValue(an_sf_sheen_color, m_time, Color(sheen->sheen_color_factor));
			pBlock1->SetValue(an_sf_sheen_roughness, m_time, sheen->sheen_roughness_factor);

			cgltf_texture_view* sheenTexInfo = &sheen->sheen_roughness_texture;
			if (sheenTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(sheenTexInfo->texture);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, &sheen->sheen_roughness_texture);

				Texmap* pAlphaTex = NULL;
				SetAlphaClrCorrectMap(pBmpTex, &pAlphaTex);
				pBlock1->SetValue(an_sf_sheen_roughness_connected, m_time, pAlphaTex);
				pBlock1->SetValue(an_sf_sheen_roughness_shader, m_time, pAlphaTex);
			}
			sheenTexInfo = &sheen->sheen_color_texture;
			if (sheenTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(sheenTexInfo->texture);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, &sheen->sheen_color_texture);
				pBlock1->SetValue(an_sf_sheen_color_connected, m_time, pBmpTex);
				pBlock1->SetValue(an_sf_sheen_color_shader, m_time, pBmpTex);
			}
		}

		if (mtl->has_anisotropy) {
			cgltf_anisotropy* anisotropy = &mtl->anisotropy;
			pBlock1->SetValue(an_sf_specular_anisotropy, m_time, anisotropy->anisotropy_strength);
			pBlock1->SetValue(an_sf_specular_rotation, m_time, anisotropy->anisotropy_rotation);
			cgltf_texture_view* anisoTexInfo = &anisotropy->anisotropy_texture;
			pBmpTex = GetBitmapTexFromglTexture(anisoTexInfo->texture);
			if (pBmpTex) {
				SetTextureUVoffset(pBmpTex, anisoTexInfo);

				Texmap* pMetalTex = NULL;
				Texmap* pRoughTex = NULL;
				Texmap* pOccTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, &pMetalTex, &pRoughTex, &pOccTex, TRUE, FALSE, TRUE);// , & metalRgh->metallic_roughness_texture);

				pBlock1->SetValue(an_sf_specular_anisotropy_shader, m_time, pMetalTex);

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

				pBlock1->SetValue(an_sf_specular_rotation_shader, m_time, pMixTex);

			}
		}

		if (mtl->has_diffuse_transmission) {
			cgltf_diffuse_transmission* diff_trans = &mtl->diffuse_transmission;

			pBlock1->SetValue(an_sf_subsurface_type, m_time, 0);

			pBlock1->SetValue(an_sf_subsurface, m_time, diff_trans->diffuseTransmissionFactor);

			float* col = diff_trans->diffuseTransmissionColorFactor;
			if (col) {
				Color c(col[0], col[1], col[2]);
				pBlock1->SetValue(an_sf_subsurface_color, m_time, c);
			}

			cgltf_texture_view* texview = &diff_trans->diffuseTransmissionColorTexture;
			if (texview) {
				pBmpTex = GetBitmapTexFromglTexture(texview->texture);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, texview);
				pBlock1->SetValue(an_sf_subsurface_color_shader, m_time, pBmpTex);
			}
			texview = &diff_trans->diffuseTransmissionTexture;
			if (texview) {
				pBmpTex = GetBitmapTexFromglTexture(texview->texture);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, texview);
				pBlock1->SetValue(an_sf_subsurface_shader, m_time, pBmpTex);
			}
		}

		if (mtl->has_dispersion) {
			cgltf_dispersion* dispersion = &mtl->dispersion;
			pBlock1->SetValue(an_sf_transmission_dispersion, m_time, dispersion->dispersion);
		}

		if (mtl->has_pbr_specular_glossiness) {
			cgltf_pbr_specular_glossiness* spl_gls = &mtl->pbr_specular_glossiness;
			cgltf_texture_view* diffuseTexInfo = &spl_gls->diffuse_texture;
			if (diffuseTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(diffuseTexInfo->texture);
				SetTextureUVoffset(pBmpTex, diffuseTexInfo);
				pBlock1->SetValue(an_sf_base_color_shader, m_time, pBmpTex);
			}
			float* colDiff = spl_gls->diffuse_factor;
			if (colDiff) {
				Color col(colDiff[0], colDiff[1], colDiff[2]);
				pBlock1->SetValue(an_sf_base_color, m_time, col);
			}

			cgltf_texture_view* spglTexInfo = &spl_gls->specular_glossiness_texture;
			if (spglTexInfo->texture) {
				pBlock1->SetValue(an_sf_specular_connected, m_time, 1);
				pBmpTex = GetBitmapTexFromglTexture(spglTexInfo->texture);
				SetTextureUVoffset(pBmpTex, spglTexInfo);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				pBlock1->SetValue(an_sf_specular_shader, m_time, pBmpTex);
			}
			float* colSpec = spl_gls->specular_factor;
			if (colSpec) {
				Color col(colSpec[0], colSpec[1], colSpec[2]);
				pBlock1->SetValue(an_sf_specular_color, m_time, col);
			}
			else {
			}
			float f = spl_gls->glossiness_factor;
			pBlock1->SetValue(an_sf_specular, m_time, f);
		}

		AttachAlphaModeCustAttr(pSmat, mtl->alpha_mode);

		m_MaterialMap.insert(std::make_pair(mtl, pSmat));
		SetMtlImportStatus(i + 1);
	}
}
