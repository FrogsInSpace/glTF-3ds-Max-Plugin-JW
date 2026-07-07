
#include "KHRglTFExporter.h"

//----------------------------------------------------------
//----------------------------------------------------------
void glTFExporter_Core::StdMaterial(MtlBase *pMtl, tinygltf::Material &material)
{
	BOOL animated = FALSE;

	Color c = ((StdMat*)pMtl)->GetDiffuse(0);
	material.pbrMetallicRoughness.baseColorFactor[0] = c.r < 0.0 ? 0.0f : c.r;
	material.pbrMetallicRoughness.baseColorFactor[1] = c.g < 0.0 ? 0.0f : c.g;
	material.pbrMetallicRoughness.baseColorFactor[2] = c.b < 0.0 ? 0.0f : c.b;
	material.pbrMetallicRoughness.baseColorFactor[3] = 1.0f;

	Texmap *pTex = pMtl->GetSubTexmap(ID_DI);
	if (pTex) {
		if (pTex->ClassID() == CompositeTexClassID) {
			pTex->GetParamBlock(0)->GetValue(9, m_time, pTex, FOREVER);
		}
		CreateBaseColorTexture(material, pTex);
	}
	else {
		material.pbrMetallicRoughness.baseColorTexture.index = -1;
	}

	pTex = pMtl->GetSubTexmap(ID_BU);
	if (pTex) {
		float scale = 1.0f;
		if (pTex->ClassID() == NormalBumpMapClassID) {
			IParamBlock2 *p = pTex->GetParamBlockByID(0);
			p->GetValue(0, m_time, scale, FOREVER);
			p->GetValue(2, m_time, pTex, FOREVER);
		}
		if (pTex) CreateNormalTexture(material, pTex, scale);
	}
	else {
		material.normalTexture.index = -1;
	}

	pTex = pMtl->GetSubTexmap(ID_OP);
	if (pTex) {
		if (pTex->ClassID() == OSLTex_CLASS_ID) {
			float value;
			IParamBlock2 *pPBlock1 = pTex->GetParamBlock(1);
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

	material.alphaMode = GetAlphaMode(pMtl, material.alphaMode);

	material.pbrMetallicRoughness.metallicRoughnessTexture.index = -1;
	material.pbrMetallicRoughness.metallicFactor = 0.0f;
	material.pbrMetallicRoughness.roughnessFactor = 0.6f;


	{
		IORStruct str;
		if (SetIORParams(pMtl, str, animated)) {
			CreateIORTexture(material, str, animated);
		}
	} 

	{
		UnlitStruct str;
		if (SetUnlitParams(pMtl, str)) {
			CreateUnlitTexture(material, str, animated);
		}
	}
	{
		TransmissionStruct str;
		if (SetTransmissionParams(pMtl, str, animated)) {
			CreateTransmissionTexture(material, str, animated);
		}
	}
	{
		IridescenceStruct str;
		if (SetIridescenceParams(pMtl, str, animated)) {
			CreateIridescenceTexture(material, str, animated);
		}
	}
	{
		VolumeStruct str;
		if (SetVolumeParams(pMtl, str, animated)) {
			CreateVolumeTexture(material, str, animated);
		}
	}

	{
		EmissiveStrengthStruct str;
		if (SetEmissiveStrengthParams(pMtl, str, animated)) {
			CreateEmissiveStrengthTexture(material, str, animated);
		}
	}

	{
		DispersionStruct str;
		if (SetDispersionParams(pMtl, str, animated)) {
			CreateDispersionTexture(material, str, animated);
		}
	}

	{
		AnisotropyStruct str;
		if (SetAnisotropyParams(pMtl, str, animated)) {
			CreateAnisotropyTexture(material, str, animated);
		}
	}

	{
		DiffuseTransmissionStruct str;
		if (SetDiffuseTransmissionParams(pMtl, str, animated)) {
			CreateDiffuseTransmissionTexture(material, str, animated);
		}
	}
}

