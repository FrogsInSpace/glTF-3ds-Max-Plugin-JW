
#include "HSglTFExporter.h"


static std::vector<Texmap*> TexmapList;
static std::vector<tstring> ImageFileList;

void glTFExporter_Core::BufferPreCalculate(void)
{
	m_AccessorNum = 0;
	m_MaterialNum = 0;
	m_TextureNum = 0;
	m_ImageNum = 0;
	m_NodeNum = 0;
	m_MeshNum = 0;
	//m_PrimitiveNum = 0;
	m_BufferViewNum = 0;
	m_SkinNum = 0;
	m_CameraNum = 0;
	m_LightNum = 0;
	m_NameNum = 0;
	m_AnimationNum = 0;
	m_AnimChannelNum = 0;
	m_AnimSamplerNum = 0;

	for(int i=0; i<GetCOREInterface()->GetRootNode()->NumChildren();i++) {
		CalcRec(GetCOREInterface()->GetRootNode()->GetChildNode(i));
	}

	TexmapList.clear();
	ImageFileList.clear();
	MtlBaseLib *mtlLib = GetCOREInterface()->GetSceneMtls();
	for (int i = 0; i < mtlLib->Count(); i++) {
		Mtl *pMtl = (Mtl*)*mtlLib->Addr(i);
		CalcMtlRec(pMtl);
	}

	if (m_ExportFileType == 3) {
		m_BufferViewNum += m_ImageNum;
	}
}

