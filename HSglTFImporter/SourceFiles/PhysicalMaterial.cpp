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
void glTFImporter_Core::CreatePhysicalMaterial(void)
{
	// ビューレンダラがMetallnesに対応できるようにダミーで1つPBRマテリアルを作る
	//Mtl* pDummyMtl = (Mtl*)GetCOREInterface()->CreateInstance(MATERIAL_CLASS_ID, PBRMetalMtlID);

	for (int i = 0; i < m_glTF_data->materials_count; i++) {
		cgltf_material* mtl = &m_glTF_data->materials[i];
#if MAX_RELEASE <= 22000
		StdMat2* pSmat = (StdMat2*)GetCOREInterface()->CreateInstance(MATERIAL_CLASS_ID, PHYSICALMATERIAL_CLASS_ID);
#else
		StdMat2* pSmat = (StdMat2*)NewPhysicalMaterial();
#endif

		pSmat->SetName(StringToWString(mtl->name).c_str());
		IParamBlock2* pBlock0 = pSmat->GetParamBlockByID(0);
		//const TCHAR *ptr = pBlock->GetLocalName();

		pBlock0->SetValue(fm_base_color, m_time, AColor(1.0f,1.0f,1.0f,1.0f));

		BitmapTex* pBmpTex;

		cgltf_pbr_metallic_roughness* metalRgh = &mtl->pbr_metallic_roughness;

		BOOL ColorFound = FALSE;
		float* col = metalRgh->base_color_factor;
		if (col)
			if (col[0] < 1.0f || col[1] < 1.0f || col[2] < 1.0f) ColorFound = TRUE;

		Texmap* pBaseColorMap = NULL;
		if (m_UseColorComposite && metalRgh->base_color_texture.texture && ColorFound) {
			pBaseColorMap = CreateBaseColorMap();
			pBlock0->SetValue(fm_base_color_map, m_time, pBaseColorMap);
			//Control* pWireC = (Control*)GetCOREInterface()->CreateInstance(CTRL_POINT4_CLASS_ID, Class_ID(0x4697286b, 0x2f7f05ff));
			//pBaseColorMap->GetParamBlock(0)->SetControllerByID(0, 0, pWireC);
		}

		Control* pBaseColorCtrl = (Control*)GetCOREInterface()->CreateInstance(CTRL_POINT4_CLASS_ID, Class_ID(0x2012, 0x0));
		pBlock0->SetControllerByIndex(fm_base_color, 0, pBaseColorCtrl);

		if (ColorFound) {
			AColor c(col[0], col[1], col[2], col[3]);
			//Color c(col[0], col[1], col[2]);
			pBlock0->SetValue(fm_base_color, m_time, c);
			pBlock0->SetValue(fm_trans_color, m_time, c);
			if (pBaseColorMap) {
				Texmap *pColTex = CreateColorMap(c);
				pColTex->GetParamBlock(0)->SetControllerByIndex(0,0, pBaseColorCtrl);
				pBaseColorMap->GetParamBlock(0)->SetValue(9, m_time, pColTex, 1);
				pBaseColorMap->GetParamBlock(0)->SetValue(5, m_time, 5, 1);
				//IParamWireMgr* pWMgr = GetParamWireMgr();
				//pWMgr->Connect(pBlock0, 1, pColTex->GetParamBlock(0), 0, _T("solidcolor"));
			}
		}

		float metal = metalRgh->metallic_factor;
		pBlock0->SetValue(fm_metalness_map_on, m_time, (metal > 0) ? TRUE : FALSE);
		float rough = metalRgh->roughness_factor;
		pBlock0->SetValue(fm_roughness_map_on, m_time, (rough > 0) ? TRUE : FALSE);

		pBlock0->SetValue(fm_roughness, m_time, metalRgh->roughness_factor);
		pBlock0->SetValue(fm_metalness, m_time, metalRgh->metallic_factor);
		if (metalRgh->base_color_texture.texture) {
			pBmpTex = GetBitmapTexFromglTexture(metalRgh->base_color_texture.texture);
			SetTextureUVoffset(pBmpTex, &metalRgh->base_color_texture);
			if (m_UseColorComposite) {
				if (pBaseColorMap) {
					pBaseColorMap->GetParamBlock(0)->SetValue(9, m_time, pBmpTex, 0);
				}
				else {
					pBlock0->SetValue(fm_base_color_map, m_time, pBmpTex);
				}
			}
			else {
				if (ColorFound) {
					pBlock0->SetValue(fm_base_weight_map, m_time, pBmpTex);
					pBlock0->SetValue(fm_reflectivity_map, m_time, pBmpTex);
				}
				else {
					pBlock0->SetValue(fm_base_color_map, m_time, pBmpTex);
				}
				pBlock0->SetValue(fm_trans_color_map, m_time, pBmpTex);
			}
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
					Texmap* pCCTex = CreateAlphaFilterMap(NULL, AColor(col));
					IParamBlock2* pBlock = pCCTex->GetParamBlock(0);
					pBlock->SetControllerByID(0, 0, pBaseColorCtrl);

					Texmap* pOSLMap = CreateCutOffOSLNode(pAlphaBmp, mtl->alpha_cutoff, pCCTex);
					if (pOSLMap)
						pBlock0->SetValue(fm_cutout_map, m_time, pOSLMap);
					else
						pBlock0->SetValue(fm_cutout_map, m_time, pAlphaBmp);

				}
				else {
					//((IParamBlock2*)pAlphaBmp->GetReference(1))->SetValue(10, 0, 1);
					Texmap* pOSLMap = CreateSubtractOSLNode(pAlphaBmp);
					if (pOSLMap)
						pBlock0->SetValue(fm_transparency_map, m_time, pOSLMap);
					else
						pBlock0->SetValue(fm_transparency_map, m_time, pAlphaBmp);
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
			//tstring orgFilePath = pBmpTex->GetMapName();
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
				BitmapTex* p = GetBitmapTexFromglTexture(mtl->occlusion_texture.texture);
				TSTR f1 = p->GetMapName();
				TSTR f2 = pBmpTex->GetMapName();
				if (f1 == f2)Occlusion = TRUE;
			}

			if (m_MapUnpackMode == 1) {
				Texmap* pMetalTex = NULL;
				Texmap* pRoughTex = NULL;
				Texmap* pOccTex = NULL;
				SetMetalRoughOccOSLMap(pBmpTex, &pMetalTex, &pRoughTex, &pOccTex, Metalness, Roughness, Occlusion, &metalRgh->metallic_roughness_texture);
				pBlock0->SetValue(fm_metalness_map, m_time, pMetalTex);
				pBlock0->SetValue(fm_roughness_map, m_time, pRoughTex);
				//pBlock0->SetValue(pbr_ao_map, m_time, pOccTex);
			}
			else if (m_MapUnpackMode == 2) {
				Texmap* pMetalTex = NULL;
				Texmap* pRoughTex = NULL;
				Texmap* pOccTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, &pMetalTex, &pRoughTex, &pOccTex, Metalness, Roughness, Occlusion);
				pBlock0->SetValue(fm_metalness_map, m_time, pMetalTex);
				pBlock0->SetValue(fm_roughness_map, m_time, pRoughTex);
			}
			else {
				if (Occlusion) {
					BitmapTex* pBmpRTex = SplitOcclusionTexture(pBmpTex);
					if (pBmpRTex) {
						CorrectBitmapGamma(pBmpRTex, 1.0f);
						if (&mtl->occlusion_texture) {
							SetTextureUVoffset(pBmpRTex, &mtl->occlusion_texture);
						}
						else {
							SetTextureUVoffset(pBmpRTex, &metalRgh->metallic_roughness_texture);
						}
						//pBlock0->SetValue(pbr_ao_map, m_time, pBmpRTex);
					}
				}
				if (Roughness) {
					BitmapTex* pBmpGTex = SplitRoughnessTexture(pBmpTex);
					if (pBmpGTex) {
						CorrectBitmapGamma(pBmpGTex, 1.0f);
						SetTextureUVoffset(pBmpGTex, &metalRgh->metallic_roughness_texture);
						pBlock0->SetValue(fm_roughness_map, m_time, pBmpGTex);
					}
				}
				if (Metalness) {
					BitmapTex* pBmpBTex = SplitMetalnessTexture(pBmpTex);
					if (pBmpBTex) {
						CorrectBitmapGamma(pBmpBTex, 1.0f);
						SetTextureUVoffset(pBmpBTex, &metalRgh->metallic_roughness_texture);
						pBlock0->SetValue(fm_metalness_map, m_time, pBmpBTex);
					}
				}
			}
		}

		cgltf_texture_view* occTexInfo = &mtl->occlusion_texture;
		if (occTexInfo->texture) {
			pBmpTex = GetBitmapTexFromglTexture(occTexInfo->texture);
			CorrectBitmapGamma(pBmpTex, 1.0f);

			SetTextureUVoffset(pBmpTex, occTexInfo);
			pBmpTex->GetUVGen()->SetTextureTiling(0);		/***********************/
			pBmpTex = SplitOcclusionTexture(pBmpTex);
			//pBlock0->SetValue(pbr_ao_map, m_time, pBmpTex);
		}

		cgltf_texture_view* nrmTexInfo = &mtl->normal_texture;
		if (nrmTexInfo->texture) {
			Texmap* pNormlMap = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, GNORMAL_CLASS_ID);
			pBmpTex = GetBitmapTexFromglTexture(nrmTexInfo->texture);
			CorrectBitmapGamma(pBmpTex, 1.0f);
			//float gm = bi.GetEffectiveGamma();

			SetTextureUVoffset(pBmpTex, nrmTexInfo);
			//pNormlMap->GetParamBlock(0)->SetValue(0, m_time, nrmTexInfo->scale);
			pNormlMap->GetParamBlock(0)->SetValue(2, m_time, pBmpTex);
			pNormlMap->GetParamBlock(0)->SetValue(7, m_time, m_FlipNormalRed);
			pNormlMap->GetParamBlock(0)->SetValue(8, m_time, m_FlipNormalGrn);
			pBlock0->SetValue(fm_bump_map, m_time, pNormlMap);
			pBlock0->SetValue(fm_bump_map_amt, m_time, nrmTexInfo->scale);
		}

		Color EmissiveColor(0.0f, 0.0f, 0.0f);
		float* emissive = mtl->emissive_factor;
		if (emissive) {
			EmissiveColor = Color(emissive[0], emissive[1], emissive[2]);
			pBlock0->SetValue(fm_emit_color, m_time, EmissiveColor);
		}

		cgltf_texture_view* emissTexInfo = &mtl->emissive_texture;
		if (emissTexInfo->texture) {
			pBmpTex = GetBitmapTexFromglTexture(emissTexInfo->texture);
			SetTextureUVoffset(pBmpTex, emissTexInfo);

			Texmap* pColMulTex = CreateColorMultiplyOSLNode(pBmpTex, EmissiveColor);
			pBlock0->SetValue(fm_emit_color_map, m_time, pColMulTex);

			Control* pColorCtrl = (Control*)GetCOREInterface()->CreateInstance(CTRL_POINT4_CLASS_ID, Class_ID(0x2012, 0x0));
			pBlock0->SetControllerByIndex(34, 0, pColorCtrl);
			pColMulTex->GetParamBlock(1)->SetControllerByIndex(1, 0, pColorCtrl);
		}

		if (mtl->has_emissive_strength) {
			cgltf_emissive_strength* strength = &mtl->emissive_strength;
			float lum = strength->emissive_strength * 300.0f;
			pBlock0->SetValue(fm_emit_luminance, m_time, lum);
		}
		else {
			pBlock0->SetValue(fm_emit_luminance, m_time, 0.0f);
		}

		if (mtl->has_clearcoat) {
			cgltf_clearcoat* clearcoat = &mtl->clearcoat;
			pBlock0->SetValueByName(_T("coating"), clearcoat->clearcoat_factor, m_time);
			pBlock0->SetValueByName(_T("coat_roughness"), clearcoat->clearcoat_roughness_factor, m_time);
			pBmpTex = GetBitmapTexFromglTexture(clearcoat->clearcoat_texture.texture);
			if (pBmpTex) {
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, &clearcoat->clearcoat_texture);

				Texmap* pRTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, NULL, NULL, &pRTex, FALSE, FALSE, TRUE);

				//pBlock0->SetValue(fm_coat_map, m_time, pBmpTex);
				pBlock0->SetValueByName(_T("coat_map"), pRTex, m_time);
			}
			pBmpTex = GetBitmapTexFromglTexture(clearcoat->clearcoat_roughness_texture.texture);
			if (pBmpTex) {
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, &clearcoat->clearcoat_roughness_texture);

				Texmap* pGTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, NULL, &pGTex, NULL, FALSE, TRUE, FALSE);
				//pBlock0->SetValue(fm_coat_rough_map, m_time, pBmpTex);
				pBlock0->SetValueByName(_T("coat_rough_map"), pGTex, m_time);
			}
			pBmpTex = GetBitmapTexFromglTexture(clearcoat->clearcoat_normal_texture.texture);
			if (pBmpTex) {
				CorrectBitmapGamma(pBmpTex, 1.0f);
				//pBlock0->SetValue(fm_coat_bump_map, m_time, pBmpTex);
				pBlock0->SetValueByName(_T("coat_bump_map"), pBmpTex, m_time);
			}
		}

		if (mtl->has_transmission) {
			cgltf_transmission* transmission = &mtl->transmission;
			pBmpTex = GetBitmapTexFromglTexture(transmission->transmission_texture.texture);
			if (pBmpTex) {
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, &transmission->transmission_texture);
				Texmap* pRTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, NULL, NULL, &pRTex, FALSE, FALSE, TRUE);
				pBlock0->SetValue(fm_transparency_map, m_time, pRTex);
			}
			else {
				float f = transmission->transmission_factor;
				if (f > 0.0f) {
					//TSTR ComStr;
					//ComStr.printf(_T("sceneMaterials[\"%s\"].transparency=%f"), pSmat->GetName().data(), f);
					//ExecuteMAXScriptScript(ComStr, MAXScript::ScriptSource::NonEmbedded);
					//pBlock0->SetValue(fm_transparency, m_time, f);
					pBlock0->SetValueByName(_T("transparency"), f, m_time);
					//float* col = metalRgh->base_color_factor;
					//Color c(col[0], col[1], col[2]);
					//pBlock0->SetValueByName(_T("trans_color"), c, m_time);
				}
			}
		}

		if (mtl->has_ior) {
			cgltf_ior* ior = &mtl->ior;
			pBlock0->SetValue(fm_trans_ior, m_time, ior->ior);
		}
		else {
		}

		if (mtl->has_specular) {
		}

		if (mtl->has_bump) {
			cgltf_material_bump* bump = &mtl->material_bump;
			cgltf_texture_view* bumpTexInfo = &bump->bumpTexture;
			if (bumpTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(bumpTexInfo->texture);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, bumpTexInfo);
				pBlock0->SetValue(fm_bump_map, m_time, pBmpTex);
				pBlock0->SetValue(fm_bump_map_amt, m_time, bump->bump_factor*10.0f);
			}
		}

		if (mtl->has_anisotropy) {
			cgltf_anisotropy* anisotropy = &mtl->anisotropy;

			pBlock0->SetValue(fm_anisotropy, m_time, anisotropy->anisotropy_strength);
			pBlock0->SetValue(fm_anisoangle, m_time, anisotropy->anisotropy_rotation);
			pBmpTex = GetBitmapTexFromglTexture(anisotropy->anisotropy_texture.texture);
			if (pBmpTex) {
				SetTextureUVoffset(pBmpTex, &anisotropy->anisotropy_texture);

				Texmap* pMetalTex = NULL;
				Texmap* pRoughTex = NULL;
				Texmap* pOccTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, &pMetalTex, &pRoughTex, &pOccTex, TRUE, FALSE, TRUE);// , & metalRgh->metallic_roughness_texture);

				pBlock0->SetValue(fm_anisotropy_map, m_time, pMetalTex);

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

				pBlock0->SetValue(fm_aniso_angle_map, m_time, pMixTex);

			}
		}
		else {
			pBlock0->SetValue(fm_anisotropy, m_time, 0.0f);
			pBlock0->SetValue(fm_anisoangle, m_time, 0.0f);
		}

