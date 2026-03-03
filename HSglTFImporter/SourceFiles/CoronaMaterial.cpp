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
void glTFImporter_Core::CreateCoronaMaterial(void)
{
	for (int i = 0; i < m_glTF_data->materials_count; i++) {
		//cgltf_texture *tex;
		//cgltf_image *image;
		BitmapTex *pBmpTex;

		cgltf_material *mtl = &m_glTF_data->materials[i];
		Mtl *pSmat = (Mtl*)GetCOREInterface()->CreateInstance(MATERIAL_CLASS_ID, CoronaMaterialID);
		pSmat->SetName(StringToWString(mtl->name).c_str());
		IParamBlock2 *pBlock0 = pSmat->GetParamBlockByID(0);

		cgltf_pbr_metallic_roughness *metalRgh = &mtl->pbr_metallic_roughness;

		//pBlock0->SetValue(pbr_normal_flip_red, m_time, m_FlipNormalRed);
		//pBlock0->SetValue(pbr_normal_flip_green, m_time, m_FlipNormalGrn);
		//Texmap *pBaseColorMap = CreateBaseColorMap();
		//pBlock0->SetValue(crn_baseTexmap, m_time, pBaseColorMap);
		pBlock0->SetValue(crn_metalnessMode, m_time, 1);
		pBlock0->SetValue(crn_baseRoughness, m_time, 0.0f);
		/*
		if (mtl->has_transmission) 
			pBlock0->SetValue(crn_metalnessMode, m_time, 0);
		else
			pBlock0->SetValue(crn_metalnessMode, m_time, 1);
*/
		BOOL ColorFound = FALSE;
		float *col = metalRgh->base_color_factor;
		if (col)
			if (col[0] < 1.0f || col[1] < 1.0f || col[2] < 1.0f) ColorFound = TRUE;

		Texmap *pBaseColorMap = NULL;
		if (m_UseColorComposite && metalRgh->base_color_texture.texture && ColorFound) {
			pBaseColorMap = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, CoronaMixID);
			pBaseColorMap->GetParamBlock(0)->SetValue(200, m_time, 2);
			pBlock0->SetValue(crn_baseTexmap, m_time, pBaseColorMap);
		}

		if (ColorFound) {
			Color c(col[0], col[1], col[2]);
			if (pBaseColorMap && metalRgh->base_color_texture.texture) {
				Texmap *pColTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, CoronaColorID);
				pColTex->GetParamBlock(0)->SetValue(52, m_time, c);
				pBaseColorMap->GetParamBlock(0)->SetValue(206, m_time, pColTex);

				Control* pClr = (Control*)GetCOREInterface()->CreateInstance(CTRL_POINT3_CLASS_ID, Class_ID(0x2011, 0x0));
				pBlock0->SetControllerByID(crn_baseColor, 0, pClr);
				pColTex->GetParamBlock(0)->SetControllerByID(52, 0, pClr);
			}
			pBlock0->SetValue(crn_baseColor, m_time, c);
		}

		float metal = metalRgh->metallic_factor;
		pBlock0->SetValue(crn_metalnessTexmapOn, m_time, (metal > 0) ? TRUE : FALSE);
		float rough = metalRgh->roughness_factor;
		pBlock0->SetValue(crn_baseRoughnessTexmapOn, m_time, (rough > 0) ? TRUE : FALSE);

		if (metalRgh->base_color_texture.texture) {
			pBmpTex = GetBitmapTexFromglTexture(metalRgh->base_color_texture.texture);
			SetTextureUVoffset(pBmpTex, &metalRgh->base_color_texture);
			Texmap *pTex = BitmapTexToCoronaBitmap(pBmpTex);
			if (pBaseColorMap) {
				pBaseColorMap->GetParamBlock(0)->SetValue(207, m_time, pTex);
			}
			else {
				pBlock0->SetValue(crn_baseTexmap, m_time, pTex);
			}
			if (mtl->alpha_mode == cgltf_alpha_mode_mask || mtl->alpha_mode == cgltf_alpha_mode_blend) {
				/*
				Texmap *pAlphaBmp = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, CoronaBitmapID);
				pAlphaBmp->GetParamBlock(0)->SetValue(101, m_time, pBmpTex->GetMapName());
				pAlphaBmp->GetParamBlock(0)->SetValue(118, m_time, 1);
				BitmapTex *pAlphaBmp = NewDefaultBitmapTex();
				pAlphaBmp->SetName(pBmpTex->GetName() + TSTR(_T("_Alpha")));
				pAlphaBmp->GetUVGen()->SetCoordMapping(UVMAP_SCREEN_ENV);
				pAlphaBmp->GetUVGen()->SetTextureTiling(U_WRAP | V_WRAP);
				pAlphaBmp->GetUVGen()->InitSlotType(MAPSLOT_TEXTURE);
				pAlphaBmp->SetMapName(pBmpTex->GetMapName());
				pAlphaBmp->SetMtlFlag(MTL_TEX_DISPLAY_ENABLED, TRUE);
				pAlphaBmp->SetAlphaAsMono(TRUE);
				*/
				Texmap* pAlphaBmp = CreateAlphaFilterMap(pTex);

				if (mtl->alpha_mode == cgltf_alpha_mode_mask) {
					((IParamBlock2*)pAlphaBmp->GetReference(1))->SetValue(10, 0, 1);
					Texmap *pOSLMap = CreateCutOffOSLNode(pAlphaBmp, mtl->alpha_cutoff);
					if (pOSLMap)
						pBlock0->SetValue(crn_opacityTexmap, m_time, pOSLMap);
				}
				else {
					pBlock0->SetValue(crn_opacityTexmap, m_time, pAlphaBmp);
				}

			}
		}

		BOOL Roughness = FALSE;
		BOOL Metalness = FALSE;
		BOOL Occlusion = FALSE;
		pBlock0->SetValue(crn_baseRoughnessMapAmount, m_time, metalRgh->roughness_factor);
		if (metalRgh->metallic_roughness_texture.texture) {
			pBmpTex = GetBitmapTexFromglTexture(metalRgh->metallic_roughness_texture.texture);
			CorrectBitmapGamma(pBmpTex, 1.0f);
			SetTextureUVoffset(pBmpTex, &metalRgh->metallic_roughness_texture);
			//Bitmap *pOriginalBmp = pBmpTex->GetBitmap(0);
			//BitmapInfo bi = pOriginalBmp->GetBitmapInfo();
			std::filesystem::path fname = pBmpTex->GetMapName();
			//tstring orgFilePath = fname;

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
				Texmap *pMetalTex = NULL;
				Texmap *pRoughTex = NULL;
				Texmap *pOccTex = NULL;
				SetMetalRoughOccOSLMap(pBmpTex, &pMetalTex, &pRoughTex, &pOccTex, Metalness, Roughness, Occlusion, &metalRgh->metallic_roughness_texture);
				pBlock0->SetValue(crn_metalnessTexmap, m_time, pMetalTex);
				pBlock0->SetValue(crn_baseRoughnessTexmap, m_time, pRoughTex);
			}
			else if (m_MapUnpackMode == 2) {
				Texmap* pMetalTex = NULL;
				Texmap* pRoughTex = NULL;
				Texmap* pOccTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, &pMetalTex, &pRoughTex, &pOccTex, Metalness, Roughness, Occlusion);
				pBlock0->SetValue(crn_metalnessTexmap, m_time, pMetalTex);
				pBlock0->SetValue(crn_baseRoughnessTexmap, m_time, pRoughTex);
			}
			else {
				if (Occlusion) {
					BitmapTex *pBmpRTex = SplitOcclusionTexture(pBmpTex);
					SetTextureUVoffset(pBmpRTex, &metalRgh->metallic_roughness_texture);
					Texmap *pTex = BitmapTexToCoronaBitmap(pBmpRTex, 1.0f);
					//pBlock0->SetValue(pbr_ao_map, m_time, pBmpRTex);
				}
				if (Roughness) {
					BitmapTex *pBmpGTex = SplitRoughnessTexture(pBmpTex);
					SetTextureUVoffset(pBmpGTex, &metalRgh->metallic_roughness_texture);
					Texmap *pTex = BitmapTexToCoronaBitmap(pBmpGTex, 1.0f);
					pBlock0->SetValue(crn_baseRoughnessTexmap, m_time, pTex);
				}
				else {
					pBlock0->SetValue(crn_baseRoughness, m_time, 0.0f);
				}
				if (Metalness) {
					BitmapTex *pBmpBTex = SplitMetalnessTexture(pBmpTex);
					SetTextureUVoffset(pBmpBTex, &metalRgh->metallic_roughness_texture);
					Texmap *pTex = BitmapTexToCoronaBitmap(pBmpBTex, 1.0f);
					pBlock0->SetValue(crn_metalnessTexmap, m_time, pTex);
				}
			}
		}

		cgltf_texture_view *nrmTexInfo = &mtl->normal_texture;
		if (nrmTexInfo->texture) {
			pBmpTex = GetBitmapTexFromglTexture(nrmTexInfo->texture);
			CorrectBitmapGamma(pBmpTex, 1.0f);
			SetTextureUVoffset(pBmpTex, nrmTexInfo);
			Texmap *pTex = BitmapTexToCoronaBitmap(pBmpTex, 1.0f);

			Texmap *pNormlMap = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, CoronaNormalMapID);
			pNormlMap->GetParamBlock(0)->SetValue(crn_nrm_normalMap, m_time, pTex);
			pNormlMap->GetParamBlock(0)->SetValue(crn_nrm_multplier, m_time, nrmTexInfo->scale);
			pNormlMap->GetParamBlock(0)->SetValue(crn_nrm_flipRed, m_time, m_FlipNormalRed);
			pNormlMap->GetParamBlock(0)->SetValue(crn_nrm_flipGreen, m_time, m_FlipNormalGrn);
			pBlock0->SetValue(crn_baseBumpTexmap, m_time, pNormlMap);
		}
		/*
				cgltf_texture_view *occTexInfo = &mtl->occlusion_texture;
				if (occTexInfo->texture) {
					pBmpTex = GetBitmapTexFromglTexture(occTexInfo->texture);
					SetTextureUVoffset(pBmpTex, occTexInfo);

					pBmpTex->GetUVGen()->SetTextureTiling(0);
					pBmpTex = SplitOcclusionTexture(pBmpTex);
					//pBlock0->SetValue(pbr_ao_map, m_time, pBmpTex);
				}
		*/

		Color EmissiveColor;
		float* emissive = mtl->emissive_factor;
		if (emissive) {
			EmissiveColor = Color(emissive[0], emissive[1], emissive[2]);
			pBlock0->SetValue(crn_selfIllumColor, m_time, EmissiveColor);
		}

		cgltf_texture_view *emissTexInfo = &mtl->emissive_texture;
		if (emissTexInfo->texture) {
			pBmpTex = GetBitmapTexFromglTexture(emissTexInfo->texture);
			SetTextureUVoffset(pBmpTex, emissTexInfo);
			Texmap *pTex = BitmapTexToCoronaBitmap(pBmpTex);

			Texmap* pColMulTex = CreateColorMultiplyOSLNode(pTex, EmissiveColor);
			pBlock0->SetValue(crn_selfIllumTexmap, m_time, pColMulTex);

			Control* pColorCtrl = (Control*)GetCOREInterface()->CreateInstance(CTRL_POINT4_CLASS_ID, Class_ID(0x2012, 0x0));
			pBlock0->SetControllerByIndex(89, 0, pColorCtrl);
			pColMulTex->GetParamBlock(1)->SetControllerByIndex(1, 0, pColorCtrl);
		}

		if (mtl->has_emissive_strength) {
			cgltf_emissive_strength *strength = &mtl->emissive_strength;
		}

		if (mtl->has_clearcoat) {
			cgltf_clearcoat *clearcoat = &mtl->clearcoat;
			pBlock0->SetValue(crn_clearcoatAmountMapAmount, m_time, clearcoat->clearcoat_factor);
			pBlock0->SetValue(crn_clearcoatRoughness, m_time, clearcoat->clearcoat_roughness_factor);
			pBmpTex = GetBitmapTexFromglTexture(clearcoat->clearcoat_texture.texture);
			if (pBmpTex) {
				CorrectBitmapGamma(pBmpTex, 1.0f);
				Texmap *pTex = BitmapTexToCoronaBitmap(pBmpTex, 1.0f);
				//pBlock0->SetValue(fm_coat_map, m_time, pBmpTex);
				pBlock0->SetValue(crn_clearcoatAmountTexmap, m_time, pTex);
			}
			pBmpTex = GetBitmapTexFromglTexture(clearcoat->clearcoat_roughness_texture.texture);
			if (pBmpTex) {
				CorrectBitmapGamma(pBmpTex, 1.0f);
				Texmap *pTex = BitmapTexToCoronaBitmap(pBmpTex, 1.0f);
				pBlock0->SetValue(crn_clearcoatRoughnessTexmap, m_time, pTex);
			}
			pBmpTex = GetBitmapTexFromglTexture(clearcoat->clearcoat_normal_texture.texture);
			if (pBmpTex) {
				CorrectBitmapGamma(pBmpTex, 1.0f);
				Texmap *pTex = BitmapTexToCoronaBitmap(pBmpTex, 1.0f);
				pBlock0->SetValue(crn_clearcoatBumpTexmap, m_time, pTex);
			}
		}
		if (mtl->has_transmission) {
			cgltf_transmission *transmission = &mtl->transmission;
			float f = transmission->transmission_factor;
			pBlock0->SetValue(crn_refractionAmount, m_time, f);

			pBmpTex = GetBitmapTexFromglTexture(transmission->transmission_texture.texture);
			if (pBmpTex) {
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, &transmission->transmission_texture);
				Texmap *pTex = BitmapTexToCoronaBitmap(pBmpTex, 1.0f);
				pBlock0->SetValue(crn_translucencyColorTexmap, m_time, pTex);
			}

			//pBlock0->SetValue(crn_opacityColor, m_time, Color(0.0f,0.0f,0.0f));
			//pBlock0->SetValue(crn_opacityLevel, m_time, 1.0f - f);
				/*
				if (f > 0.0f) {
//					pBlock0->SetValue(crn_translucencyColorMapAmount, m_time, f);
					pBlock0->SetValue(crn_opacityMapAmount, m_time, 1.0f-f);
					float *col = metalRgh->base_color_factor;
					Color c(col[0], col[1], col[2]);
//					pBlock0->SetValue(crn_translucencyColor, m_time, c);
					pBlock0->SetValue(crn_opacityColor, m_time, c);
				}
				*/
		}
		if (mtl->has_volume) {
			cgltf_volume *volume = &mtl->volume;

/*
			cgltf_texture_view thickness_texture;
			cgltf_float thickness_factor;
			cgltf_float attenuation_color[3];
			cgltf_float attenuation_distance;

#define crn_volumetricAbsorptionColor		261	// color
#define crn_volumetricAbsorptionTexmap		262	// texturemap
#define crn_volumetricAbsorptionTexmapOn	263	// boolean
#define crn_volumetricAbsorptionMapAmount	264	// float
#define crn_volumetricScatteringColor		271	// color
#define crn_volumetricScatteringTexmap		272	// texturemap
#define crn_volumetricScatteringTexmapOn	273	// boolean
#define crn_volumetricScatteringMapAmount	274	// float
*/
		}
		if (mtl->has_ior) {
			cgltf_ior *ior = &mtl->ior;
			pBlock0->SetValue(crn_baseIor, m_time, ior->ior);
		}

		if (mtl->has_specular) {
			cgltf_specular *specular = &mtl->specular;
			pBmpTex = GetBitmapTexFromglTexture(specular->specular_texture.texture);
			SetTextureUVoffset(pBmpTex, &specular->specular_texture);
			Texmap *pTex = BitmapTexToCoronaBitmap(pBmpTex);
			float f = specular->specular_factor;
			//pBlock0->SetValue(fm_transparency_map, m_time, pTex);
		}

		if (mtl->has_sheen) {
			cgltf_sheen* sheen = &mtl->sheen;
			float* col = sheen->sheen_color_factor;
			if (col) {
				Color c(col[0], col[1], col[2]);
				pBlock0->SetValue(crn_sheenColor, m_time, c);
			}
			cgltf_texture_view* sheenTexInfo = &sheen->sheen_color_texture;
			if (sheenTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(sheenTexInfo->texture);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, sheenTexInfo);
				Texmap *pTex = BitmapTexToCoronaBitmap(pBmpTex);
				pBlock0->SetValue(crn_sheenColorTexmap, m_time, pTex);
			}
			pBlock0->SetValue(crn_sheenRoughness, m_time, sheen->sheen_roughness_factor);
			sheenTexInfo = &sheen->sheen_roughness_texture;
			if (sheenTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(sheenTexInfo->texture);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, sheenTexInfo);
				Texmap *pTex = BitmapTexToCoronaBitmap(pBmpTex, 1.0f);
				pBlock0->SetValue(crn_sheenRoughnessTexmap, m_time, pTex);
			}
		}

		if (mtl->has_anisotropy) {
			cgltf_anisotropy* anisotropy = &mtl->anisotropy;

			cgltf_texture_view* anisoTexInfo = &anisotropy->anisotropy_texture;
			if (anisoTexInfo->texture) {
				pBlock0->SetValue(crn_baseAnisoRotation, m_time, anisotropy->anisotropy_rotation);
				pBlock0->SetValue(crn_baseAnisotropy, m_time, anisotropy->anisotropy_strength);
				pBmpTex = GetBitmapTexFromglTexture(anisoTexInfo->texture);
				Texmap* pOrgTex = BitmapTexToCoronaBitmap(pBmpTex, 1.0f);
				{
					Texmap* pMetalTex = NULL;
					Texmap* pRoughTex = NULL;
					Texmap* pOccTex = NULL;
					SetMetalRoughOccClrCorrectMap(pOrgTex, &pMetalTex, &pRoughTex, &pOccTex, TRUE, TRUE, TRUE);// , & metalRgh->metallic_roughness_texture);

					pBlock0->SetValue(crn_baseAnisotropyTexmap, m_time, pMetalTex);

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

					pBlock0->SetValue(crn_baseAnisoRotationTexmap, m_time, pMixTex);
					pBlock0->SetValue(crn_baseAnisoRotationTexmapOn, m_time, 1);
				}

			}
			else {
				pBlock0->SetValue(crn_baseAnisotropy, m_time, anisotropy->anisotropy_strength * 0.75f);
			}
		}

		if (mtl->has_dispersion) {
			cgltf_dispersion* dispersion = &mtl->dispersion;
			//pBlock0->SetValue(an_sf_transmission_dispersion, m_time, dispersion->dispersion);
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
		*/
		float alphaCutoff = mtl->alpha_cutoff;
		bool doubleSided = mtl->double_sided;

		m_MaterialMap.insert(std::make_pair(mtl, pSmat));
		SetMtlImportStatus(i + 1);
	}
}