void glTFExporter_Core::CalcRec(INode *pNode)
{
	TSTR name = pNode->GetName();

	m_NodeNum++;
	m_NameNum++;

	BOOL deleteIt = FALSE;
	TriObject *pTri = GetTriObjectFromNode(pNode, m_time, deleteIt);
	if (pTri) {
		int subMtlsCount = 1;
		Mtl *pMtl = pNode->GetMtl();
		if (pMtl) {
			subMtlsCount = 1;
			std::map<int, std::vector<int> > mtlIDMap;
			Mesh *pMesh = &pTri->mesh;
			CreateMtlIDTable(pMesh, pMtl, mtlIDMap);
			//if (pMtl->ClassID() == multiClassID)
			subMtlsCount = mtlIDMap.size();

			if (pTri->mesh.mapSupport(1)) {
				m_AccessorNum += subMtlsCount;
				m_BufferViewNum += subMtlsCount;
			}
		}

		// Index, Position, Normal
		m_AccessorNum += 3 * subMtlsCount;
		m_BufferViewNum += 3 * subMtlsCount;

		m_MeshNum++;
		m_NameNum++;

		if (deleteIt) delete pTri;

		Modifier *pMod;
		if (FindModifier(pNode, SKIN_CLASSID, &pMod) >= 0) {
			m_SkinNum++;

			// Weight
			m_AccessorNum += subMtlsCount;
			m_BufferViewNum += subMtlsCount;
			// Joint
			m_AccessorNum += subMtlsCount;
			m_BufferViewNum += subMtlsCount;

			//inverseBindMatrices
			m_AccessorNum++;
			m_BufferViewNum++;
		}
	}

	Object *pObj = pNode->GetObjectRef();
	if (pObj->SuperClassID() == CAMERA_CLASS_ID) {
		m_CameraNum++;
	}
	else if (pObj->SuperClassID() == LIGHT_CLASS_ID) {
		m_LightNum++;
	}

	Tab<TimeValue> PosFrameList;
	Tab<TimeValue> RotFrameList;
	Tab<TimeValue> SclFrameList;
	PosFrameList.ZeroCount();
	RotFrameList.ZeroCount();
	SclFrameList.ZeroCount();

	Control *pC = pNode->GetTMController();
	pC->GetKeyTimes(PosFrameList, FOREVER, KEYAT_POSITION);
	pC->GetKeyTimes(SclFrameList, FOREVER, KEYAT_SCALE);
	Control *pRotC = pC->GetRotationController();
	if (pRotC->NumKeys() > 0) {
		for (int i = 0; i < pRotC->NumKeys(); i++) {
			TimeValue t = pRotC->GetKeyTime(i);
			RotFrameList.Append(1, &t);
		}
	}

	if (PosFrameList.Count() >0) {
		m_AccessorNum += 2;
		m_BufferViewNum += 2;
		m_AnimChannelNum++;
		m_AnimSamplerNum++;
		m_AnimationNum = 1;
	}
	if (RotFrameList.Count() > 0) {
		m_AccessorNum += 2;
		m_BufferViewNum += 2;
		m_AnimChannelNum++;
		m_AnimSamplerNum++;
		m_AnimationNum = 1;
	}
	if (SclFrameList.Count() > 0) {
		m_AccessorNum += 2;
		m_BufferViewNum += 2;
		m_AnimChannelNum++;
		m_AnimSamplerNum++;
		m_AnimationNum = 1;
	}

/*
	Control *pC = pNode->GetTMController();
	if (pC->IsAnimated()) {

		BOOL PosAnimated = FALSE;
		BOOL RotAnimated = FALSE;
		BOOL SclAnimated = FALSE;

		Control *pPosC = pC->GetPositionController();
		Control *pRotC = pC->GetRotationController();
		Control *pSclC = pC->GetScaleController();
		IKeyControl *pIk = GetKeyControlInterface(pPosC);
		if (pIk) {
			if(pIk->GetNumKeys()>0) PosAnimated = TRUE;
		}
		else {
			Control *pXC = pPosC->GetXController();
			if(GetKeyControlInterface(pXC)->GetNumKeys() > 0) PosAnimated = TRUE;
			Control *pYC = pPosC->GetYController();
			if(GetKeyControlInterface(pYC)->GetNumKeys() > 0) PosAnimated = TRUE;
			Control *pZC = pPosC->GetZController();
			if(GetKeyControlInterface(pZC)->GetNumKeys() > 0) PosAnimated = TRUE;
		}
		pIk = GetKeyControlInterface(pRotC);
		if (pIk) {
			if(pIk > 0) RotAnimated = TRUE;
		}
		else {
			Control *pXC = pRotC->GetXController();
			if (GetKeyControlInterface(pXC)->GetNumKeys() > 0) RotAnimated = TRUE;
			Control *pYC = pRotC->GetYController();
			if (GetKeyControlInterface(pYC)->GetNumKeys() > 0) RotAnimated = TRUE;
			Control *pZC = pRotC->GetZController();
			if (GetKeyControlInterface(pZC)->GetNumKeys() > 0) RotAnimated = TRUE;
		}
		pIk = GetKeyControlInterface(pSclC);
		if (pIk) {
			if(pIk->GetNumKeys() > 0) SclAnimated = TRUE;
		}
		else {
			Control *pXC = pSclC->GetXController();
			if (GetKeyControlInterface(pXC)->GetNumKeys() > 0) SclAnimated = TRUE;
			Control *pYC = pSclC->GetYController();
			if (GetKeyControlInterface(pYC)->GetNumKeys() > 0) SclAnimated = TRUE;
			Control *pZC = pSclC->GetZController();
			if (GetKeyControlInterface(pZC)->GetNumKeys() > 0) SclAnimated = TRUE;
		}

		if (PosAnimated) {
			m_AccessorNum+=2;
			m_BufferViewNum+=2;
			m_AnimChannelNum++;
			m_AnimSamplerNum++;
		}
		if (RotAnimated) {
			m_AccessorNum += 2;
			m_BufferViewNum += 2;
			m_AnimChannelNum++;
			m_AnimSamplerNum++;
		}
		if (SclAnimated) {
			m_AccessorNum += 2;
			m_BufferViewNum += 2;
			m_AnimChannelNum++;
			m_AnimSamplerNum++;
		}
		m_AnimSamplerNum=1;

	}
*/

	for(int i=0; i<pNode->NumChildren();i++) {
		CalcRec(pNode->GetChildNode(i));
	}
}

