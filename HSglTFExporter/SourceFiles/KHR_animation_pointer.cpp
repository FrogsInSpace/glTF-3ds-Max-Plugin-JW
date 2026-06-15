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


#include "HSglTFExporter.h"
#include <Shaders.h>

//Control* ConvertColorToFloatController(Control* pSrcC, UINT ch);

//======================================================================
//======================================================================
BaseShader *GetShader(MtlBase *pMtl)
{
	for (int i = 0; i < pMtl->NumSubs(); i++) {
		Animatable* p = pMtl->SubAnim(i);
		if (p->SuperClassID() == SHADER_CLASS_ID) return (BaseShader*)p;
	}
	return NULL;
}

//======================================================================
//======================================================================
void glTFExporter_Core::CreateAnimationPointer(void)
{

	m_ParamAnimationTable.clear();

	for (auto m : m_MaterialMap) {

		MtlBase* pMtl = m.first;
		TSTR name = pMtl->GetName();
		if (pMtl->ClassID() == ScanLineMtlID) {
			Texmap *pTex = pMtl->GetSubTexmap(ID_DI);
			CreateUVAnimation(pTex, m.second, TargetTex::BaseColorMap);
			pTex = pMtl->GetSubTexmap(ID_BU);
			CreateUVAnimation(pTex, m.second, TargetTex::NormalMap);

			BaseShader* pShader = GetShader(pMtl);
			IParamBlock2* pBlock = pShader->GetParamBlockByID(0);
			Control* pC = pBlock->GetControllerByID(1);
			CreateBaseColorAnimation(pC, m.second, FALSE);

			pTex = pMtl->GetSubTexmap(ID_OP);
			if (GetOSLMapType(pTex) == OSL_CutOff) {
				IParamBlock2* pBlock = pTex->GetParamBlock(1);
				Control* pC = pBlock->GetControllerByID(0);
				CreateAlphaCutOffAnimation(pC, m.second);
			}
		}
		else if (pMtl->ClassID() == PBRMetalMtlID) {
			IParamBlock2* pBlock = pMtl->GetParamBlockByID(1);
			Texmap* pTex = pBlock->GetTexmap(pbr_base_color_map);
			CreateUVAnimation(pTex, m.second, TargetTex::BaseColorMap);
			pTex = pBlock->GetTexmap(pbr_metalness_map);
			CreateUVAnimation(pTex, m.second, TargetTex::MetalnessMap);
			pTex = pBlock->GetTexmap(pbr_roughness_map);
			CreateUVAnimation(pTex, m.second, TargetTex::RoughnessMap);
			pTex = pBlock->GetTexmap(pbr_norm_map);
			CreateUVAnimation(pTex, m.second, TargetTex::NormalMap);
			pTex = pBlock->GetTexmap(pbr_emit_color_map);
			CreateUVAnimation(pTex, m.second, TargetTex::EmissiveMap);
			pTex = pBlock->GetTexmap(pbr_ao_map);
			CreateUVAnimation(pTex, m.second, TargetTex::OcclusionMap);

			Control* pC = pBlock->GetControllerByID(pbr_base_color);
			CreateBaseColorAnimation(pC, m.second, TRUE);

			pTex = pBlock->GetTexmap(pbr_opacity_map, 0);
			if (GetOSLMapType(pTex) == OSL_CutOff) {
				IParamBlock2* pBlock = pTex->GetParamBlock(1);
				Control* pC = pBlock->GetControllerByID(0);
				CreateAlphaCutOffAnimation(pC, m.second);
			}

			pC = pBlock->GetControllerByID(pbr_metalness);
			CreateMetalicFactorAnimation(pC, m.second);

			pC = pBlock->GetControllerByID(pbr_roughness);
			CreateRoughnessFactorAnimation(pC, m.second);

			pC = pBlock->GetControllerByID(pbr_bump_map_amt);
			CreateNormalScaleAnimation(pC, m.second);

			pC = pBlock->GetControllerByID(pbr_emit_color);
			CreateEmissiveFactorAnimation(pC, m.second);
		}
		else if (pMtl->ClassID() == PBRSpecGlossMtlID) {
		}
		else if (pMtl->ClassID() == PHYSICALMATERIAL_CLASS_ID) {
			IParamBlock2* pBlock = pMtl->GetParamBlockByID(0);
			Texmap* pTex = pBlock->GetTexmap(fm_base_color_map);
			CreateUVAnimation(pTex, m.second, TargetTex::BaseColorMap);
			pTex = pBlock->GetTexmap(fm_metalness_map);
			CreateUVAnimation(pTex, m.second, TargetTex::MetalnessMap);
			pTex = pBlock->GetTexmap(fm_roughness_map);
			CreateUVAnimation(pTex, m.second, TargetTex::RoughnessMap);
			pTex = pBlock->GetTexmap(fm_emission_map);
			CreateUVAnimation(pTex, m.second, TargetTex::EmissiveMap);
			pTex = pBlock->GetTexmap(fm_transparency_map);
			CreateUVAnimation(pTex, m.second, TargetTex::TransmissionMap);
			pTex = pBlock->GetTexmap(fm_bump_map);
			CreateUVAnimation(pTex, m.second, TargetTex::NormalMap);
			//pTex = pBlock->GetTexmap(pbr_ao_map);
			//CreateUVAnimation(pTex, m.second, OcclusionMap);
			pTex = pBlock->GetTexmap(fm_coat_map);
			CreateUVAnimation(pTex, m.second, TargetTex::ClearcoatMap);
			pTex = pBlock->GetTexmap(fm_coat_rough_map);
			CreateUVAnimation(pTex, m.second, TargetTex::ClearcoatRoughnessMap);
			pTex = pBlock->GetTexmap(fm_coat_bump_map);
			CreateUVAnimation(pTex, m.second, TargetTex::ClearcoatNormalMap);
#if MAX_RELEASE >= 25000
			pTex = pBlock->GetTexmap(fm_sheen_color_map);
			CreateUVAnimation(pTex, m.second, TargetTex::SheenColorMap);
			pTex = pBlock->GetTexmap(fm_sheen_rough_map);
			CreateUVAnimation(pTex, m.second, TargetTex::SheenRoughnessMap);
			pTex = pBlock->GetTexmap(fm_thin_film_map);
			CreateUVAnimation(pTex, m.second, TargetTex::IridescenceMap);
#endif

			Control* pC = pBlock->GetControllerByID(fm_base_color);
			CreateBaseColorAnimation(pC, m.second, TRUE);

			//pBlock = pMtl->GetParamBlock(0);
			pTex = pBlock->GetTexmap(fm_cutout_map, 0);
			if (GetOSLMapType(pTex) == OSL_CutOff) {
				IParamBlock2* pBlock = pTex->GetParamBlock(1);
				Control* pC = pBlock->GetControllerByID(0);
				CreateAlphaCutOffAnimation(pC, m.second);
			}

			pC = pBlock->GetControllerByID(fm_metalness);
			CreateMetalicFactorAnimation(pC, m.second);

			pC = pBlock->GetControllerByID(fm_roughness);
			CreateRoughnessFactorAnimation(pC, m.second);

			//pC = pBlock->GetControllerByID(glTF_ambientOcclusion);
			//CreateOcclusionStrengthAnimation(pC, m.second);

			pC = pBlock->GetControllerByID(fm_emit_color);
			CreateEmissiveFactorAnimation(pC, m.second);

#if MAX_RELEASE >= 25000
			pC = pBlock->GetControllerByID(fm_thin_film_weight);
			CreateIridescenceFactorAnimation(pC, m.second);
			pC = pBlock->GetControllerByID(fm_thin_film_thickness);
			CreateIridescenceThicknessMaxAnimation(pC, m.second);
			pC = pBlock->GetControllerByID(fm_thin_film_ior);
			CreateIridescenceIorAnimation(pC, m.second);
#endif
		}
		else  if (pMtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {
		}
		else  if (pMtl->ClassID() == glTFMaterialID) {
			int onoff;
			IParamBlock2* pBlock0 = pMtl->GetParamBlockByID(0);
			IParamBlock2* pBlock1 = pMtl->GetParamBlockByID(1);

			Texmap* pTex = pBlock0->GetTexmap(glTF_baseColorMap);
			CreateUVAnimation(pTex, m.second, TargetTex::BaseColorMap);
			//pTex = pBlock0->GetTexmap(glTF_alphaMap);
			//CreateUVAnimation(pTex, m.second, TransmissionMap);
			pTex = pBlock0->GetTexmap(glTF_metalnessMap);
			if (!CreateUVAnimation(pTex, m.second, TargetTex::MetalnessMap)) {
				pTex = pBlock0->GetTexmap(glTF_roughnessMap);
				CreateUVAnimation(pTex, m.second, TargetTex::RoughnessMap);
			}
			pTex = pBlock0->GetTexmap(glTF_normalMap);
			CreateUVAnimation(pTex, m.second, TargetTex::NormalMap);
			pTex = pBlock0->GetTexmap(glTF_ambientOcclusionMap);
			CreateUVAnimation(pTex, m.second, TargetTex::OcclusionMap);
			pTex = pBlock0->GetTexmap(glTF_emissionMap);
			CreateUVAnimation(pTex, m.second, TargetTex::EmissiveMap);

			pTex = pBlock1->GetTexmap(glTF_clearcoatMap);
			CreateUVAnimation(pTex, m.second, TargetTex::ClearcoatMap);
			pTex = pBlock1->GetTexmap(glTF_clearcoatRoughnessMap);
			CreateUVAnimation(pTex, m.second, TargetTex::ClearcoatRoughnessMap);
			pTex = pBlock1->GetTexmap(glTF_clearcoatNormalMap);
			CreateUVAnimation(pTex, m.second, TargetTex::ClearcoatNormalMap);
			pTex = pBlock1->GetTexmap(glTF_sheenColorMap);
			CreateUVAnimation(pTex, m.second, TargetTex::SheenColorMap);
			pTex = pBlock1->GetTexmap(glTF_sheenRoughnessMap);
			CreateUVAnimation(pTex, m.second, TargetTex::SheenRoughnessMap);
			pTex = pBlock1->GetTexmap(glTF_specularMap);
			CreateUVAnimation(pTex, m.second, TargetTex::SpecularMap);
			pTex = pBlock1->GetTexmap(glTF_specularColorMap);
			CreateUVAnimation(pTex, m.second, TargetTex::SpecularColorMap);
			pTex = pBlock1->GetTexmap(glTF_transmissionMap);
			CreateUVAnimation(pTex, m.second, TargetTex::TransmissionMap);
			pTex = pBlock1->GetTexmap(glTF_volumeThicknessMap);
			CreateUVAnimation(pTex, m.second, TargetTex::VolumeThicknessMap);

			Control* pC = pBlock0->GetControllerByID(glTF_baseColor);
			CreateBaseColorAnimation(pC, m.second, TRUE);

			pC = pBlock0->GetControllerByID(glTF_alphaCutoff);
			CreateAlphaCutOffAnimation(pC, m.second);

			pC = pBlock0->GetControllerByID(glTF_metalness);
			CreateMetalicFactorAnimation(pC, m.second);

			pC = pBlock0->GetControllerByID(glTF_roughness);
			CreateRoughnessFactorAnimation(pC, m.second);

			pC = pBlock0->GetControllerByID(glTF_normal);
			CreateNormalScaleAnimation(pC, m.second);

			pC = pBlock0->GetControllerByID(glTF_ambientOcclusion);
			CreateOcclusionStrengthAnimation(pC, m.second);

			pC = pBlock0->GetControllerByID(glTF_emissionColor);
			CreateEmissiveFactorAnimation(pC, m.second);
			//pC = ConvertColorToFloatController(pC, 0);
			//CreateEmissiveStrengthAnimation(pC, m.second);

			onoff = pBlock1->GetInt(glTF_enableTransmission);
			if (onoff) {
				pC = pBlock1->GetControllerByID(glTF_transmission);
				CreateTransmissionAnimation(pC, m.second);
			}
			onoff = pBlock1->GetInt(glTF_enableIndexOfRefraction);
			if (onoff) {
				pC = pBlock1->GetControllerByID(glTF_indexOfRefraction);
				CreateIORAnimation(pC, m.second);
			}
			onoff = pBlock1->GetInt(glTF_enableVolume);
			if (onoff) {
				pC = pBlock1->GetControllerByID(glTF_volumeThickness);
				CreateThicknessFactorAnimation(pC, m.second);
				pC = pBlock1->GetControllerByID(glTF_volumeDistance);
				CreateAttenuationDistanceAnimation(pC, m.second);
				pC = pBlock1->GetControllerByID(glTF_volumeColor);
				CreateAttenuationColorAnimation(pC, m.second);
			}

			onoff = pBlock1->GetInt(glTF_enableClearcoat);
			if (onoff) {
				pC = pBlock1->GetControllerByID(glTF_clearcoat);
				CreateClearcoatFactorAnimation(pC, m.second);
				pC = pBlock1->GetControllerByID(glTF_clearcoatRoughness);
				CreateClearcoatRoughnessAnimation(pC, m.second);
			}

			onoff = pBlock1->GetInt(glTF_enableSheen);
			if (onoff) {
				pC = pBlock1->GetControllerByID(glTF_sheenColor);
				CreateSheenColorAnimation(pC, m.second);
				pC = pBlock1->GetControllerByID(glTF_sheenRoughness);
				CreateSheenRoughnessAnimation(pC, m.second);
			}
		}
		else  if (pMtl->ClassID() == USDMaterialID) {
			IParamBlock2* pBlock = pMtl->GetParamBlockByID(1);
			Texmap* pTex = pBlock->GetTexmap(usd_diffuseColor_map);
			CreateUVAnimation(pTex, m.second, TargetTex::BaseColorMap);

			Control* pC = pBlock->GetControllerByID(usd_diffuseColor);
			CreateBaseColorAnimation(pC, m.second, TRUE);

			pC = pBlock->GetControllerByID(usd_metallic);
			CreateMetalicFactorAnimation(pC, m.second);

			pC = pBlock->GetControllerByID(usd_roughness);
			CreateRoughnessFactorAnimation(pC, m.second);

			pC = pBlock->GetControllerByID(usd_normal);
			CreateNormalScaleAnimation(pC, m.second);

			pC = pBlock->GetControllerByID(usd_occlusion);
			CreateOcclusionStrengthAnimation(pC, m.second);

			pC = pBlock->GetControllerByID(usd_ior);
			CreateIORAnimation(pC, m.second);

			pC = pBlock->GetControllerByID(usd_opacityThreshold);
			CreateAlphaCutOffAnimation(pC, m.second);

			pC = pBlock->GetControllerByID(usd_emissiveColor);
			CreateEmissiveFactorAnimation(pC, m.second);

			pC = pBlock->GetControllerByID(usd_clearcoat);
			CreateClearcoatFactorAnimation(pC, m.second);
			pC = pBlock->GetControllerByID(usd_clearcoatRoughness);
			CreateClearcoatRoughnessAnimation(pC, m.second);
		}
		else  if (pMtl->ClassID() == Arnold_StandardSufaceID) {
			IParamBlock2* pBlock = pMtl->GetParamBlockByID(1);
			Texmap* pTex = pBlock->GetTexmap(an_sf_base_color_shader);
			CreateUVAnimation(pTex, m.second, TargetTex::BaseColorMap);
			pTex = pBlock->GetTexmap(an_sf_metalness_shader);
			CreateUVAnimation(pTex, m.second, TargetTex::MetalnessMap);
			pTex = pBlock->GetTexmap(an_sf_diffuse_roughness_shader);
			CreateUVAnimation(pTex, m.second, TargetTex::RoughnessMap);
			pTex = pBlock->GetTexmap(an_sf_emission_shader);
			CreateUVAnimation(pTex, m.second, TargetTex::EmissiveMap);
			pTex = pBlock->GetTexmap(an_sf_normal_shader);
			CreateUVAnimation(pTex, m.second, TargetTex::NormalMap);

			Control* pC = pBlock->GetControllerByID(an_sf_base_color);
			CreateBaseColorAnimation(pC, m.second, TRUE);

			pC = pBlock->GetControllerByID(an_sf_transmission);
			CreateTransmissionAnimation(pC, m.second);

			pC = pBlock->GetControllerByID(an_sf_metalness);
			CreateMetalicFactorAnimation(pC, m.second);

			pC = pBlock->GetControllerByID(an_sf_specular_roughness);
			CreateRoughnessFactorAnimation(pC, m.second);

			pC = pBlock->GetControllerByID(an_sf_emission_color);
			CreateEmissiveFactorAnimation(pC, m.second);

			pC = pBlock->GetControllerByID(an_sf_emission);
			CreateEmissiveStrengthAnimation(pC, m.second);

			pC = pBlock->GetControllerByID(an_sf_specular_IOR);
			CreateIORAnimation(pC, m.second);

			pC = pBlock->GetControllerByID(an_sf_transmission_color);
			CreateAttenuationColorAnimation(pC, m.second);

			pTex = pBlock->GetTexmap(an_sf_opacity_shader, 0);
			if (GetOSLMapType(pTex) == OSL_CutOff) {
				IParamBlock2* pBlock = pTex->GetParamBlock(1);
				Control* pC = pBlock->GetControllerByID(0);
				CreateAlphaCutOffAnimation(pC, m.second);
			}

			pTex = pBlock->GetTexmap(an_sf_normal_shader, 0);
			if (pTex) {
				if (pTex->ClassID() == ArnoldNormalMapID) {
					IParamBlock2* pBlock = pTex->GetParamBlock(1);
					Control* pC = pBlock->GetControllerByID(15);
					CreateNormalScaleAnimation(pC, m.second);
				}
			}

		}
		else  if (pMtl->ClassID() == VRayMaterialID) {
			Texmap* pTex = NULL;
			IParamBlock2* pBlock = pMtl->GetParamBlockByID(1);
			pBlock->GetValueByName(vr_texmap_diffuse, m_time, pTex, FOREVER);
			if (pTex) {
				if (pTex->ClassID() == VRayCompTexID) {
					pTex->GetParamBlock(0)->GetValue(1, m_time, pTex, FOREVER, 0);
				}
				CreateUVAnimation(pTex, m.second, TargetTex::BaseColorMap);
			}
			pBlock = pMtl->GetParamBlock(0);
			Control* pC = pBlock->GetControllerByID(vr_diffuse);
			CreateBaseColorAnimation(pC, m.second, TRUE);

			pC = pBlock->GetControllerByID(vr_reflection_metalness);
			CreateMetalicFactorAnimation(pC, m.second);

			IParamBlock2* pBlockEx = GetCustAttrPBlock(pMtl, tstring(_T("VRay Extention")));
			if (pBlockEx) {
				//pC = pBlock->GetControllerByID(vr_diffuse_roughness);
				pC = pBlockEx->GetControllerByID(1);
				CreateRoughnessFactorAnimation(pC, m.second);
			}

			pC = pBlock->GetControllerByID(vr_selfIllumination);
			CreateEmissiveFactorAnimation(pC, m.second);

			pC = pBlock->GetControllerByID(vr_selfIllumination_multiplier);
			CreateEmissiveStrengthAnimation(pC, m.second);

			pC = pBlock->GetControllerByID(vr_translucency_color);
			CreateAttenuationColorAnimation(pC, m.second);
			pC = pBlock->GetControllerByID(vr_translucency_fbCoeff);
			CreateAttenuationDistanceAnimation(pC, m.second);
			pC = pBlock->GetControllerByID(vr_translucency_thickness);
			CreateThicknessFactorAnimation(pC, m.second);

			pTex = pMtl->GetParamBlockByID(4)->GetTexmap(vr_texmap_opacity, 0);
			if (GetOSLMapType(pTex) == OSL_CutOff) {
				IParamBlock2* pBlock = pTex->GetParamBlock(1);
				Control* pC = pBlock->GetControllerByID(0);
				CreateAlphaCutOffAnimation(pC, m.second);
			}
		}
		else  if (pMtl->ClassID() == CoronaMaterialID) {
		}
		else  if (pMtl->ClassID() == Pencil4MaterialID) {
		}

		//-------------------------------
		Control* pC = NULL;
		IParamBlock2* pBlock = GetCustAttrPBlock(pMtl, tstring(_T("Transmission")));
		if (pBlock) {
			if (pBlock->GetInt(1)) {
				pC = pBlock->GetControllerByID(2);
				if(CreateTransmissionAnimation(pC, m.second))
					m_AnimationPointer_Used = TRUE;

				Texmap* pTex = pBlock->GetTexmap(3);
				CreateUVAnimation(pTex, m.second, TargetTex::TransmissionMap);
			}
		}
		pBlock = GetCustAttrPBlock(pMtl, tstring(_T("IOR")));
		if (pBlock) {
			if (pBlock->GetInt(1)) {
				pC = pBlock->GetControllerByID(2);
				if(CreateIORAnimation(pC, m.second))
					m_AnimationPointer_Used = TRUE;
			}
		}
		pBlock = GetCustAttrPBlock(pMtl, tstring(_T("EmissiveStrength")));
		if (pBlock) {
			if (pBlock->GetInt(1)) {
				pC = pBlock->GetControllerByID(2);
				if(CreateEmissiveStrengthAnimation(pC, m.second))
					m_AnimationPointer_Used = TRUE;
			}
		}
		pBlock = GetCustAttrPBlock(pMtl, tstring(_T("Volume")));
		if (pBlock) {
			if (pBlock->GetInt(1)) {
				pC = pBlock->GetControllerByIndex(2);
				if (CreateThicknessFactorAnimation(pC, m.second))
					m_AnimationPointer_Used = TRUE;

				pC = pBlock->GetControllerByID(3);
				if (CreateAttenuationDistanceAnimation(pC, m.second))
					m_AnimationPointer_Used = TRUE;

				pC = pBlock->GetControllerByID(4);
				if (CreateAttenuationColorAnimation(pC, m.second))
					m_AnimationPointer_Used = TRUE;

				Texmap* pTex = pBlock->GetTexmap(5);
				CreateUVAnimation(pTex, m.second, TargetTex::VolumeThicknessMap);
			}
		}
		pBlock = GetCustAttrPBlock(pMtl, tstring(_T("Iridescence")));
		if (pBlock) {
			if (pBlock->GetInt(1)) {
				pC = pBlock->GetControllerByIndex(2);
				if (CreateIridescenceFactorAnimation(pC, m.second))
					m_AnimationPointer_Used = TRUE;

				pC = pBlock->GetControllerByID(3);
				if (CreateIridescenceIorAnimation(pC, m.second))
					m_AnimationPointer_Used = TRUE;

				pC = pBlock->GetControllerByID(4);
				if (CreateIridescenceThicknessMinAnimation(pC, m.second)) m_AnimationPointer_Used = TRUE;

				pC = pBlock->GetControllerByID(5);
				if (CreateIridescenceThicknessMaxAnimation(pC, m.second)) m_AnimationPointer_Used = TRUE;

				Texmap* pTex = pBlock->GetTexmap(6);
				CreateUVAnimation(pTex, m.second, TargetTex::IridescenceMap);
				pTex = pBlock->GetTexmap(7);
				CreateUVAnimation(pTex, m.second, TargetTex::IridescenceThicknessMap);
			}
		}
		pBlock = GetCustAttrPBlock(pMtl, tstring(_T("Sheen")));
		if (pBlock) {
			if (pBlock->GetInt(1)) {
				pC = pBlock->GetControllerByIndex(2);
				if (CreateSheenColorAnimation(pC, m.second))
					m_AnimationPointer_Used = TRUE;

				pC = pBlock->GetControllerByID(4);
				if (CreateSheenRoughnessAnimation(pC, m.second))
					m_AnimationPointer_Used = TRUE;

				Texmap* pTex = pBlock->GetTexmap(3);
				CreateUVAnimation(pTex, m.second, TargetTex::SheenColorMap);
				pTex = pBlock->GetTexmap(5);
				CreateUVAnimation(pTex, m.second, TargetTex::SheenRoughnessMap);
			}
		}
		pBlock = GetCustAttrPBlock(pMtl, tstring(_T("Clearcoat")));
		if (pBlock) {
			if (pBlock->GetInt(1)) {
				pC = pBlock->GetControllerByIndex(2);
				if (CreateClearcoatFactorAnimation(pC, m.second))	m_AnimationPointer_Used = TRUE;
	;
				pC = pBlock->GetControllerByID(4);
				if (CreateClearcoatRoughnessAnimation(pC, m.second)) m_AnimationPointer_Used = TRUE;

				Texmap* pTex = pBlock->GetTexmap(3);
				CreateUVAnimation(pTex, m.second, TargetTex::ClearcoatMap);
				pTex = pBlock->GetTexmap(5);
				CreateUVAnimation(pTex, m.second, TargetTex::ClearcoatRoughnessMap);
				pTex = pBlock->GetTexmap(6);
				CreateUVAnimation(pTex, m.second, TargetTex::ClearcoatNormalMap);
			}
		}
		pBlock = GetCustAttrPBlock(pMtl, tstring(_T("Specular")));
		if (pBlock) {
			if (pBlock->GetInt(1)) {
				pC = pBlock->GetControllerByIndex(2);
				if (CreateClearcoatFactorAnimation(pC, m.second))	m_AnimationPointer_Used = TRUE;
				;
				pC = pBlock->GetControllerByID(2);
				if (CreateSpecularFactorAnimation(pC, m.second)) m_AnimationPointer_Used = TRUE;

				pC = pBlock->GetControllerByID(3);
				if (CreateSpecularColorAnimation(pC, m.second)) m_AnimationPointer_Used = TRUE;

				Texmap* pTex = pBlock->GetTexmap(3);
				CreateUVAnimation(pTex, m.second, TargetTex::ClearcoatMap);
				pTex = pBlock->GetTexmap(3);
				CreateUVAnimation(pTex, m.second, TargetTex::SpecularMap);
				pTex = pBlock->GetTexmap(5);
				CreateUVAnimation(pTex, m.second, TargetTex::SpecularColorMap);
			}
		}

		pBlock = GetCustAttrPBlock(pMtl, tstring(_T("Anisotropy")));
		if (pBlock) {
			if (pBlock->GetInt(1)) {
				pC = pBlock->GetControllerByIndex(2);
				if (CreateAnisotropyStrengthAnimation(pC, m.second)) m_AnimationPointer_Used = TRUE;

				pC = pBlock->GetControllerByIndex(3);
				if (CreateAnisotropyRotationAnimation(pC, m.second)) m_AnimationPointer_Used = TRUE;

				Texmap* pTex = pBlock->GetTexmap(4);
				CreateUVAnimation(pTex, m.second, TargetTex::AnisotropyMap);
			}
		}

		pBlock = GetCustAttrPBlock(pMtl, tstring(_T("Dispersion")));
		if (pBlock) {
			if (pBlock->GetInt(1)) {
				pC = pBlock->GetControllerByIndex(2);
				if (CreateDispersionAnimation(pC, m.second)) m_AnimationPointer_Used = TRUE;
			}
		}

		pBlock = GetCustAttrPBlock(pMtl, tstring(_T("DiffuseTransmission")));
		if (pBlock) {
			if (pBlock->GetInt(1)) {
				pC = pBlock->GetControllerByIndex(2);
				if (CreateDiffTransFactorAnimation(pC, m.second)) m_AnimationPointer_Used = TRUE;

				pC = pBlock->GetControllerByIndex(3);
				if (CreateDiffTransColorAnimation(pC, m.second)) m_AnimationPointer_Used = TRUE;

				Texmap* pTex = pBlock->GetTexmap(5);
				CreateUVAnimation(pTex, m.second, TargetTex::DiffuseTransmissionMap);
				pTex = pBlock->GetTexmap(4);
				CreateUVAnimation(pTex, m.second, TargetTex::DiffuseTransmissionColorMap);
			}
		}

	}

	for (auto n : m_LightMap) {

		GenLight* pLightObj = n.first;
		IParamBlock* pBlock = GetParamBlock(pLightObj, 0);
		if (pBlock) {

			Control *pC = pBlock->GetController(0);
			CreateLightColorAnimation(pC, n.second);

			pC = pBlock->GetController(1);
			CreateLightIntensAnimation(pC, n.second);

			int type = pLightObj->Type();
			if (type == DIR_LIGHT || type == FSPOT_LIGHT) {
				pC = pBlock->GetController(18);
				CreateLightRangeAnimation(pC, n.second);
			}
			if (type == TSPOT_LIGHT || type == FSPOT_LIGHT) {
				pC = pLightObj->GetFalloffControl();
				CreateLightOutAngleAnimation(pC, n.second);

				pC = pLightObj->GetHotSpotControl();
				CreateLightInAngleAnimation(pC, n.second);
			}
		}
	}

	if (m_ParamAnimationTable.size() > 0) {
		m_AnimationPointer_Used = TRUE;
	}
}

//======================================================================
//======================================================================
UINT glTFExporter_Core::IsUVAnimated(Texmap* pSrcTex)
{
	UINT ret = 0x0;

	if (!pSrcTex) return ret;
	if(!m_ExportAnimation) return ret;
	Class_ID cc = pSrcTex->ClassID();

	Texmap* pTex = pSrcTex;
	if (pTex->ClassID() == CompositeTexClassID) {
		pTex = pTex->GetParamBlock(0)->GetTexmap(9);
		if (!pTex) return ret;
	}
	if (pTex->ClassID() == ColorCorrectTexID) {
		pTex = pTex->GetParamBlock(0)->GetTexmap(1);
		if(!pTex) return ret;
	}
	if (pTex->ClassID() == RGBMultiTexID) {
		Texmap *pTex2 = pTex->GetParamBlock(0)->GetTexmap(2);
		if(pTex2)
			pTex = pTex2;
		else
			pTex = pSrcTex->GetParamBlock(0)->GetTexmap(3);
		
		if(!pTex) return ret;
	}

	StdUVGen* pUVGen = NULL;
	if (pTex->ClassID() == VRayBitmapID) {
		pUVGen = GetUVGen(pTex);
	}
	else if (pTex->ClassID() == bmptexClassID) {
		pUVGen = GetUVGen(pTex);
	}
	if (!pUVGen) return ret;

	IParamBlock* pBlock = GetParamBlock(pUVGen, 0);
	Control* pOffsetUC = pBlock->GetController(0);
	if (pOffsetUC) if (pOffsetUC->IsAnimated()) ret |= UV_ANIMATE_OFSET;
	Control* pOffsetVC = pBlock->GetController(1);
	if (pOffsetVC) if (pOffsetVC->IsAnimated()) ret |= UV_ANIMATE_OFSET;
	Control* pScaleUC = pBlock->GetController(2);
	if (pScaleUC) if (pScaleUC->IsAnimated()) ret |= UV_ANIMATE_SCALE;
	Control* pScaleVC = pBlock->GetController(3);
	if (pScaleVC) if (pScaleVC->IsAnimated()) ret |= UV_ANIMATE_SCALE;
	Control* pRotateW = pBlock->GetController(6);
	if (pRotateW)if (pRotateW->IsAnimated()) ret |= UV_ANIMATE_ROTATE;

	return ret;
}
//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateUVAnimation(Texmap *pSrcTex, UINT mtlIdx, TargetTex target)
{
	if (!pSrcTex) return FALSE;

	StdUVGen* pUVGen = NULL;
	Texmap* pTex = GetBitmapTextureRec(pSrcTex);
	if (pTex) {
		pUVGen = GetUVGen(pTex);
	}
	else {
		pUVGen = GetUVGen(pSrcTex);
	}
	if (!pUVGen) return FALSE;

	UINT samplerIdx = m_animation.channels.size();

	IParamBlock* pBlock = GetParamBlock(pUVGen, 0);

	std::list<TimeValue> KeyFrameList1;
	Control* pScaleUC = pBlock->GetController(2);
	Control* pScaleVC = pBlock->GetController(3);
	CreateKeyFrameList(pScaleUC, KeyFrameList1);
	CreateKeyFrameList(pScaleVC, KeyFrameList1, FALSE);
	if (KeyFrameList1.size() > 0) {
		std::string name;
		if(target== TargetTex::BaseColorMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/pbrMetallicRoughness/baseColorTexture/extensions/KHR_texture_transform/scale";
		else if (target == TargetTex::EmissiveMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/emissiveTexture/extensions/KHR_texture_transform/scale";
		else if (target == TargetTex::MetalnessMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/pbrMetallicRoughness/metallicRoughnessTexture/extensions/KHR_texture_transform/scale";
		else if (target == TargetTex::RoughnessMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/pbrMetallicRoughness/metallicRoughnessTexture/extensions/KHR_texture_transform/scale";
		else if (target == TargetTex::NormalMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/normalTexture/extensions/KHR_texture_transform/scale";
		else if (target == TargetTex::OcclusionMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/occlusionTexture/extensions/KHR_texture_transform/scale";

		else if (target == TargetTex::ClearcoatMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_clearcoat/clearcoatTexture/extensions/KHR_texture_transform/scale";
		else if (target == TargetTex::ClearcoatRoughnessMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_clearcoat/clearcoatRoughnessTexture/extensions/KHR_texture_transform/scale";
		else if (target == TargetTex::ClearcoatNormalMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_clearcoat/clearcoatNormalTexture/extensions/KHR_texture_transform/scale";
		else if (target == TargetTex::SheenColorMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_sheen/sheenColorTexture/extensions/KHR_texture_transform/scale";
		else if (target == TargetTex::SheenRoughnessMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_sheen/sheenRoughnessTexture/extensions/KHR_texture_transform/scale";
		else if (target == TargetTex::SpecularMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_specular/specularTexture/extensions/KHR_texture_transform/scale";
		else if (target == TargetTex::SpecularColorMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_specular/specularColorTexture/extensions/KHR_texture_transform/scale";
		else if (target == TargetTex::TransmissionMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_transmission/transmissionTexture/extensions/KHR_texture_transform/scale";
		else if (target == TargetTex::VolumeThicknessMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_volume/thicknessTexture/extensions/KHR_texture_transform/scale";
		else if (target == TargetTex::IridescenceMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_iridescence/iridescenceTexture/extensions/KHR_texture_transform/scale";
		else if (target == TargetTex::IridescenceThicknessMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_iridescence/iridescenceThicknessTexture/extensions/KHR_texture_transform/scale";
		else if (target == TargetTex::AnisotropyMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_anisotropy/anisotropyTexture/extensions/KHR_texture_transform/scale";
		else if (target == TargetTex::DiffuseTransmissionMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_diffuse_transmission/diffuseTransmissionTexture/extensions/KHR_texture_transform/scale";
		else if (target == TargetTex::DiffuseTransmissionColorMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_diffuse_transmission/diffuseTransmissionColorTexture/extensions/KHR_texture_transform/scale";

		KeyFrameList1.sort();
		SetVec2Animation(KeyFrameList1, pScaleUC, pScaleVC, name);
	}

	Control* pRotateW = pBlock->GetController(6);
	CreateKeyFrameList(pRotateW, KeyFrameList1);
	if (KeyFrameList1.size() > 0) {
		std::string name;
		if (target == TargetTex::BaseColorMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/pbrMetallicRoughness/baseColorTexture/extensions/KHR_texture_transform/rotation";
		else if (target == TargetTex::EmissiveMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/emissiveTexture/extensions/KHR_texture_transform/rotation";
		else if (target == TargetTex::MetalnessMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/pbrMetallicRoughness/metallicRoughnessTexture/extensions/KHR_texture_transform/rotation";
		else if (target == TargetTex::RoughnessMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/pbrMetallicRoughness/metallicRoughnessTexture/extensions/KHR_texture_transform/rotation";
		else if (target == TargetTex::NormalMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/normalTexture/extensions/KHR_texture_transform/rotation";
		else if (target == TargetTex::OcclusionMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/occlusionTexture/extensions/KHR_texture_transform/rotation";

		else if (target == TargetTex::ClearcoatMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_clearcoat/clearcoatTexture/extensions/KHR_texture_transform/rotation";
		else if (target == TargetTex::ClearcoatRoughnessMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_clearcoat/clearcoatRoughnessTexture/extensions/KHR_texture_transform/rotation";
		else if (target == TargetTex::ClearcoatNormalMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_clearcoat/clearcoatNormalTexture/extensions/KHR_texture_transform/rotation";
		else if (target == TargetTex::SheenColorMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_sheen/sheenColorTexture/extensions/KHR_texture_transform/rotation";
		else if (target == TargetTex::SheenRoughnessMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_sheen/sheenRoughnessTexture/extensions/KHR_texture_transform/rotation";
		else if (target == TargetTex::SpecularMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_specular/specularTexture/extensions/KHR_texture_transform/rotation";
		else if (target == TargetTex::SpecularColorMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_specular/specularColorTexture/extensions/KHR_texture_transform/rotation";
		else if (target == TargetTex::TransmissionMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_transmission/transmissionTexture/extensions/KHR_texture_transform/rotation";
		else if (target == TargetTex::VolumeThicknessMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_volume/thicknessTexture/extensions/KHR_texture_transform/rotation";
		else if (target == TargetTex::IridescenceMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_iridescence/iridescenceTexture/extensions/KHR_texture_transform/rotation";
		else if (target == TargetTex::IridescenceThicknessMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_iridescence/iridescenceThicknessTexture/extensions/KHR_texture_transform/rotation";
		else if (target == TargetTex::AnisotropyMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_anisotropy/anisotropyTexture/extensions/KHR_texture_transform/rotation";
		else if (target == TargetTex::DiffuseTransmissionMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_diffuse_transmission/diffuseTransmissionTexture/extensions/KHR_texture_transform/rotation";
		else if (target == TargetTex::DiffuseTransmissionColorMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_diffuse_transmission/diffuseTransmissionColorTexture/extensions/KHR_texture_transform/rotation";

		KeyFrameList1.sort();
		SetFloatAnimation(KeyFrameList1, pRotateW, name);

		m_TexTransform_Used = TRUE;
	}

	Control* pOffsetUC = pBlock->GetController(0);
	Control* pOffsetVC = pBlock->GetController(1);
	CreateKeyFrameList(pOffsetUC, KeyFrameList1);
	CreateKeyFrameList(pOffsetVC, KeyFrameList1, FALSE);

	AnimateOn();

	Control* pOfsU2C = (Control*)GetCOREInterface()->CreateInstance(CTRL_FLOAT_CLASS_ID, Class_ID(0x2007, 0x0));
	Control* pOfsV2C = (Control*)GetCOREInterface()->CreateInstance(CTRL_FLOAT_CLASS_ID, Class_ID(0x2007, 0x0));
	for (auto t : KeyFrameList1) {
		float ofsetU = pUVGen->GetUOffs(t);
		float ofsetV = pUVGen->GetVOffs(t);
		float sclU = pUVGen->GetUScl(t);
		float sclV = pUVGen->GetVScl(t);
		float rot = pUVGen->GetWAng(t);

		float localoffsetU = -0.5f * cos(rot) + 0.5f * sin(rot) + 0.5f;
		float localoffsetV = -0.5f * sin(rot) - 0.5f * cos(rot) + 0.5f;
		//localoffsetU += (1.0f - (1.0f / sclU)) / 2.0f;
		//localoffsetV += (1.0f - (1.0f / sclV)) / 2.0f;

		if (sclU >= 1.0f) {
			localoffsetU += (1.0f - (1.0f / sclU)) / 2.0f;
			ofsetU += localoffsetU;
			ofsetU *= -1.0f;
		}
		else {
			localoffsetU += (1.0f - sclU) / 2.0f;
			ofsetU = localoffsetU - ofsetU * sclU;
		}

		if (sclV >= 1.0f) {
			localoffsetV += (1.0f - (1.0f / sclV)) / 2.0f;
			ofsetV -= localoffsetV;
		}
		else {
			localoffsetV += (1.0f - sclV) / 2.0f;
			ofsetV = localoffsetV + ofsetV * sclV;
		}

		pOfsU2C->SetValue(t, &ofsetU);
		pOfsV2C->SetValue(t, &ofsetV);
	}

	AnimateOff();

	if (KeyFrameList1.size() > 0) {
		std::string name;
		if (target == TargetTex::BaseColorMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/pbrMetallicRoughness/baseColorTexture/extensions/KHR_texture_transform/offset";
		else if (target == TargetTex::EmissiveMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/emissiveTexture/extensions/KHR_texture_transform/offset";
		else if (target == TargetTex::MetalnessMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/pbrMetallicRoughness/metallicRoughnessTexture/extensions/KHR_texture_transform/offset";
		else if (target == TargetTex::RoughnessMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/pbrMetallicRoughness/metallicRoughnessTexture/extensions/KHR_texture_transform/offset";
		else if (target == TargetTex::NormalMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/normalTexture/extensions/KHR_texture_transform/offset";
		else if (target == TargetTex::OcclusionMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/occlusionTexture/extensions/KHR_texture_transform/offset";

		else if (target == TargetTex::ClearcoatMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_clearcoat/clearcoatTexture/extensions/KHR_texture_transform/offset";
		else if (target == TargetTex::ClearcoatRoughnessMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_clearcoat/clearcoatRoughnessTexture/extensions/KHR_texture_transform/offset";
		else if (target == TargetTex::ClearcoatNormalMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_clearcoat/clearcoatNormalTexture/extensions/KHR_texture_transform/offset";
		else if (target == TargetTex::SheenColorMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_sheen/sheenColorTexture/extensions/KHR_texture_transform/offset";
		else if (target == TargetTex::SheenRoughnessMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_sheen/sheenRoughnessTexture/extensions/KHR_texture_transform/offset";
		else if (target == TargetTex::SpecularMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_specular/specularTexture/extensions/KHR_texture_transform/offset";
		else if (target == TargetTex::SpecularColorMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_specular/specularColorTexture/extensions/KHR_texture_transform/offset";
		else if (target == TargetTex::TransmissionMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_transmission/transmissionTexture/extensions/KHR_texture_transform/offset";
		else if (target == TargetTex::VolumeThicknessMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_volume/thicknessTexture/extensions/KHR_texture_transform/offset";
		else if (target == TargetTex::IridescenceMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_iridescence/iridescenceTexture/extensions/KHR_texture_transform/offset";
		else if (target == TargetTex::IridescenceThicknessMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_iridescence/iridescenceThicknessTexture/extensions/KHR_texture_transform/offset";
		else if (target == TargetTex::AnisotropyMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_anisotropy/anisotropyTexture/extensions/KHR_texture_transform/offset";
		else if (target == TargetTex::DiffuseTransmissionMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_diffuse_transmission/diffuseTransmissionTexture/extensions/KHR_texture_transform/offset";
		else if (target == TargetTex::DiffuseTransmissionColorMap)
			name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_diffuse_transmission/diffuseTransmissionColorTexture/extensions/KHR_texture_transform/offset";

		KeyFrameList1.sort();
		SetVec2Animation(KeyFrameList1, pOfsU2C, pOfsV2C, name);
		//SetVec2Animation(KeyFrameList1, pOffsetUC, pOffsetVC, name);

		m_TexTransform_Used = TRUE;
	}

	return (KeyFrameList1.size() > 0);
}
//======================================================================
//GetCOREInterface(COLORPIPELINEMGR_INTERFACE)
//======================================================================
BOOL glTFExporter_Core::CreateBaseColorAnimation(Control* pC, UINT mtlIdx, BOOL alpha)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationColor(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	// If all keys have the same value, it is considered not animated.
	if (KeyFrameList.size() > 0) {
		BOOL animate = FALSE;
		Point4 baseC;
		pC->GetValue(*KeyFrameList.begin(), baseC, FOREVER);
		for (auto t : KeyFrameList) {
			Point4 c;
			pC->GetValue(t, c, FOREVER);
			if (c != baseC) {
				animate = TRUE;
				break;;
			}
		}
		if (!animate) return FALSE;
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/pbrMetallicRoughness/baseColorFactor";
		SetColorAnimation(KeyFrameList, pC, name, TRUE);
		return TRUE;
	}

	return FALSE;
}

//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateAlphaCutOffAnimation(Control* pC, UINT mtlIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/alphaCutoff";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}

//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateTransmissionAnimation(Control* pC, UINT mtlIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_transmission/transmissionFactor";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}

//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateIORAnimation(Control* pC, UINT mtlIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_ior/ior";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}

//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateMetalicFactorAnimation(Control* pC, UINT mtlIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/pbrMetallicRoughness/metallicFactor";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}
//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateRoughnessFactorAnimation(Control* pC, UINT mtlIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/pbrMetallicRoughness/roughnessFactor";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}

//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateNormalScaleAnimation(Control* pC, UINT mtlIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/normalTexture/scale";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}

//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateOcclusionStrengthAnimation(Control* pC, UINT mtlIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/occlusionTexture/strength";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}
//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateEmissiveStrengthAnimation(Control* pC, UINT mtlIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_emissive_strength/emissiveStrength";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}
//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateEmissiveFactorAnimation(Control* pC, UINT mtlIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationColor(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/emissiveFactor";
		SetColorAnimation(KeyFrameList, pC, name, FALSE);
		return TRUE;
	}

	return FALSE;
}
//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateThicknessFactorAnimation(Control* pC, UINT mtlIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_volume/thicknessFactor";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}

//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateAttenuationDistanceAnimation(Control* pC, UINT mtlIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_volume/attenuationDistance";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}
//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateAttenuationColorAnimation(Control* pC, UINT mtlIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationColor(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_volume/attenuationColor";
		SetColorAnimation(KeyFrameList, pC, name, FALSE);
		return TRUE;
	}

	return FALSE;
}

//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateIridescenceFactorAnimation(Control* pC, UINT mtlIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_iridescence/iridescenceFactor";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}

//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateIridescenceIorAnimation(Control* pC, UINT mtlIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_iridescence/iridescenceIor";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}

//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateIridescenceThicknessMinAnimation(Control* pC, UINT mtlIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_iridescence/iridescenceThicknessMinimum";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}

//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateIridescenceThicknessMaxAnimation(Control* pC, UINT mtlIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_iridescence/iridescenceThicknessMaximum";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}

//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateSheenColorAnimation(Control* pC, UINT mtlIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationColor(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_sheen/sheenColorFactor";
		SetColorAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}
//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateSheenRoughnessAnimation(Control* pC, UINT mtlIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_sheen/sheenRoughnessFactor";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}
//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateClearcoatFactorAnimation(Control* pC, UINT mtlIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_clearcoat/clearcoatFactor";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}
//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateClearcoatRoughnessAnimation(Control* pC, UINT mtlIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_clearcoat/clearcoatRoughnessFactor";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}

//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateAnisotropyStrengthAnimation(Control* pC, UINT mtlIdx)
{
	if (!pC) return FALSE;

	std::list<TimeValue> KeyFrameList;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_anisotropy/anisotropyStrength";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}
//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateAnisotropyRotationAnimation(Control* pC, UINT mtlIdx)
{
	if (!pC) return FALSE;

	std::list<TimeValue> KeyFrameList;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_anisotropy/anisotropyRotation";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}
//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateDispersionAnimation(Control* pC, UINT mtlIdx)
{
	if (!pC) return FALSE;

	std::list<TimeValue> KeyFrameList;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_dispersion/dispersion";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}
//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateDiffTransFactorAnimation(Control* pC, UINT mtlIdx)
{
	if (!pC) return FALSE;

	std::list<TimeValue> KeyFrameList;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_diffuse_transmission/diffuseTransmissionFactor";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}
//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateDiffTransColorAnimation(Control* pC, UINT mtlIdx)
{
	if (!pC) return FALSE;

	std::list<TimeValue> KeyFrameList;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_diffuse_transmission/diffuseTransmissionColorFactor";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}
//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateSpecularFactorAnimation(Control* pC, UINT mtlIdx)
{
	if (!pC) return FALSE;

	std::list<TimeValue> KeyFrameList;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_specular/specularFactor";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}
//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateSpecularColorAnimation(Control* pC, UINT mtlIdx)
{
	if (!pC) return FALSE;

	std::list<TimeValue> KeyFrameList;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/materials/" + std::to_string(mtlIdx) + "/extensions/KHR_materials_specular/specularColorFactor";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}

//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateLightColorAnimation(Control* pC, UINT nodeIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationColor(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/extensions/KHR_lights_punctual/lights/" + std::to_string(nodeIdx) + "/color";
		SetColorAnimation(KeyFrameList, pC, name, FALSE);
		return TRUE;
	}

	return FALSE;
}
//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateLightIntensAnimation(Control* pC, UINT nodeIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/extensions/KHR_lights_punctual/lights/" + std::to_string(nodeIdx) + "/intensity";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}
//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateLightRangeAnimation(Control* pC, UINT nodeIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/extensions/KHR_lights_punctual/lights/" + std::to_string(nodeIdx) + "/range";
		SetFloatAnimation(KeyFrameList, pC, name);
		return TRUE;
	}

	return FALSE;
}
//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateLightOutAngleAnimation(Control* pC, UINT nodeIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/extensions/KHR_lights_punctual/lights/" + std::to_string(nodeIdx) + "/spot/outerConeAngle";
		SetFloatAnimation(KeyFrameList, pC, name, DEG_TO_RAD);
		return TRUE;
	}

	return FALSE;
}
//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateLightInAngleAnimation(Control* pC, UINT nodeIdx)
{
	std::list<TimeValue> KeyFrameList;

	if (!pC) return FALSE;
	if (m_FullFrame) {
		GetFullFrameAnimationFloat(pC, KeyFrameList);
	}
	else {
		if (!pC->IsAnimated()) return FALSE;
		CreateKeyFrameList(pC, KeyFrameList);
	}

	UINT samplerIdx = m_animation.channels.size();

	if (KeyFrameList.size() > 0) {
		std::string name;
		name = "/extensions/KHR_lights_punctual/lights/" + std::to_string(nodeIdx) + "/spot/innerConeAngle";
		SetFloatAnimation(KeyFrameList, pC, name, DEG_TO_RAD);
		return TRUE;
	}

	return FALSE;
}



//======================================================================
//======================================================================
void glTFExporter_Core::SetFloatAnimation(std::list<TimeValue>& KeyFrameList, Control* pC, std::string& name, float scale)
{
	UINT mod = m_BufferByteOffset % 4;
	if (mod) SecureMemory(4 - mod);

	tinygltf::AnimationSampler sampler;// = Create_glTFAnimSampler();
	sampler.interpolation = "LINEAR";

	tinygltf::Accessor accIn;
	accIn.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
	accIn.type = TINYGLTF_TYPE_SCALAR;
	accIn.count = KeyFrameList.size();

	tinygltf::BufferView bfViewIn;
	bfViewIn.buffer = 0;
	bfViewIn.byteOffset = m_BufferByteOffset;
	bfViewIn.byteLength = KeyFrameList.size() * sizeof(float);

	void* ptr = SecureMemory(bfViewIn.byteLength);
	float* pFrame = (float*)((char*)ptr + bfViewIn.byteOffset);
	float time_min = (float)*KeyFrameList.begin() / m_TimeScale;
	float time_max = (float)*KeyFrameList.begin() / m_TimeScale;
	for (auto t : KeyFrameList) {
		float fr = (float)t / m_TimeScale;
		if (time_min > fr) time_min = fr;
		if (time_max < fr) time_max = fr;
		*pFrame++ = fr;
	}
	accIn.maxValues.push_back(time_max);
	accIn.minValues.push_back(time_min);

	m_model.bufferViews.push_back(bfViewIn);
	accIn.bufferView = m_model.bufferViews.size() - 1;
	m_model.accessors.push_back(accIn);
	sampler.input = m_model.accessors.size() - 1;

	tinygltf::Accessor accOut;
	accOut.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
	accOut.type = TINYGLTF_TYPE_SCALAR;
	accOut.count = KeyFrameList.size();

	tinygltf::BufferView bfViewOut;
	bfViewOut.buffer = 0;
	bfViewOut.byteOffset = m_BufferByteOffset;
	bfViewOut.byteLength = accOut.count * sizeof(float) * 1;

	float min_value;
	pC->GetValue(*KeyFrameList.begin(), &min_value, FOREVER);
	float max_value = min_value;

	ptr = SecureMemory(bfViewOut.byteLength);
	float* pData = (float*)((char*)ptr + bfViewOut.byteOffset);
	for (auto fr : KeyFrameList) {
		float ff;
		pC->GetValue(fr, &ff, FOREVER);
		*pData++ = ff * scale;
		if (min_value > ff) min_value = ff;
		if (max_value < ff) max_value = ff;
	}
	accOut.minValues.push_back(min_value);
	accOut.maxValues.push_back(max_value);

	m_model.bufferViews.push_back(bfViewOut);
	accOut.bufferView = m_model.bufferViews.size() - 1;
	m_model.accessors.push_back(accOut);
	sampler.output = m_model.accessors.size() - 1;
	m_animation.samplers.push_back(sampler);

	tinygltf::AnimationChannel channel;// = Create_glTFAnimChannel();
	channel.target_path = "pointer";
	channel.sampler = m_animation.samplers.size() - 1;

	tinygltf::Value::Object o;
	o.insert(std::make_pair("pointer", tinygltf::Value(name)));

	tinygltf::Value::Object obj;
	obj.insert(std::make_pair("pointer", tinygltf::Value(std::string(""))));

	tinygltf::Value::Object extension;
	extension.insert(std::make_pair("KHR_animation_pointer", tinygltf::Value(o)));

	channel.target_extensions = extension;

	m_animation.channels.push_back(channel);

	m_AnimationPointer_Used = TRUE;
}

//======================================================================
//======================================================================
void glTFExporter_Core::SetVec2Animation(std::list<TimeValue> &KeyFrameList, Control *pC1, Control* pC2, std::string& name)
{
	UINT mod = m_BufferByteOffset % 4;
	if (mod) SecureMemory(4 - mod);

	tinygltf::AnimationSampler sampler;// = Create_glTFAnimSampler();
	sampler.interpolation = "LINEAR";

	tinygltf::Accessor accIn;
	accIn.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
	accIn.type = TINYGLTF_TYPE_SCALAR;
	accIn.count = KeyFrameList.size();

	tinygltf::BufferView bfViewIn;
	bfViewIn.buffer = 0;
	bfViewIn.byteOffset = m_BufferByteOffset;
	bfViewIn.byteLength = KeyFrameList.size() * sizeof(float);

	void* ptr = SecureMemory(bfViewIn.byteLength);
	float* pFrame = (float*)((char*)ptr + bfViewIn.byteOffset);
	float time_min = (float)*KeyFrameList.begin() / m_TimeScale;
	float time_max = (float)*KeyFrameList.begin() / m_TimeScale;
	for (auto t :KeyFrameList) {
		float fr = (float)t / m_TimeScale;
		if (time_min > fr) time_min = fr;
		if (time_max < fr) time_max = fr;
		*pFrame++ = fr;
	}
	accIn.maxValues.push_back(time_max);
	accIn.minValues.push_back(time_min);

	m_model.bufferViews.push_back(bfViewIn);
	accIn.bufferView = m_model.bufferViews.size() - 1;
	m_model.accessors.push_back(accIn);
	sampler.input = m_model.accessors.size() - 1;

	tinygltf::Accessor accOut;
	accOut.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
	accOut.type = TINYGLTF_TYPE_VEC2;
	accOut.count = KeyFrameList.size();

	tinygltf::BufferView bfViewOut;
	bfViewOut.buffer = 0;
	bfViewOut.byteOffset = m_BufferByteOffset;
	bfViewOut.byteLength = accOut.count * sizeof(float) * 2;

	Point2 min_p;
	pC1->GetValue(*KeyFrameList.begin(), &min_p.x, FOREVER);
	pC2->GetValue(*KeyFrameList.begin(), &min_p.y, FOREVER);
	Point2 max_p = min_p;

	ptr = SecureMemory(bfViewOut.byteLength);
	float* pData = (float*)((char*)ptr + bfViewOut.byteOffset);
	for (auto fr : KeyFrameList) {
		float u, v;
		pC1->GetValue(fr, &u, FOREVER);
		pC2->GetValue(fr, &v, FOREVER);
		*pData++ = u;
		*pData++ = v;
		if (min_p.x > u) min_p.x = u;
		if (min_p.y > v) min_p.y = v;
		if (max_p.x < u) max_p.x = u;
		if (max_p.y < v) max_p.y = v;
	}
	accOut.minValues.push_back(min_p.x);
	accOut.minValues.push_back(min_p.y);
	accOut.maxValues.push_back(max_p.x);
	accOut.maxValues.push_back(max_p.y);

	m_model.bufferViews.push_back(bfViewOut);
	accOut.bufferView = m_model.bufferViews.size() - 1;
	m_model.accessors.push_back(accOut);
	sampler.output = m_model.accessors.size() - 1;
	m_animation.samplers.push_back(sampler);

	tinygltf::AnimationChannel channel;// = Create_glTFAnimChannel();
	channel.target_path = "pointer";
	channel.sampler = m_animation.samplers.size() - 1;

	tinygltf::Value::Object o;
	o.insert(std::make_pair("pointer", tinygltf::Value(name)));

	tinygltf::Value::Object obj;
	obj.insert(std::make_pair("pointer", tinygltf::Value(std::string(""))));

	tinygltf::Value::Object extension;
	extension.insert(std::make_pair("KHR_animation_pointer", tinygltf::Value(o)));

	channel.target_extensions = extension;

	m_animation.channels.push_back(channel);

	m_AnimationPointer_Used = TRUE;
}

//======================================================================
//======================================================================
void glTFExporter_Core::SetColorAnimation(std::list<TimeValue>& KeyFrameList, Control* pC, std::string& name, BOOL alpha)
{
	UINT mod = m_BufferByteOffset % 4;
	if (mod) SecureMemory(4 - mod);

	tinygltf::AnimationSampler sampler;// = Create_glTFAnimSampler();
	sampler.interpolation = "LINEAR";

	tinygltf::Accessor accIn;
	accIn.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
	accIn.type = TINYGLTF_TYPE_SCALAR;
	accIn.count = KeyFrameList.size();

	tinygltf::BufferView bfViewIn;
	bfViewIn.buffer = 0;
	bfViewIn.byteOffset = m_BufferByteOffset;
	bfViewIn.byteLength = KeyFrameList.size() * sizeof(float);

	void* ptr = SecureMemory(bfViewIn.byteLength);
	float* pFrame = (float*)((char*)ptr + bfViewIn.byteOffset);
	float time_min = (float)*KeyFrameList.begin() / m_TimeScale;
	float time_max = (float)*KeyFrameList.begin() / m_TimeScale;
	for (auto t : KeyFrameList) {
		float fr = (float)t / m_TimeScale;
		if (time_min > fr) time_min = fr;
		if (time_max < fr) time_max = fr;
		*pFrame++ = fr;
	}
	accIn.maxValues.push_back(time_max);
	accIn.minValues.push_back(time_min);

	m_model.bufferViews.push_back(bfViewIn);
	accIn.bufferView = m_model.bufferViews.size() - 1;
	m_model.accessors.push_back(accIn);
	sampler.input = m_model.accessors.size() - 1;

	tinygltf::Accessor accOut;
	accOut.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
	accOut.type = (alpha)?TINYGLTF_TYPE_VEC4: TINYGLTF_TYPE_VEC3;
	accOut.count = KeyFrameList.size();

	tinygltf::BufferView bfViewOut;
	bfViewOut.buffer = 0;
	bfViewOut.byteOffset = m_BufferByteOffset;
	bfViewOut.byteLength = accOut.count * sizeof(float) * (alpha?4:3);

	AColor min_p;
	pC->GetValue(*KeyFrameList.begin(), &min_p, FOREVER);
	AColor max_p = min_p;

	ptr = SecureMemory(bfViewOut.byteLength);
	float* pData = (float*)((char*)ptr + bfViewOut.byteOffset);
	for (auto fr : KeyFrameList) {
		AColor col;
		pC->GetValue(fr, &col, FOREVER);
		*pData++ = col.r;
		*pData++ = col.g;
		*pData++ = col.b;
		if(alpha) *pData++ = col.a;
		if (min_p.r > col.r) min_p.r = col.r;
		if (min_p.g > col.g) min_p.g = col.g;
		if (min_p.b > col.b) min_p.b = col.b;
		if (min_p.a > col.a) min_p.a = col.a;
		if (max_p.r < col.r) max_p.r = col.r;
		if (max_p.g < col.g) max_p.g = col.g;
		if (max_p.b < col.b) max_p.b = col.b;
		if (max_p.a < col.a) max_p.a = col.a;
	}
	accOut.minValues.push_back(min_p.r);
	accOut.minValues.push_back(min_p.g);
	accOut.minValues.push_back(min_p.b);
	accOut.maxValues.push_back(max_p.r);
	accOut.maxValues.push_back(max_p.g);
	accOut.maxValues.push_back(max_p.b);
	if (alpha) {
		accOut.minValues.push_back(min_p.a);
		accOut.maxValues.push_back(max_p.a);
	}

	m_model.bufferViews.push_back(bfViewOut);
	accOut.bufferView = m_model.bufferViews.size() - 1;
	m_model.accessors.push_back(accOut);
	sampler.output = m_model.accessors.size() - 1;
	m_animation.samplers.push_back(sampler);

	tinygltf::AnimationChannel channel;// = Create_glTFAnimChannel();
	channel.target_path = "pointer";
	channel.sampler = m_animation.samplers.size() - 1;

	tinygltf::Value::Object o;
	o.insert(std::make_pair("pointer", tinygltf::Value(name)));

	tinygltf::Value::Object obj;
	obj.insert(std::make_pair("pointer", tinygltf::Value(std::string(""))));

	tinygltf::Value::Object extension;
	extension.insert(std::make_pair("KHR_animation_pointer", tinygltf::Value(o)));

	channel.target_extensions = extension;

	m_animation.channels.push_back(channel);

	m_AnimationPointer_Used = TRUE;
}

/*
//======================================================================
//======================================================================
Control* ConvertColorToFloatController(Control* pSrcC, UINT ch)
{
	if (!pSrcC) return NULL;

	Control* pDstC = (Control*)GetCOREInterface()->CreateInstance(CTRL_FLOAT_CLASS_ID, Class_ID(0x2007, 0x0));
	IKeyControl* ikeys = GetKeyControlInterface(pSrcC);
	for (int i = 0; i < ikeys->GetNumKeys(); i++) {
		IBezPoint4Key key;
		ikeys->GetKey(i, &key);
		TimeValue t = key.time;
		Point4 val = key.val;
		pDstC->SetValue(key.time, &val.x);
	}
	return pDstC;
}
*/