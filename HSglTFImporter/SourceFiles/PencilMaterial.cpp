#include "HSglTFImporter.h"
#include <shaders.h>
#include "define.h"

#define PencilZone Class_ID(0x6bd2229, 0x7d7d6ca5)

//=============================================================================
//=============================================================================
void glTFImporter_Core::CreatePencilMaterial(void)
{
	TSTR ComStr;
	FPValue fpv;

#if MAX_RELEASE >= 24000
	ExecuteMAXScriptScript(_T("global pcl4lineset = Pencil_4_Lineset()"), MAXScript::ScriptSource::NonEmbedded, TRUE, &fpv);
	ExecuteMAXScriptScript(_T("pcl4lineset.openEdgeObjectVisible=off"), MAXScript::ScriptSource::NonEmbedded, TRUE, &fpv);
	//ExecuteMAXScriptScript(_T("pcl4lineset.edgeIsectVisible=off"), MAXScript::ScriptSource::NonEmbedded, TRUE, &fpv);
#else
	ExecuteMAXScriptScript(_T("global pcl4lineset = Pencil_4_Lineset()"), TRUE, &fpv);
	ExecuteMAXScriptScript(_T("pcl4lineset.openEdgeObjectVisible=off"), TRUE, &fpv);
#endif

	for (int i = 0; i < m_glTF_data->materials_count; i++) {

		Mtl* pSmat = NULL;
#if MAX_RELEASE >= 24000
		ExecuteMAXScriptScript(_T("global tempmtl = Pencil_4_Material()"), MAXScript::ScriptSource::NonEmbedded, TRUE, &fpv);
		pSmat = fpv.mtl;
		ExecuteMAXScriptScript(_T("appendIfUnique pcl4lineset.mtls tempmtl"), MAXScript::ScriptSource::NonEmbedded, TRUE, &fpv);
#else
		ExecuteMAXScriptScript(_T("global tempmtl = Pencil_4_Material()"), TRUE, &fpv);
		pSmat = fpv.mtl;
		ExecuteMAXScriptScript(_T("appendIfUnique pcl4lineset.mtls tempmtl"), TRUE, &fpv);
#endif
		//Mtl* pSmat = (Mtl*)GetCOREInterface()->CreateInstance(MATERIAL_CLASS_ID, Pencil4MaterialID);
		//ReferenceTarget* pZone = (ReferenceTarget*)GetCOREInterface()->CreateInstance(REF_TARGET_CLASS_ID, PencilZone);

		cgltf_material* mtl = &m_glTF_data->materials[i];
		cgltf_pbr_metallic_roughness* metalRgh = &mtl->pbr_metallic_roughness;

		pSmat->SetName(StringToWString(mtl->name).c_str());
		IParamBlock2* pBlock0 = pSmat->GetParamBlock(0);
		IParamBlock2* pBlock1 = pSmat->GetParamBlockByID(1);
		IParamBlock2* pBlock2 = pSmat->GetParamBlockByID(2);

		Mtl* pBaseMtl = pBlock0->GetMtl(pen_basicMaterial);
		Shader* pShader = ((StdMat2*)pBaseMtl)->GetShader();
		IParamBlock2* pBaseBlock0 = pBaseMtl->GetParamBlock(0);
		IParamBlock2* pBaseBlock1 = pBaseMtl->GetParamBlock(1);
		IParamBlock2* pBaseBlock2 = pBaseMtl->GetParamBlock(2);
		IParamBlock2* pBaseBlock3 = pBaseMtl->GetParamBlock(3);

		BOOL ColorFound = FALSE;
		float* col = metalRgh->base_color_factor;

		BitmapTex* pBmpTex = NULL;
		if (metalRgh->base_color_texture.texture) {
			pBmpTex = GetBitmapTexFromglTexture(metalRgh->base_color_texture.texture);
			pBmpTex->SetAlphaSource(ALPHA_FILE);
			SetTextureUVoffset(pBmpTex, &metalRgh->base_color_texture);
		}

		if (mtl->has_pbr_specular_glossiness) {
			cgltf_pbr_specular_glossiness* spl_gls = &mtl->pbr_specular_glossiness;
			cgltf_texture_view* diffuseTexInfo = &spl_gls->diffuse_texture;
			if (diffuseTexInfo->texture) {
				pBmpTex = GetBitmapTexFromglTexture(diffuseTexInfo->texture);
				SetTextureUVoffset(pBmpTex, diffuseTexInfo);
			}

			cgltf_texture_view* spglTexInfo = &spl_gls->specular_glossiness_texture;
			float* colSpec = spl_gls->specular_factor;
			float* colDiff = spl_gls->diffuse_factor;
		}

		ReferenceTarget* pZone0;
		ReferenceTarget* pZone1;

		pBlock0->Resize(pen_Zones, 2);
		pBlock0->GetValue(pen_Zones, m_time, pZone0, FOREVER, 0);
		pBlock0->GetValue(pen_Zones, m_time, pZone1, FOREVER, 1);

		IParamBlock2* pZoneBlock0 = pZone0->GetParamBlock(0);
		IParamBlock2* pZoneBlock1 = pZone1->GetParamBlock(0);

		pZoneBlock0->SetValue(pen_Zone_posMin, m_time, 0.0f);
		pZoneBlock0->SetValue(pen_Zone_posMax, m_time, 80.0f);
		pZoneBlock1->SetValue(pen_Zone_posMin, m_time, 80.0f);
		pZoneBlock1->SetValue(pen_Zone_posMax, m_time, 100.0f);

		if (col) {
			Color c(col);
			pZoneBlock0->SetValue(pen_Zone_color, m_time, c*0.5f);
			pZoneBlock1->SetValue(pen_Zone_color, m_time, c);
			if(pShader)pShader->SetDiffuseClr(c, m_time);
		}

		if (pBmpTex) {
			pZoneBlock0->SetValue(pen_Zone_blendMode, m_time, 3);
			pZoneBlock1->SetValue(pen_Zone_blendMode, m_time, 3);
			pBaseMtl->SetSubTexmap(ID_DI, pBmpTex);
		}

		if (pBmpTex && (mtl->alpha_mode == cgltf_alpha_mode_opaque)) {
			pBmpTex->SetAlphaSource(ALPHA_NONE);
		}
		else if (pBmpTex && (mtl->alpha_mode == cgltf_alpha_mode_mask || mtl->alpha_mode == cgltf_alpha_mode_blend)) {
			Texmap* pAlphaBmp = CreateAlphaFilterMap(pBmpTex);

			if (mtl->alpha_mode == cgltf_alpha_mode_mask) {
				Texmap* pOSLMap = CreateCutOffOSLNode(pAlphaBmp, mtl->alpha_cutoff);
				if (pOSLMap)
					pBaseMtl->SetSubTexmap(ID_OP, pOSLMap);
			}
			else {
				pBaseMtl->SetSubTexmap(ID_OP, pAlphaBmp);
			}
		}

		/*
		if (col)
			if (col[0] < 1.0f || col[1] < 1.0f || col[2] < 1.0f) ColorFound = TRUE;
		Texmap* pBaseColorMap = NULL;
		if (m_UseColorComposite && metalRgh->base_color_texture.texture && ColorFound) {
			pBaseColorMap = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, VRayCompTexID);
			pBaseColorMap->GetParamBlock(0)->SetValue(2, m_time, 3);
		}

		if (ColorFound) {
			//AColor c(col[0], col[1], col[2], col[3]);
			Color c(col[0], col[1], col[2]);
			if (pBaseColorMap && metalRgh->base_color_texture.texture) {
				Texmap* pColTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, VRayColorID);
				pColTex->GetParamBlock(0)->SetValue(5, m_time, c);
				pBaseColorMap->GetParamBlock(0)->SetValue(0, m_time, pColTex);
			}
		}

		if (metalRgh->base_color_texture.texture) {
			pBmpTex = GetBitmapTexFromglTexture(metalRgh->base_color_texture.texture);
			SetTextureUVoffset(pBmpTex, &metalRgh->base_color_texture);
			Texmap* pTex = BitmapTexToVRayBitmap(pBmpTex);
			if (pBaseColorMap) {
				pBaseColorMap->GetParamBlock(0)->SetValue(1, m_time, pTex);
			}
			else {
			}

			if (mtl->alpha_mode == cgltf_alpha_mode_mask || mtl->alpha_mode == cgltf_alpha_mode_blend) {
				Texmap* pAlphaBmp = CreateAlphaFilterMap(pTex);

				if (mtl->alpha_mode == cgltf_alpha_mode_mask) {
					Texmap* pOSLMap = CreateCutOffOSLNode(pAlphaBmp, mtl->alpha_cutoff);
					if (pOSLMap){}
					else {}
				}
				else {
				}
			}
		}
*/
		cgltf_texture_view* nrmTexInfo = &mtl->normal_texture;
		if (nrmTexInfo->texture) {
			Texmap* pNormlMap = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, GNORMAL_CLASS_ID);
			Texmap* pSubMap = GetBitmapTexFromglTexture(nrmTexInfo->texture);
			//CorrectBitmapGamma(pSubMap, 1.0f);

			SetTextureUVoffset(pSubMap, nrmTexInfo);
			pNormlMap->GetParamBlock(0)->SetValue(0, m_time, nrmTexInfo->scale);
			pNormlMap->GetParamBlock(0)->SetValue(2, m_time, pSubMap);
			pNormlMap->GetParamBlock(0)->SetValue(7, m_time, m_FlipNormalRed);
			pNormlMap->GetParamBlock(0)->SetValue(8, m_time, m_FlipNormalGrn);

			pBlock0->SetValue(pen_bumpEnable, m_time, 1);
			pBlock0->SetValue(pen_bumpAmount, m_time, 100);
			pBlock0->SetValue(pen_bumpMap, m_time, pNormlMap);
		}

		cgltf_texture_view* emissTexInfo = &mtl->emissive_texture;
		if (emissTexInfo->texture) {
			pBmpTex = GetBitmapTexFromglTexture(emissTexInfo->texture);
			SetTextureUVoffset(pBmpTex, emissTexInfo);
			pBaseBlock3->SetValue(1, m_time, pBmpTex, 5);
		}

		float* emissive = mtl->emissive_factor;
		if (emissive) {
			Color c(emissive[0], emissive[1], emissive[2]);
		}

		if (mtl->has_emissive_strength) {
			cgltf_emissive_strength* strength = &mtl->emissive_strength;
		}

		if (mtl->has_clearcoat) {
			cgltf_clearcoat* clearcoat = &mtl->clearcoat;
		}

		if (mtl->has_sheen) {
			cgltf_sheen* sheen = &mtl->sheen;
		}

		if (mtl->has_transmission) {
			cgltf_transmission* transmission = &mtl->transmission;
			float f = transmission->transmission_factor;
			pBlock1->SetValue(1, m_time, 1.0f-f);
			pBmpTex = GetBitmapTexFromglTexture(transmission->transmission_texture.texture);
			if (pBmpTex) {
				CorrectBitmapGamma(pBmpTex, 1.0f);
				SetTextureUVoffset(pBmpTex, &transmission->transmission_texture);
				pBaseMtl->SetSubTexmap(ID_OP, pBmpTex);
				//pBlock1->SetValue(glTF_transmissionMap, m_time, pBmpTex);
			}
		}

		if (mtl->has_volume) {
			cgltf_volume* volume = &mtl->volume;
		}

		if (mtl->has_ior) {
			cgltf_ior* ior = &mtl->ior;
		}

		AttacheAlphaModeCustAttr(pSmat, mtl->alpha_mode);

		m_MaterialMap.insert(std::make_pair(mtl, pSmat));
		SetMtlImportStatus(i + 1);
	}