#if MAX_RELEASE>=25000

		if (mtl->has_dispersion) {
			cgltf_dispersion* dispersion = &mtl->dispersion;

			pBlock0->SetValue(fm_dispersion, m_time, dispersion->dispersion);
		}

		if (mtl->has_sheen) {
			cgltf_sheen* sheen = &mtl->sheen;
			float* col = sheen->sheen_color_factor;
			if (col) {
				Color c(col[0], col[1], col[2]);
				pBlock0->SetValue(fm_sheen_color, m_time, c);
			}
			cgltf_texture_view* sheenTexInfo = &sheen->sheen_color_texture;
			if (sheenTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(sheenTexInfo->texture);
				SetTextureUVoffset(pBmpTex, sheenTexInfo);
				pBlock0->SetValue(fm_sheen_color_map, m_time, pBmpTex);
			}
			pBlock0->SetValue(fm_sheen_roughness, m_time, sheen->sheen_roughness_factor);
			sheenTexInfo = &sheen->sheen_roughness_texture;
			if (sheenTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(sheenTexInfo->texture);
				SetTextureUVoffset(pBmpTex, sheenTexInfo);

				Texmap* pAlphaTex = NULL;
				SetAlphaClrCorrectMap(pBmpTex, &pAlphaTex);
				pBlock0->SetValue(fm_sheen_rough_map, m_time, pAlphaTex);
				pBlock0->SetValue(fm_sheen_roughness, m_time, sheen->sheen_roughness_factor);
			}
		}
		else {
			pBlock0->SetValue(fm_sheen_color, m_time, Color(0.0f, 0.0f, 0.0f));
			pBlock0->SetValue(fm_sheen_roughness, m_time, 0.0f);
		}

		if (mtl->has_pbr_specular_glossiness) {
			cgltf_pbr_specular_glossiness* spl_gls = &mtl->pbr_specular_glossiness;
			cgltf_texture_view* diffuseTexInfo = &spl_gls->diffuse_texture;
			BitmapTex* pBmpTex1 = NULL;
			if (diffuseTexInfo->texture) {
				pBmpTex1 = GetBitmapTexFromglTexture(diffuseTexInfo->texture);
				SetTextureUVoffset(pBmpTex1, diffuseTexInfo);
				pBlock0->SetValue(fm_base_color_map, m_time, pBmpTex1);
			}
			float* colDiff = spl_gls->diffuse_factor;
			if (colDiff) {
				Color c(colDiff[0], colDiff[1], colDiff[2]);
				pBlock0->SetValue(fm_base_color, m_time, c);
			}
			cgltf_texture_view* spglTexInfo = &spl_gls->specular_glossiness_texture;
			if (spglTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(spglTexInfo->texture);
				SetTextureUVoffset(pBmpTex, spglTexInfo);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				Texmap* pTex = CreateSpecGloddFilterOSLNode(pBmpTex1, pBmpTex);
				pBlock0->SetValue(fm_base_color_map, m_time, pTex);
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

					pBlock0->SetValue(fm_roughness_map_on, m_time, TRUE);
					pBlock0->SetValue(fm_roughness_map, m_time, pAlphaBmp);
				}
			}
		}

