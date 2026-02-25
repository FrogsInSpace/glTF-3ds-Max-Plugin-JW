
#include "HSglTFImporter.h"
#include "define.h"


//=============================================================================
//=============================================================================
void glTFImporter_Core::CreateUSDMaterial(void)
{
	for (int i = 0; i < m_glTF_data->materials_count; i++) {
		cgltf_material *mtl = &m_glTF_data->materials[i];
		Mtl *pSmat = (Mtl*)GetCOREInterface()->CreateInstance(MATERIAL_CLASS_ID, USDMaterialID);
		pSmat->SetName(StringToWString(mtl->name).c_str());
		IParamBlock2 *pBlock0 = pSmat->GetParamBlockByID(0);
		IParamBlock2 *pBlock1 = pSmat->GetParamBlockByID(1);
		//const TCHAR *ptr = pBlock->GetLocalName();

		BitmapTex *pBmpTex;

		cgltf_pbr_metallic_roughness *metalRgh = &mtl->pbr_metallic_roughness;

		BOOL ColorFound = FALSE;
		float *col = metalRgh->base_color_factor;
		if (col)
			if (col[0] < 1.0f || col[1] < 1.0f || col[2] < 1.0f) ColorFound = TRUE;

		Texmap *pBaseColorMap = NULL;
		if (m_UseColorComposite && metalRgh->base_color_texture.texture && ColorFound) {
			pBaseColorMap = CreateBaseColorMap();
			pBlock1->SetValue(usd_diffuseColor_map, m_time, pBaseColorMap);
		}

		if (ColorFound) {
			//AColor c(col[0], col[1], col[2], col[3]);
			Color c(col[0], col[1], col[2]);
			if (pBaseColorMap) {
				Texmap *pColTex = CreateColorMap(AColor(c));
				pBaseColorMap->GetParamBlock(0)->SetValue(9, m_time, pColTex, 1);
				pBaseColorMap->GetParamBlock(0)->SetValue(5, m_time, 5, 1);
			}
			pBlock1->SetValue(usd_diffuseColor, m_time, c);
		}

		pBlock0->SetValue(usd_normal_flip_red, m_time, m_FlipNormalRed);
		pBlock0->SetValue(usd_normal_flip_green, m_time, m_FlipNormalGrn);

		float metal = metalRgh->metallic_factor;
		pBlock1->SetValue(usd_metallic, m_time, metal);
		float rough = metalRgh->roughness_factor;
		pBlock1->SetValue(usd_roughness, m_time, rough);

		if (metalRgh->base_color_texture.texture) {
			pBmpTex = GetBitmapTexFromglTexture(metalRgh->base_color_texture.texture);
			SetTextureUVoffset(pBmpTex, &metalRgh->base_color_texture);
			if (pBaseColorMap) {
				pBaseColorMap->GetParamBlock(0)->SetValue(9, m_time, pBmpTex, 0);
			}
			else {
				pBlock1->SetValue(usd_diffuseColor_map, m_time, pBmpTex);
			}

			if (mtl->alpha_mode == cgltf_alpha_mode_mask || mtl->alpha_mode == cgltf_alpha_mode_blend) {
				Texmap* pAlphaBmp = CreateAlphaFilterMap(pBmpTex);
				pBlock1->SetValue(usd_opacity_map, m_time, pAlphaBmp);

				if (mtl->alpha_mode == cgltf_alpha_mode_mask) {
					float alphaCutoff = mtl->alpha_cutoff;
					pBlock1->SetValue(usd_opacityThreshold, m_time, alphaCutoff);
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
				pBlock1->SetValue(usd_metallic_map, m_time, pMetalTex);
				pBlock1->SetValue(usd_roughness_map, m_time, pRoughTex);
				pBlock1->SetValue(usd_occlusion_map, m_time, pOccTex);
			}
			else if (m_MapUnpackMode == 2) {
				Texmap* pMetalTex = NULL;
				Texmap* pRoughTex = NULL;
				Texmap* pOccTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, &pMetalTex, &pRoughTex, &pOccTex, Metalness, Roughness, Occlusion);
				pBlock1->SetValue(usd_metallic_map, m_time, pMetalTex);
				pBlock1->SetValue(usd_roughness_map, m_time, pRoughTex);
				pBlock1->SetValue(usd_occlusion_map, m_time, pOccTex);
			}
			else {
				if (Occlusion) {
					BitmapTex *pBmpRTex = SplitOcclusionTexture(pBmpTex);
					CorrectBitmapGamma(pBmpRTex, 1.0f);
					SetTextureUVoffset(pBmpRTex, &metalRgh->metallic_roughness_texture);
					pBlock1->SetValue(usd_occlusion_map, m_time, pBmpRTex);
				}
				if (Roughness) {
					BitmapTex *pBmpGTex = SplitRoughnessTexture(pBmpTex);
					CorrectBitmapGamma(pBmpGTex, 1.0f);
					SetTextureUVoffset(pBmpGTex, &metalRgh->metallic_roughness_texture);
					pBlock1->SetValue(usd_roughness_map, m_time, pBmpGTex);
				}
				if (Metalness) {
					BitmapTex *pBmpBTex = SplitMetalnessTexture(pBmpTex);
					CorrectBitmapGamma(pBmpBTex, 1.0f);
					SetTextureUVoffset(pBmpBTex, &metalRgh->metallic_roughness_texture);
					pBlock1->SetValue(usd_metallic_map, m_time, pBmpBTex);
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
				pBlock1->SetValue(usd_occlusion_map, m_time, pOccTex);
			}
			else if (m_MapUnpackMode == 2) {
				Texmap* pMetalTex = NULL;
				Texmap* pRoughTex = NULL;
				Texmap* pOccTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, &pMetalTex, &pRoughTex, &pOccTex, FALSE, FALSE, TRUE);
				pBlock1->SetValue(usd_occlusion_map, m_time, pOccTex);
			}
			else {
				pBmpTex = SplitOcclusionTexture(pBmpTex);
				pBlock1->SetValue(usd_occlusion_map, m_time, pBmpTex);
			}
		}

		cgltf_texture_view *nrmTexInfo = &mtl->normal_texture;
		if (nrmTexInfo->texture) {
			pBmpTex = GetBitmapTexFromglTexture(nrmTexInfo->texture);
			CorrectBitmapGamma(pBmpTex, 1.0f);

			SetTextureUVoffset(pBmpTex, nrmTexInfo);
			pBlock1->SetValue(usd_normal_map, m_time, pBmpTex);
			pBlock1->SetValue(usd_normal, m_time, nrmTexInfo->scale);
		}

		cgltf_texture_view *emissTexInfo = &mtl->emissive_texture;
		if (emissTexInfo->texture) {
			pBmpTex = GetBitmapTexFromglTexture(emissTexInfo->texture);
			SetTextureUVoffset(pBmpTex, emissTexInfo);
			pBlock1->SetValue(usd_emissiveColor_map, m_time, pBmpTex);
		}

		float *emissive = mtl->emissive_factor;
		if (emissive) {
			Color c(emissive[0], emissive[1], emissive[2]);
			pBlock1->SetValue(usd_emissiveColor, m_time, c);
		}

		if (mtl->has_emissive_strength) {
			cgltf_emissive_strength *strength = &mtl->emissive_strength;
		}

		if (mtl->has_clearcoat) {
			cgltf_clearcoat* clearcoat = &mtl->clearcoat;
			pBlock1->SetValue(usd_clearcoat, m_time, clearcoat->clearcoat_factor);
			pBlock1->SetValue(usd_clearcoatRoughness, m_time, clearcoat->clearcoat_roughness_factor);
			cgltf_texture_view* clrTexInfo = &clearcoat->clearcoat_texture;
			if (clrTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(clrTexInfo->texture);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, clrTexInfo);
				Texmap* pRTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, NULL, NULL, &pRTex, FALSE, FALSE, TRUE);
				pBlock1->SetValue(usd_clearcoat_map, m_time, pRTex);
			}
			cgltf_texture_view* clrRghTexInfo = &clearcoat->clearcoat_roughness_texture;
			if (clrRghTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(clrRghTexInfo->texture);
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, clrRghTexInfo);
				Texmap* pGTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, NULL, &pGTex, NULL, FALSE, TRUE, FALSE);
				pBlock1->SetValue(usd_clearcoatRoughness_map, m_time, pGTex);
			}
		}

		if (mtl->has_transmission) {
			cgltf_transmission *transmission = &mtl->transmission;
			pBmpTex = GetBitmapTexFromglTexture(transmission->transmission_texture.texture);
			if (pBmpTex) {
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, &transmission->transmission_texture);
				Texmap* pRTex = NULL;
				SetMetalRoughOccClrCorrectMap(pBmpTex, NULL, NULL, &pRTex, FALSE, FALSE, TRUE);
				pBlock1->SetValue(usd_opacity_map, m_time, pRTex);
			}
			else {
				float f = transmission->transmission_factor;
				if (f > 0.0f) {
					Color c(col);
					c *= 0.1f;
					Texmap *pColTex = CreateColorMap(AColor(c));
					pBlock1->SetValue(usd_opacity_map, m_time, pColTex);
				}
			}
		}
		if (mtl->has_volume) {
			cgltf_volume volume;
		}
		if (mtl->has_ior) {
			cgltf_ior *ior = &mtl->ior;
			pBlock1->SetValue(usd_ior, m_time, ior->ior);
		}
		if (mtl->has_specular) {
			cgltf_specular *specular = &mtl->specular;
			Color c(specular->specular_color_factor);
			pBlock1->SetValue(usd_specularcolor, m_time, c);
			pBmpTex = GetBitmapTexFromglTexture(specular->specular_texture.texture);
			if (pBmpTex) {
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, &specular->specular_texture);
				pBlock1->SetValue(usd_specularColor_map, m_time, pBmpTex);
			}
		}
		if (mtl->has_sheen) {
			cgltf_sheen sheen;
		}
		if (mtl->has_pbr_specular_glossiness) {
			cgltf_pbr_specular_glossiness spec_gls;
		}
		
		cgltf_extension *ext = mtl->extensions;
		for (int cnt = 0; cnt < mtl->extensions_count; cnt++, ext++) {
			char* name = ext->name;
			char* data = ext->data;
		}

		//gltf2::Extension ext = mtl.extensions();
		for (int j = 0; j < mtl->extensions_count; j++) {
			cgltf_extension *ext = &mtl->extensions[j];
		}

		bool doubleSided = mtl->double_sided;

		AttacheAlphaModeCustAttr(pSmat, mtl->alpha_mode);

		CreateTransmissionAttr(pSmat, &mtl->transmission, mtl->has_transmission);
		CreateVolumeAttr(pSmat, &mtl->volume, mtl->has_volume);
		CreateIridescenceAttr(pSmat, &mtl->iridescence, mtl->has_iridescence);
		CreateSheenAttr(pSmat, &mtl->sheen, mtl->has_sheen);
		CreateUnlitAttr(pSmat, mtl->unlit);
		CreateEmissiveStrengthAttr(pSmat, &mtl->emissive_strength, mtl->has_emissive_strength);
		CreateDiffuseTransmissionAttr(pSmat, &mtl->diffuse_transmission, mtl->has_diffuse_transmission);

		m_MaterialMap.insert(std::make_pair(mtl, pSmat));
		SetMtlImportStatus(i + 1);
	}
}
