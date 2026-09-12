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


#include "KHRglTFExporter.h"



std::map<ReferenceTarget*, QuantizationInfo> quantizationInfoMap;

//======================================================================
//======================================================================
void SetQuantizationUVInfo(Mesh *pMesh, int mapCh, float &scale, Point2 &offset)
{
	scale = 1.0f;
	offset = Point2(0.0f, 0.0f);

	if (!pMesh) return;
	if (!pMesh->mapSupport(mapCh)) return;

	MeshMap* pMap = &pMesh->Map(mapCh);

	UVVert* pSrcUV = pMap->tv;
	Point3 minUV = pMap->tv[0];
	minUV.y = 1.0f - minUV.y;
	Point3 maxUV = minUV;

	for(int i=0;i<pMap->vnum; i++) {
		UVVert p = pSrcUV[i];
		p.y = -p.y + 1.0f;
		if (p.x > maxUV.x)	maxUV.x = p.x;
		if (p.y > maxUV.y)	maxUV.y = p.y;
		if (p.x < minUV.x)	minUV.x = p.x;
		if (p.y < minUV.y)	minUV.y = p.y;
	}

	if (maxUV.x < 1.0f)	maxUV.x = 1.0f;
	if (maxUV.y < 1.0f)	maxUV.y = 1.0f;
	if (minUV.x > 0.0f)	minUV.x = 0.0f;
	if (minUV.y > 0.0f)	minUV.y = 0.0f;

	scale = std::max({ (maxUV - minUV).x, (maxUV - minUV).y }) / 65535.0f;
	offset = Point2(- minUV.x, - minUV.y);
}

//======================================================================
//======================================================================
void SetQuantizationTexRec(MtlBase* pRef, QuantizationInfo& info)
{
	if (!pRef) return;

	if (pRef->ClassID() == bmptexClassID) {
		quantizationInfoMap[pRef] = info;
		return;
	}

	if (pRef->SuperClassID() == TEXMAP_CLASS_ID) {
		Texmap* pTex = (Texmap*)pRef;
		for (int i = 0; i < pTex->NumSubTexmaps(); i++) {
			SetQuantizationTexRec(pTex->GetSubTexmap(i), info);
		}
	}
	else if (pRef->SuperClassID() == MATERIAL_CLASS_ID) {
		Mtl *pMtl = (Mtl*)pRef;
		for (int i = 0; i < pMtl->NumSubTexmaps(); i++) {
			SetQuantizationTexRec(pMtl->GetSubTexmap(i), info);
		}

		for (int i = 0; i < pMtl->NumSubMtls(); i++) {
			SetQuantizationTexRec(pMtl->GetSubMtl(i), info);
		}
	}
}

//======================================================================
//======================================================================
void CreateQuatizationMapRec(INode *pNode,float sceneScale)
{
	if (IsGeometryObject(pNode)) {
		QuantizationInfo info;

		Modifier* pSkinMod = NULL;
		if (FindModifier(pNode, SKIN_CLASSID, &pSkinMod) >= 0) {
			pSkinMod->DisableMod();
			pSkinMod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
		}

		BOOL deleteIt = FALSE;
		TriObject* pTri = GetTriObjectFromNode(pNode, 0, deleteIt);

		Box3 box;
		pTri->GetDeformBBox(0, box);

		float v = std::max({ fabs(box.pmax.x), fabs(box.pmax.y), fabs(box.pmax.z),fabs(box.pmin.x), fabs(box.pmin.y), fabs(box.pmin.z) });
		info.meshScale = v;// 32767.0f / v;// / sceneScale;// / 32767.0f * sceneScale;
		//info, meshSize = Point3(
		//	std::max({ fabs(box.pmax.x),fabs(box.pmin.x) }),
		//	std::max({ fabs(box.pmax.y),fabs(box.pmin.y) }),
		//	std::max({ fabs(box.pmax.z),fabs(box.pmin.z) }));
	
		if (pNode->GetMtl()) {
			SetQuantizationUVInfo(&pTri->mesh, 1, info.uvmap1Scale, info.uvmap1Offset);
			SetQuantizationUVInfo(&pTri->mesh, 2, info.uvmap2Scale, info.uvmap2Offset);
		}
		quantizationInfoMap[pNode] = info;

		if (deleteIt) delete pTri;

		if (pSkinMod) {
			pSkinMod->EnableMod();
		}

		SetQuantizationTexRec(pNode->GetMtl(), info);
	}


	for (int i = 0; i < pNode->NumberOfChildren(); i++) {
		CreateQuatizationMapRec(pNode->GetChildNode(i), sceneScale);
	}
}

//======================================================================
//======================================================================
void glTFExporter_Core::CreateQuatizationMap(void)
{
	quantizationInfoMap.clear();

	INode* pRoot = GetCOREInterface()->GetRootNode();
	for(int i = 0; i < pRoot->NumberOfChildren(); i++) {
		CreateQuatizationMapRec(pRoot->GetChildNode(i),m_scale);
	}			
}


//======================================================================
//======================================================================
BOOL glTFExporter_Core::GetQuatizationInfo(ReferenceTarget *pRef, QuantizationInfo &info)
{
	auto it = quantizationInfoMap.find(pRef);
	if(it == quantizationInfoMap.end()) {
		info = QuantizationInfo();
		return FALSE;
	}

	info = it->second;
	return TRUE;
}