#endif
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
		*/

		//float alphaCutoff = mtl->alpha_cutoff;
		//bool doubleSided = mtl->double_sided;


		AttacheAlphaModeCustAttr(pSmat, mtl->alpha_mode);

		CreateVolumeAttr(pSmat, &mtl->volume, mtl->has_volume);

		//CreateIridescenceAttr(pSmat, &mtl->iridescence, mtl->has_iridescence);
		if (mtl->has_iridescence) {
			cgltf_iridescence* iridescence = &mtl->iridescence;

			cgltf_texture_view* texInfo1 = &iridescence->iridescence_texture;
			if (texInfo1->texture) {
				BitmapTex* pBmpTex1 = GetBitmapTexFromglTexture(texInfo1->texture);
				SetTextureUVoffset(pBmpTex1, texInfo1);
				Texmap* pRTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex1, NULL, NULL, &pRTex, FALSE, FALSE, TRUE);
				pBlock0->SetValue(fm_thin_film_map, m_time, pRTex);
			}
			cgltf_texture_view* texInfo2 = &iridescence->iridescence_thickness_texture;
			if (texInfo2->texture) {
				BitmapTex* pBmpTex2 = GetBitmapTexFromglTexture(texInfo2->texture);
				SetTextureUVoffset(pBmpTex2, texInfo2);
				Texmap* pGTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex2, NULL, &pGTex, NULL, FALSE, TRUE, FALSE);
				pBlock0->SetValue(fm_thin_film_ior_map, m_time, pGTex);
			}

			pBlock0->SetValue(fm_thin_film_weight, m_time, iridescence->iridescence_factor);
			pBlock0->SetValue(fm_thin_film_thickness, m_time, iridescence->iridescence_thickness_max);
			pBlock0->SetValue(fm_thin_film_ior, m_time, iridescence->iridescence_ior);
		}
		else {
			pBlock0->SetValue(fm_thin_film_thickness, m_time, 400.0f);
		}

		//CreateSheenAttr(pSmat, &mtl->sheen, mtl->has_sheen);
		//CreateClearcoatAttr(pSmat, &mtl->clearcoat, mtl->has_clearcoat);
		CreateUnlitAttr(pSmat, mtl->unlit);
		CreateSpecularAttr(pSmat, &mtl->specular, mtl->has_specular);
		CreateDiffuseTransmissionAttr(pSmat, &mtl->diffuse_transmission, mtl->has_diffuse_transmission);

		m_MaterialMap.insert(std::make_pair(mtl, pSmat));
		SetMtlImportStatus(i + 1);
	}
}
