
#include "HSglTFExporter.h"

//----------------------------------------------------------
//----------------------------------------------------------
void glTFExporter_Core::PencilMaterial(MtlBase* pMtl, tinygltf::Material& material)
{
	BOOL animated = FALSE;

	IParamBlock2* pBlock0 = pMtl->GetParamBlock(0);
	Mtl* pBaseMtl = pBlock0->GetMtl(pen_basicMaterial);
	if (!pBaseMtl) {
		return;
	}

	Color c = ((StdMat*)pBaseMtl)->GetDiffuse(0);
	material.pbrMetallicRoughness.baseColorFactor[0] = c.r;
	material.pbrMetallicRoughness.baseColorFactor[1] = c.g;
	material.pbrMetallicRoughness.baseColorFactor[2] = c.b;
	material.pbrMetallicRoughness.baseColorFactor[3] = 1.0f;

	Texmap* pTex = pBaseMtl->GetSubTexmap(ID_DI);
	if (!pTex) {
		ReferenceTarget* pZone0;
		pBlock0->GetValue(pen_Zones, m_time, pZone0, FOREVER, 0);
		IParamBlock2* pZoneBlock0 = pZone0->GetParamBlock(0);
		pTex = pZoneBlock0->GetTexmap(pen_Zone_colorMap);
	}

	if (pTex) {
		if (pTex->ClassID() == CompositeTexClassID) {
			pTex->GetParamBlock(0)->GetValue(9, m_time, pTex, FOREVER);
		}
		CreateBaseColorTexture(material, pTex);
	}
	else {
		material.pbrMetallicRoughness.baseColorTexture.index = -1;
	}

	pTex = pBaseMtl->GetSubTexmap(ID_BU);
	if (pTex) {
		float scale = 1.0f;
		if (pTex->ClassID() == NormalBumpMapClassID) {
			IParamBlock2* p = pTex->GetParamBlockByID(0);
			p->GetValue(0, m_time, scale, FOREVER);
			p->GetValue(2, m_time, pTex, FOREVER);
		}
		if (pTex) CreateNormalTexture(material, pTex, scale);
	}
	else {
		material.normalTexture.index = -1;
	}

	pTex = pBaseMtl->GetSubTexmap(ID_OP);
	if (pTex) {
		if (pTex->ClassID() == OSLTex_CLASS_ID) {
			float value;
			IParamBlock2* pPBlock1 = pTex->GetParamBlock(1);
			pPBlock1->GetValue(0, m_time, value, FOREVER);
			pPBlock1->GetValue(4, m_time, pTex, FOREVER);
			material.alphaMode = "MASK";
			material.alphaCutoff = value;
		}
		else {
			material.alphaMode = "BLEND";
		}
	}
	else {
		material.alphaMode = "OPAQUE";
	}

	material.alphaMode = GetAlphaMode(pBaseMtl, material.alphaMode);

	material.pbrMetallicRoughness.metallicRoughnessTexture.index = -1;
	material.pbrMetallicRoughness.metallicFactor = 0.0f;
	material.pbrMetallicRoughness.roughnessFactor = 0.6f;


	{
		IORStruct str;
		if (SetIORParams(pBaseMtl, str, animated)) {
			CreateIORTexture(material, str, animated);
		}
	}

	{
		UnlitStruct str;
		if (SetUnlitParams(pBaseMtl, str)) {
			CreateUnlitTexture(material, str, animated);
		}
	}
	{
		TransmissionStruct str;
		if (SetTransmissionParams(pBaseMtl, str, animated)) {
			CreateTransmissionTexture(material, str, animated);
		}
	}
	{
		IridescenceStruct str;
		if (SetIridescenceParams(pBaseMtl, str, animated)) {
			CreateIridescenceTexture(material, str, animated);
		}
	}
	{
		VolumeStruct str;
		if (SetVolumeParams(pBaseMtl, str, animated)) {
			CreateVolumeTexture(material, str, animated);
		}
	}

	{
		EmissiveStrengthStruct str;
		if (SetEmissiveStrengthParams(pBaseMtl, str, animated)) {
			CreateEmissiveStrengthTexture(material, str, animated);
		}
	}
}