#if MAX_RELEASE >= 24000
	ExecuteMAXScriptScript(_T("global pcl4line = Pencil_4_Line()"), MAXScript::ScriptSource::NonEmbedded, TRUE, &fpv);
	ExecuteMAXScriptScript(_T("pcl4line.linesets = #(pcl4lineset)"), MAXScript::ScriptSource::NonEmbedded, TRUE, &fpv);
	ExecuteMAXScriptScript(_T("addEffect( pcl4line )"), MAXScript::ScriptSource::NonEmbedded, TRUE, &fpv);

	//ExecuteMAXScriptScript(_T("(NitrousGraphicsManager.GetActiveViewportSetting()).ShowMaterialOption=#Realistic"), MAXScript::ScriptSource::NonEmbedded, TRUE, &fpv);
	ExecuteMAXScriptScript(_T("renderers.production =Default_Scanline_Renderer()"), MAXScript::ScriptSource::NonEmbedded, TRUE, &fpv);
#else
	ExecuteMAXScriptScript(_T("global pcl4line = Pencil_4_Line()"), TRUE, &fpv);
	ExecuteMAXScriptScript(_T("pcl4line.linesets = #(pcl4lineset)"), TRUE, &fpv);
	ExecuteMAXScriptScript(_T("addEffect( pcl4line )"), TRUE, &fpv);

	ExecuteMAXScriptScript(_T("renderers.production =Default_Scanline_Renderer()"), TRUE, &fpv);
#endif
}