//=============================================================================
//=============================================================================
Texmap *glTFImporter_Core::BitmapTexToCoronaBitmap(BitmapTex *pBmpTex, float gamma)
{
#define Corona_MapName		101
#define Corona_MapCh		103
#define Corona_MapScale		105
#define Corona_MapOffset	106
#define Corona_MapTileU		108
#define Corona_MapTileU		109
#define Corona_AlphaSource	115

#define Corona_MapGamma		125
#define Corona_MapWAngle	129


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
	Texmap *pNewTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, CoronaBitmapID);
	pNewTex->GetParamBlock(0)->SetValue(Corona_MapName, 0, str);
	pNewTex->GetParamBlock(0)->SetValue(Corona_AlphaSource, 0, 0);

	if (gamma != 0.0f) pNewTex->GetParamBlock(0)->SetValue(Corona_MapGamma, 0, gamma);

	StdUVGen* pUVGen = GetUVGen(pNewTex);
	if (pUVGen) {
		pUVGen->SetMapChannel(mapCh);
		pUVGen->SetUOffs(px, time);
		pUVGen->SetVOffs(py, time);
		pUVGen->SetUScl(sx, time);
		pUVGen->SetVScl(sy, time);
		pUVGen->SetWAng(rot, time);
		pUVGen->SetTextureTiling(tiling);
	}
	else {
		pNewTex->GetParamBlock(0)->SetValue(Corona_MapCh, time, mapCh);
		pNewTex->GetParamBlock(0)->SetValue(Corona_MapScale, time, Point3(sx, sy, 0.0f));
		pNewTex->GetParamBlock(0)->SetValue(Corona_MapOffset, time, Point3(px, py, 0.0f));
		pNewTex->GetParamBlock(0)->SetValue(Corona_MapWAngle, time, DegToRad(rot));
	}

	cgltf_texture_view* textureview = m_TextureViewMap[pBmpTex];
	m_TextureViewMap[pNewTex] = textureview;

	return pNewTex;
}
