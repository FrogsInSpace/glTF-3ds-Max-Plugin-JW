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

//=============================================================================
//=============================================================================
void glTFImporter_Core::CreateglTFMaterial(void)
{
	// Create a dummy PBR material so that the view renderer can support Metallnes.
	//Mtl* pDummyMtl = (Mtl*)GetCOREInterface()->CreateInstance(MATERIAL_CLASS_ID, PBRMetalMtlID);

	for (int i = 0; i < m_glTF_data->materials_count; i++) {
		cgltf_material* mtl = &m_glTF_data->materials[i];
		Mtl* pSmat = (Mtl*)GetCOREInterface()->CreateInstance(MATERIAL_CLASS_ID, glTFMaterialID);
		pSmat->SetName(StringToWString(mtl->name).c_str());
		IParamBlock2* pBlock0 = pSmat->GetParamBlockByID(0);
		IParamBlock2* pBlock1 = pSmat->GetParamBlockByID(1);
		//const TCHAR *ptr = pBlock->GetLocalName();

		//cgltf_texture* tex;
		//cgltf_image* image;
		BitmapTex* pBmpTex;

		cgltf_pbr_metallic_roughness* metalRgh = &mtl->pbr_metallic_roughness;

		//pBlock0->SetValue(pbr_normal_flip_red, m_time, m_FlipNormalRed);
		//pBlock0->SetValue(pbr_normal_flip_green, m_time, m_FlipNormalGrn);

		float metal = metalRgh->metallic_factor;
		pBlock0->SetValue(glTF_metalness, m_time, metal);
		float rough = metalRgh->roughness_factor;
		pBlock0->SetValue(glTF_roughness, m_time, rough);

		float* col = metalRgh->base_color_factor;
		if (col) {
			if ((col[0] == 0.0f) && (col[1] == 0.0f) && (col[2] == 0.0f) && metalRgh->base_color_texture.texture) col[2] = 0.00001f;
			AColor c(col[0], col[1], col[2], col[3]);
			pBlock0->SetValue(glTF_baseColor, m_time, c);
		}

		pBlock0->SetValue(glTF_alphaMode, m_time, mtl->alpha_mode + 1);

		if (metalRgh->base_color_texture.texture) {
			pBmpTex = GetBitmapTexFromglTexture(metalRgh->base_color_texture.texture);
			SetTextureUVoffset(pBmpTex, &metalRgh->base_color_texture);
			pBlock0->SetValue(glTF_baseColorMap, m_time, pBmpTex);

			if (mtl->alpha_mode == cgltf_alpha_mode_mask || mtl->alpha_mode == cgltf_alpha_mode_blend) {
				Texmap* pAlphaTex = CreateAlphaFilterMap(pBmpTex);
				pBlock0->SetValue(glTF_alphaMap, m_time, pAlphaTex);

				float f = mtl->alpha_cutoff;
				pBlock0->SetValue(glTF_alphaCutoff, m_time, f);
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
				pBlock0->SetValue(glTF_metalnessMap, m_time, pMetalTex);
				pBlock0->SetValue(glTF_roughnessMap, m_time, pRoughTex);
				SetTextureUVoffset(pOccTex, &mtl->occlusion_texture);
				pBlock0->SetValue(glTF_ambientOcclusionMap, m_time, pOccTex);
			}
			else if (m_MapUnpackMode == 2) {
				Texmap* pMetalTex = NULL;
				Texmap* pRoughTex = NULL;
				Texmap* pOccTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, &pMetalTex, &pRoughTex, &pOccTex, Metalness, Roughness, Occlusion);
				SetTextureUVoffset(pMetalTex, &metalRgh->metallic_roughness_texture);
				pBlock0->SetValue(glTF_metalnessMap, m_time, pMetalTex);
				SetTextureUVoffset(pRoughTex, &metalRgh->metallic_roughness_texture);
				pBlock0->SetValue(glTF_roughnessMap, m_time, pRoughTex);
				SetTextureUVoffset(pOccTex, &mtl->occlusion_texture);
				pBlock0->SetValue(glTF_ambientOcclusionMap, m_time, pOccTex);
			}
			else{
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
						pBlock0->SetValue(glTF_ambientOcclusionMap, m_time, pBmpRTex);
					}
				}
				if (Roughness) {
					BitmapTex* pBmpGTex = SplitRoughnessTexture(pBmpTex);
					if (pBmpGTex) {
						CorrectBitmapGamma(pBmpGTex, 1.0f);
						SetTextureUVoffset(pBmpGTex, &metalRgh->metallic_roughness_texture);
						pBlock0->SetValue(glTF_roughnessMap, m_time, pBmpGTex);
					}
				}
				if (Metalness) {
					BitmapTex* pBmpBTex = SplitMetalnessTexture(pBmpTex);
					if (pBmpBTex) {
						CorrectBitmapGamma(pBmpBTex, 1.0f);
						SetTextureUVoffset(pBmpBTex, &metalRgh->metallic_roughness_texture);
						pBlock0->SetValue(glTF_metalnessMap, m_time, pBmpBTex);
					}
				}
			}
		}

		cgltf_texture_view* occTexInfo = &mtl->occlusion_texture;
		if (occTexInfo->texture && Occlusion == FALSE) {
			pBmpTex = GetBitmapTexFromglTexture(occTexInfo->texture);
			CorrectBitmapGamma(pBmpTex, 1.0f);
			SetTextureUVoffset(pBmpTex, occTexInfo);
			pBmpTex->GetUVGen()->SetTextureTiling(0);		/***********************/
			if (m_MapUnpackMode == 1) {
				Texmap* pMetalTex = NULL;
				Texmap* pRoughTex = NULL;
				Texmap* pOccTex = NULL;
				SetMetalRoughOccOSLMap(pBmpTex, &pMetalTex, &pRoughTex, &pOccTex, FALSE, FALSE, TRUE, occTexInfo);
				pBlock0->SetValue(glTF_ambientOcclusionMap, m_time, pOccTex);
			}
			else if (m_MapUnpackMode == 2) {
				Texmap* pMetalTex = NULL;
				Texmap* pRoughTex = NULL;
				Texmap* pOccTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, &pMetalTex, &pRoughTex, &pOccTex, FALSE, FALSE, TRUE);
				pBlock0->SetValue(glTF_ambientOcclusionMap, m_time, pOccTex);
			}
			else {
				pBmpTex = SplitOcclusionTexture(pBmpTex);
				pBlock0->SetValue(glTF_ambientOcclusionMap, m_time, pBmpTex);
			}
			pBlock0->SetValue(glTF_ambientOcclusion, m_time, occTexInfo->scale);
		}

		cgltf_texture_view* nrmTexInfo = &mtl->normal_texture;
		if (nrmTexInfo->texture) {
			pBmpTex = GetBitmapTexFromglTexture(nrmTexInfo->texture);
			CorrectBitmapGamma(pBmpTex, 1.0f);
			SetTextureUVoffset(pBmpTex, nrmTexInfo);
#if 0	// Use OSL map or not
			Texmap* pFlipNormalTex = CreateFlipNormalOSLNode(pBmpTex, m_FlipNormalGrn, m_FlipNormalRed);
			pBlock0->SetValue(glTF_normalMap, m_time, pFlipNormalTex);
#else
			pBlock0->SetValue(glTF_normalMap, m_time, pBmpTex);
#endif
			pBlock0->SetValue(glTF_normal, m_time, nrmTexInfo->scale);

		}

		cgltf_texture_view* emissTexInfo = &mtl->emissive_texture;
		if (emissTexInfo->texture) {
			pBmpTex = GetBitmapTexFromglTexture(emissTexInfo->texture);
			SetTextureUVoffset(pBmpTex, emissTexInfo);
			pBlock0->SetValue(glTF_emissionMap, m_time, pBmpTex);
		}

		float* emissive = mtl->emissive_factor;
		if (emissive) {
			Color c(emissive[0], emissive[1], emissive[2]);
			pBlock0->SetValue(glTF_emissionColor, m_time, c);
		}

		if (mtl->has_emissive_strength) {
			cgltf_emissive_strength *strength = &mtl->emissive_strength;
		}

		if (mtl->has_transmission) {
			pBlock1->SetValue(glTF_enableTransmission, m_time, TRUE);
			cgltf_transmission* transmission = &mtl->transmission;
			float f = transmission->transmission_factor;
			pBlock1->SetValue(glTF_transmission, m_time, f);
			pBmpTex = GetBitmapTexFromglTexture(transmission->transmission_texture.texture);
			if (pBmpTex) {
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, &transmission->transmission_texture);
				Texmap* pRTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, NULL, NULL, &pRTex, FALSE, FALSE, TRUE);
				pBlock1->SetValue(glTF_transmissionMap, m_time, pRTex);
			}
			pBlock1->SetValue(glTF_enableVolume, m_time, mtl->has_volume);
		}


		if (mtl->has_clearcoat) {
			pBlock1->SetValue(glTF_enableClearcoat, m_time, TRUE);
			cgltf_clearcoat* clearCoat = &mtl->clearcoat;
			float f = clearCoat->clearcoat_factor;
			pBlock1->SetValue(glTF_clearcoat, m_time, f);
			cgltf_texture_view* clrTexInfo = &clearCoat->clearcoat_texture;
			if (clrTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(clrTexInfo->texture);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, clrTexInfo);
				Texmap* pRTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, NULL, NULL, &pRTex, FALSE, FALSE, TRUE);
				pBlock1->SetValue(glTF_clearcoatMap, m_time, pRTex);
			}
			cgltf_texture_view* clrNrmTexInfo = &clearCoat->clearcoat_normal_texture;
			if (clrNrmTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(clrNrmTexInfo->texture);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, clrNrmTexInfo);
				pBlock1->SetValue(glTF_clearcoatNormalMap, m_time, pBmpTex);
				pBlock1->SetValue(glTF_clearcoatNormal, m_time, clrNrmTexInfo->scale);
			}
			cgltf_texture_view* clrRghTexInfo = &clearCoat->clearcoat_roughness_texture;
			if (clrRghTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(clrRghTexInfo->texture);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, clrRghTexInfo);
				Texmap* pGTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, NULL, &pGTex, NULL, FALSE, TRUE, FALSE);
				pBlock1->SetValue(glTF_clearcoatRoughnessMap, m_time, pGTex);
				pBlock1->SetValue(glTF_clearcoatRoughness, m_time, clrRghTexInfo->scale);
			}
			f = clearCoat->clearcoat_roughness_factor;
			pBlock1->SetValue(glTF_clearcoatRoughness, m_time, f);
		}

		if (mtl->has_sheen) {
			pBlock1->SetValue(glTF_enableSheen, m_time, TRUE);
			cgltf_sheen *sheen = &mtl->sheen;
			float* col = sheen->sheen_color_factor;
			if (col) {
				Color c(col[0], col[1], col[2]);
				pBlock1->SetValue(glTF_sheenColor, m_time, c);
			}
			cgltf_texture_view* sheenTexInfo = &sheen->sheen_color_texture;
			if (sheenTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(sheenTexInfo->texture);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, sheenTexInfo);
				pBlock1->SetValue(glTF_sheenColorMap, m_time, pBmpTex);
			}
			pBlock1->SetValue(glTF_sheenRoughness, m_time, sheen->sheen_roughness_factor);
			sheenTexInfo = &sheen->sheen_roughness_texture;
			if (sheenTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(sheenTexInfo->texture);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, sheenTexInfo);

				Texmap* pAlphaTex = NULL;
				SetAlphaClrCorrectMap(pBmpTex, &pAlphaTex);
				pBlock1->SetValue(glTF_sheenRoughnessMap, m_time, pAlphaTex);
			}
		}

		if (FALSE) {	//mtl->has_thinfilm
		}

		if (mtl->has_volume) {
			pBlock1->SetValue(glTF_enableVolume, m_time, TRUE);
			cgltf_volume* volume = &mtl->volume;
			float* col = volume->attenuation_color;
			if (col) {
				Color c(col[0], col[1], col[2]);
				pBlock1->SetValue(glTF_volumeColor, m_time, c);
			}
			pBlock1->SetValue(glTF_volumeDistance, m_time, volume->attenuation_distance);
			pBlock1->SetValue(glTF_volumeThickness, m_time, volume->thickness_factor);
			cgltf_texture_view* thickTexInfo = &volume->thickness_texture;
			if (thickTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(thickTexInfo->texture);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, thickTexInfo);
				Texmap* pGTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, NULL, &pGTex, NULL, FALSE, TRUE, FALSE);
				pBlock1->SetValue(glTF_volumeThicknessMap, m_time, pGTex);
			}
		}

		if (mtl->has_ior) {
			pBlock1->SetValue(glTF_enableIndexOfRefraction, m_time, TRUE);
			cgltf_ior* ior = &mtl->ior;
			float f = ior->ior;
			pBlock1->SetValue(glTF_indexOfRefraction, m_time, f);
		}


		if (mtl->has_specular) {
			pBlock1->SetValue(glTF_unlit, m_time, FALSE);

			cgltf_specular* specular = &mtl->specular;

			pBlock1->SetValue(glTF_enableSpecular, m_time, 1);
			pBlock1->SetValue(glTF_specular, m_time, specular->specular_factor);

			float* col = specular->specular_color_factor;
			if (col) {
				Color c(col[0], col[1], col[2]);
				pBlock1->SetValue(glTF_specularcolor, m_time, c);
			}

			cgltf_texture_view* colTexInfo = &specular->specular_color_texture;
			if (colTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(colTexInfo->texture);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				pBlock1->SetValue(glTF_specularColorMap, m_time, pBmpTex);
				Texmap* pAlphaTex = NULL;
				SetAlphaClrCorrectMap(pBmpTex, &pAlphaTex);
				pBlock1->SetValue(glTF_specularMap, m_time, pAlphaTex);
			}

			cgltf_texture_view* specTexInfo = &specular->specular_texture;
			if (specTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(specTexInfo->texture);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				pBlock1->SetValue(glTF_specularMap, m_time, pBmpTex);
			}
		}

		if (mtl->has_pbr_specular_glossiness) {
			cgltf_pbr_specular_glossiness* spl_gls = &mtl->pbr_specular_glossiness;
			cgltf_texture_view* diffuseTexInfo = &spl_gls->diffuse_texture;
			if (diffuseTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(diffuseTexInfo->texture);
				SetTextureUVoffset(pBmpTex, diffuseTexInfo);
				pBlock0->SetValue(glTF_baseColorMap, m_time, pBmpTex);
			}
			float* colDiff = spl_gls->diffuse_factor;
			if (colDiff) {
				Color c(colDiff[0], colDiff[1], colDiff[2]);
				pBlock1->SetValue(glTF_baseColor, m_time, c);
			}

			cgltf_texture_view* spglTexInfo = &spl_gls->specular_glossiness_texture;
			if (spglTexInfo->texture) {
				pBlock1->SetValue(glTF_enableSpecular, m_time, 1);
				pBmpTex = GetBitmapTexFromglTexture(spglTexInfo->texture);
				SetTextureUVoffset(pBmpTex, spglTexInfo);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				pBlock1->SetValue(glTF_specularMap, m_time, pBmpTex);
			}
			float* colSpec = spl_gls->specular_factor;
			if (colSpec) {
				Color col(colSpec[0], colSpec[1], colSpec[2]);
				pBlock1->SetValue(glTF_specularcolor, m_time, col);
			}
			else {
			}
			float f = spl_gls->glossiness_factor;
			pBlock1->SetValue(glTF_specular, m_time, f);
		}

		pBlock1->SetValue(glTF_unlit, m_time, mtl->unlit);
		pBlock0->SetValue(glTF_Doublesided, m_time, mtl->double_sided);

		cgltf_extension* ext = mtl->extensions;
		for (int cnt = 0; cnt < mtl->extensions_count; cnt++, ext++) {
			char* name = ext->name;
			char* data = ext->data;
		}

		//gltf2::Extension ext = mtl.extensions();
		for (int j = 0; j < mtl->extensions_count; j++) {
			cgltf_extension* ext = &mtl->extensions[j];
		}
		float alphaCutoff = mtl->alpha_cutoff;
		bool doubleSided = mtl->double_sided;

		if(mtl->has_iridescence)
			CreateIridescenceAttr(pSmat, &mtl->iridescence, mtl->has_iridescence);
		if(mtl->has_emissive_strength)
			CreateEmissiveStrengthAttr(pSmat, &mtl->emissive_strength, mtl->has_emissive_strength);
		if(mtl->has_anisotropy)
			CreateAnisotropyAttr(pSmat, &mtl->anisotropy, mtl->has_anisotropy);
		if(mtl->has_dispersion)
			CreateDispersionAttr(pSmat, &mtl->dispersion, mtl->has_dispersion);
		if(mtl->has_diffuse_transmission)
			CreateDiffuseTransmissionAttr(pSmat, &mtl->diffuse_transmission, mtl->has_diffuse_transmission);

		m_MaterialMap.insert(std::make_pair(mtl, pSmat));
		SetMtlImportStatus(i + 1);
	}
}
