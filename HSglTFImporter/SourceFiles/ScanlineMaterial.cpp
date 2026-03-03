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
void glTFImporter_Core::CreateScanlineMaterial(void)
{
	for (int i = 0; i < m_glTF_data->materials_count; i++) {
		cgltf_material *mtl = &m_glTF_data->materials[i];
		StdMat2 *pSmat = (StdMat2*)NewDefaultStdMat();
		pSmat->SetName(StringToWString(mtl->name).c_str());

		pSmat->SetDiffuse(Color(1.0f, 1.0f, 1.0f), m_time);

		BitmapTex *pBmpTex = NULL;

		cgltf_pbr_metallic_roughness *metalRgh = &mtl->pbr_metallic_roughness;
		if (metalRgh->base_color_texture.texture) {
			pBmpTex = GetBitmapTexFromglTexture(metalRgh->base_color_texture.texture);
			SetTextureUVoffset(pBmpTex, &metalRgh->base_color_texture);
			pSmat->SetSubTexmap(ID_DI, pBmpTex);
		}

		BOOL ColorFound = FALSE;
		float *col = metalRgh->base_color_factor;
		if (col) 
			if (col[0] < 1.0f || col[1] < 1.0f || col[2] < 1.0f) ColorFound = TRUE;

		Texmap *pBaseColorMap = NULL;
		if (m_UseColorComposite && metalRgh->base_color_texture.texture && ColorFound) {
			pBaseColorMap = CreateBaseColorMap();
			pBaseColorMap->GetParamBlock(0)->SetValue(9, m_time, pBmpTex, 0);
			pSmat->SetSubTexmap(ID_DI, pBaseColorMap);
		}

		if (ColorFound) {
			Control* pClr = (Control*)GetCOREInterface()->CreateInstance(CTRL_POINT4_CLASS_ID, Class_ID(0x2013, 0x0));
			Color c(col[0], col[1], col[2]);
			if (pBaseColorMap) {
				Texmap *pColTex = CreateColorMap(AColor(c));
				pBaseColorMap->GetParamBlock(0)->SetValue(9, m_time, pColTex, 1);
				pBaseColorMap->GetParamBlock(0)->SetValue(5, m_time, 5, 1);
				pColTex->GetParamBlock(0)->SetControllerByID(5, 0, pClr);
			}
//			pBlock0->SetControllerByID(vr_diffuse, 0, pClr);
			pSmat->SetDiffuse(c, m_time);
		}
		//Class_ID cc2 = pBmpTex->ClassID();

		if (pBmpTex && (mtl->alpha_mode == cgltf_alpha_mode_opaque)) {
			Class_ID cc = pBmpTex->ClassID();
			pBmpTex->SetAlphaSource(ALPHA_NONE);
		} else if (pBmpTex && (mtl->alpha_mode == cgltf_alpha_mode_mask || mtl->alpha_mode == cgltf_alpha_mode_blend)) {
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
				Texmap *pOSLMap = CreateCutOffOSLNode(pAlphaBmp, mtl->alpha_cutoff);
				if (pOSLMap)
					pSmat->SetSubTexmap(ID_OP, pOSLMap);
			}
			else {
				pSmat->SetSubTexmap(ID_OP, pAlphaBmp);
			}
		}

		if (mtl->has_pbr_specular_glossiness) {
			cgltf_pbr_specular_glossiness *spl_gls = &mtl->pbr_specular_glossiness;
			cgltf_texture_view *diffuseTexInfo = &spl_gls->diffuse_texture;
			if (diffuseTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(diffuseTexInfo->texture);
				SetTextureUVoffset(pBmpTex, diffuseTexInfo);
				pSmat->SetSubTexmap(ID_DI, pBmpTex);
			}
			cgltf_texture_view *spglTexInfo = &spl_gls->specular_glossiness_texture;
			if (spglTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(spglTexInfo->texture);
				CorrectBitmapGamma(pBmpTex, 1.0f);
			}

			float *colSpec = spl_gls->specular_factor;
			if (colSpec) {
				Color col(colSpec[0], colSpec[1], colSpec[2]);
			}
			else {
			}
			float *colDiff = spl_gls->diffuse_factor;

			float f = spl_gls->glossiness_factor;
		}

		cgltf_texture_view *nrmTexInfo = &mtl->normal_texture;
		if (nrmTexInfo->texture) {
			Texmap *pNormlMap = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, GNORMAL_CLASS_ID);
			pBmpTex = GetBitmapTexFromglTexture(nrmTexInfo->texture);
			CorrectBitmapGamma(pBmpTex, 1.0f);

			SetTextureUVoffset(pBmpTex, nrmTexInfo);
			pNormlMap->GetParamBlock(0)->SetValue(0, m_time, nrmTexInfo->scale);
			pNormlMap->GetParamBlock(0)->SetValue(2, m_time, pBmpTex);
			pNormlMap->GetParamBlock(0)->SetValue(7, m_time, m_FlipNormalRed);
			pNormlMap->GetParamBlock(0)->SetValue(8, m_time, m_FlipNormalGrn);

			pSmat->SetSubTexmap(ID_BU, pNormlMap);

		}

		cgltf_texture_view *emissTexInfo = &mtl->emissive_texture;
		if (emissTexInfo->texture) {
			pBmpTex = GetBitmapTexFromglTexture(emissTexInfo->texture);
			SetTextureUVoffset(pBmpTex, emissTexInfo);
		}

		float *emissive = mtl->emissive_factor;
		if (emissive) {
		//	pSmat->SetSelfIllumColorOn(TRUE);
		//	Point3 em(emissive);
		//	pSmat->SetSelfIllumColor(em, m_time);
		}
		/*
				if (pSrcMtl->GetTextureCount(aiTextureType_OPACITY) > 0) {
					aiString texpath;
					pSrcMtl->GetTexture(aiTextureType_OPACITY, 0, &texpath);
					BitmapTex *pBitmap = GetBitmapTex(texpath);
					pSmat->SetSubTexmap(ID_OP, pBitmap);
				}

				if (pSrcMtl->GetTextureCount(aiTextureType_EMISSIVE) > 0) {
					aiString texpath;
					pSrcMtl->GetTexture(aiTextureType_EMISSIVE, 0, &texpath);
					BitmapTex *pBitmap = GetBitmapTex(texpath);
					pSmat->SetSubTexmap(ID_SI, pBitmap);
				}

				aiColor4D color4;
				if (pSrcMtl->Get(AI_MATKEY_COLOR_DIFFUSE, color4) == AI_SUCCESS)
					pSmat->SetDiffuse(Color(color4.r, color4.g, color4.b), m_time);

				if (pSrcMtl->Get(AI_MATKEY_COLOR_AMBIENT, color4) == AI_SUCCESS)
					pSmat->SetAmbient(Color(color4.r, color4.g, color4.b), m_time);

				if (pSrcMtl->Get(AI_MATKEY_COLOR_SPECULAR, color4) == AI_SUCCESS)
					pSmat->SetSpecular(Color(color4.r, color4.g, color4.b), m_time);

				float f;
				if (pSrcMtl->Get(AI_MATKEY_SHININESS, f) == AI_SUCCESS)
					pSmat->SetShininess(f, m_time);

				if (pSrcMtl->Get(AI_MATKEY_SHININESS_STRENGTH, f) == AI_SUCCESS)
					pSmat->SetShinStr(f, m_time);

				if (pSrcMtl->Get(AI_MATKEY_OPACITY, f) == AI_SUCCESS)
					pSmat->SetOpacity(f, m_time);

				int val;
				if (pSrcMtl->Get(AI_MATKEY_TWOSIDED, val) == AI_SUCCESS)
					pSmat->SetTwoSided(val);
		*/


		AttacheAlphaModeCustAttr(pSmat, mtl->alpha_mode);

		//CreateIORAttr(pSmat, &mtl->ior, mtl->has_ior);
		//CreateTransmissionAttr(pSmat, &mtl->transmission, mtl->has_transmission);
		//CreateVolumeAttr(pSmat, &mtl->volume, mtl->has_volume);
		//CreateIridescenceAttr(pSmat, &mtl->iridescence, mtl->has_iridescence);
		//CreateSheenAttr(pSmat, &mtl->sheen, mtl->has_sheen);
		//CreateClearcoatAttr(pSmat, &mtl->clearcoat, mtl->has_clearcoat);
		//CreateUnlitAttr(pSmat, mtl->unlit);
		//CreateEmissiveStrengthAttr(pSmat, &mtl->emissive_strength, mtl->has_emissive_strength);

		m_MaterialMap.insert(std::make_pair(mtl, pSmat));
		SetMtlImportStatus(i + 1);
	}
}