void glTFExporter_Core::CalcMtlRec(Mtl *pMtl)
{
	if (!pMtl) return;

	if (pMtl->ClassID() != multiClassID) {
		m_NameNum++;
		m_MaterialNum++;
	}

	if (pMtl->ClassID() == PBRMetalMtlID) {
		IParamBlock2 *pBlock1 = pMtl->GetParamBlockByID(1);
		Texmap *pTex;
		pBlock1->GetValue(pbr_base_color_map, m_time, pTex, FOREVER);
		if (pTex) {
			if (pTex->ClassID() == bmptexClassID) {
				if (std::find(TexmapList.begin(), TexmapList.end(), pTex) == TexmapList.end()) {
					m_TextureNum++;
					m_NameNum++;
					TexmapList.push_back(pTex);
					tstring str = ((BitmapTex*)pTex)->GetMapName();
					if (std::find(ImageFileList.begin(), ImageFileList.end(),str) == ImageFileList.end()) {
						m_ImageNum++;
						ImageFileList.push_back(str);
					}
				}
			}
		}
		pBlock1->GetValue(pbr_norm_map, m_time, pTex, FOREVER);
		if (pTex) {
			if (pTex->ClassID() == bmptexClassID) {
				if (std::find(TexmapList.begin(), TexmapList.end(), pTex) == TexmapList.end()) {
					m_TextureNum++;
					m_NameNum++;
					TexmapList.push_back(pTex);
					tstring str = ((BitmapTex*)pTex)->GetMapName();
					if (std::find(ImageFileList.begin(), ImageFileList.end(), str) == ImageFileList.end()) {
						m_ImageNum++;
						ImageFileList.push_back(str);
					}
				}
			}
		}
		pBlock1->GetValue(pbr_emit_color_map, m_time, pTex, FOREVER);
		if (pTex) {
			if (pTex->ClassID() == bmptexClassID) {
				if (std::find(TexmapList.begin(), TexmapList.end(), pTex) == TexmapList.end()) {
					m_TextureNum++;
					m_NameNum++;
					TexmapList.push_back(pTex);
					tstring str = ((BitmapTex*)pTex)->GetMapName();
					if (std::find(ImageFileList.begin(), ImageFileList.end(), str) == ImageFileList.end()) {
						m_ImageNum++;
						ImageFileList.push_back(str);
					}
				}
			}
		}
		pBlock1->GetValue(pbr_ao_map, m_time, pTex, FOREVER);
		if (pTex) {
			if (pTex->ClassID() == bmptexClassID) {
				if (std::find(TexmapList.begin(), TexmapList.end(), pTex) == TexmapList.end()) {
					m_TextureNum++;
					m_NameNum++;
					TexmapList.push_back(pTex);
					tstring str = ((BitmapTex*)pTex)->GetMapName();
					if (std::find(ImageFileList.begin(), ImageFileList.end(), str) == ImageFileList.end()) {
						m_ImageNum++;
						ImageFileList.push_back(str);
					}
				}
			}
		}
		pBlock1->GetValue(pbr_opacity_map, m_time, pTex, FOREVER);
		if (pTex) {
		}
		Texmap *pTex1, *pTex2;
		pBlock1->GetValue(pbr_metalness_map, m_time, pTex1, FOREVER);
		pBlock1->GetValue(pbr_roughness_map, m_time, pTex2, FOREVER);
		if (pTex1 || pTex2) {
			if (pTex1) {
				if (pTex1->ClassID() == bmptexClassID) {
					if (std::find(TexmapList.begin(), TexmapList.end(), pTex1) == TexmapList.end()) {
						m_TextureNum++;
						m_NameNum++;
						TexmapList.push_back(pTex1);
						tstring str = ((BitmapTex*)pTex1)->GetMapName();
						if (std::find(ImageFileList.begin(), ImageFileList.end(), str) == ImageFileList.end()) {
							m_ImageNum++;
							ImageFileList.push_back(str);
						}
					}
				}
			}
			else {
				if (pTex2->ClassID() == bmptexClassID) {
					if (std::find(TexmapList.begin(), TexmapList.end(), pTex2) == TexmapList.end()) {
						m_TextureNum++;
						m_NameNum++;
						TexmapList.push_back(pTex2);
						tstring str = ((BitmapTex*)pTex2)->GetMapName();
						if (std::find(ImageFileList.begin(), ImageFileList.end(), str) == ImageFileList.end()) {
							m_ImageNum++;
							ImageFileList.push_back(str);
						}
					}
				}
			}
		}
	}
	else if (pMtl->ClassID() == PHYSICALMATERIAL_CLASS_ID) {
	}
	else  if (pMtl->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {
		Texmap *pTex = pMtl->GetSubTexmap(ID_DI);
		if (pTex)
			if (pTex->ClassID() == bmptexClassID) {
				if (std::find(TexmapList.begin(), TexmapList.end(), pTex) == TexmapList.end()) {
					m_TextureNum++;
					m_ImageNum++;
					m_NameNum++;
					TexmapList.push_back(pTex);
				}
			}
		pTex = pMtl->GetSubTexmap(ID_BU);
	}
	if (pMtl->ClassID() == multiClassID) {
		for (int i = 0; i < ((Mtl*)pMtl)->NumSubMtls(); i++) {
			CalcMtlRec(((Mtl*)pMtl)->GetSubMtl(i));
		}
	}
}