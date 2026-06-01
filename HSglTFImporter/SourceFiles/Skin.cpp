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

//======================================================================
// Assign Skin
//======================================================================
void glTFImporter_Core::SetSkin(cgltf_node *node)
{
	INode *pNode = m_NodeMap[node];
	if(!pNode) return;
	GetCOREInterface()->SelectNode(pNode);

	Modifier *pSkinMod = AddModifier(pNode, SKIN_CLASSID);
	ISkinImportData *pSkinImp = (ISkinImportData *)pSkinMod->GetInterface(I_SKINIMPORTDATA);
	if (!pSkinImp) return;

	cgltf_skin *skin = node->skin;
	size_t numBone = skin->joints_count;
	for (int i = 0; i < numBone; i++) {
		INode *pBone = m_NodeMap[skin->joints[i]];
		if(pBone) pSkinImp->AddBoneEx(pBone, (i == (numBone-1)));
	}

	std::vector<Point4> boneIDList;
	std::vector<Point4> weightList;
	cgltf_mesh *mesh = node->mesh;
	if (!mesh) return;
	for (int i = 0; i < mesh->primitives_count; i++) {
		cgltf_primitive *pr = &mesh->primitives[i];
		cgltf_draco_mesh_compression *mc = NULL;
		if (pr->has_draco_mesh_compression) {
			mc = &pr->draco_mesh_compression;
		}
		std::vector<float> wList;
		if (mc) {
			DracoDecodeProc(mc->buffer_view, wList, DracoDecodeType::WEIGHTS);
		}
		else {
			GetDataList(wList, findAttrAccesor(pr, "WEIGHTS_0"));
		}
		for (size_t k = 0; k + 3 < wList.size(); k += 4) {
			Point4 p(wList[k], wList[k + 1], wList[k + 2], wList[k + 3]);
			weightList.push_back(p);
		}
		//for (std::vector<float>::iterator v = wList.begin(); v != wList.end(); v += 4) {
		//	Point4 p(*v, *(v + 1), *(v + 2), *(v + 3));
		//	weightList.push_back(p);
		//}
		std::vector<float> bList;
		if (mc) {
			DracoDecodeProc(mc->buffer_view, bList, DracoDecodeType::JOINTS);
		}
		else {
			GetDataList(bList, findAttrAccesor(pr, "JOINTS_0"));
		}
		for (size_t k = 0; k + 3 < bList.size(); k += 4) {
			Point4 p(bList[k], bList[k + 1], bList[k + 2], bList[k + 3]);
			boneIDList.push_back(p);
		}
		//for (std::vector<float>::iterator v = bList.begin(); v != bList.end(); v += 4) {
		//	Point4 p(*v, *(v + 1), *(v + 2), *(v + 3));
		//	boneIDList.push_back(p);
		//}
		SetSkinImportStatus(1);
	}

	cgltf_accessor *acc = skin->inverse_bind_matrices;
	if (acc) {
#ifdef MAX_RELEASE_R24
		Matrix3 ParentTM;
#else
		Matrix3 ParentTM(1);
#endif 
		if (!pNode->GetParentNode()->IsRootNode()) {
//			ParentTM = (pNode->GetParentTM(m_time));
		}
		std::vector<float> inverseMtxList;
		GetDataList(inverseMtxList, acc);
		if (inverseMtxList.size() < (size_t)numBone * 16) return;

		std::vector<float>::iterator m = inverseMtxList.begin();
		for (int i = 0; i < numBone; i++) {
			INode *pBone = m_NodeMap[skin->joints[i]];
			if (!pBone) continue;
			Matrix3 mtx;
			mtx = Inverse(Matrix3(Point3(m[0], m[1], m[2]), Point3(m[4], m[5], m[6]), Point3(m[8], m[9], m[10]), Point3(m[12], m[13], m[14])*m_scale));
			mtx = mtx * pNode->GetNodeTM(m_time) * ParentTM;
			pSkinImp->SetBoneTm(pBone, mtx, mtx);
			m += 16;
		}
	}

	// Weight
	std::vector<Point4>::iterator pb = boneIDList.begin();
	std::vector<Point4>::iterator pw = weightList.begin();
	size_t numVert = weightList.size() < boneIDList.size() ? weightList.size() : boneIDList.size();
	for (int idx = 0; idx < numVert; idx++) {
		Tab<INode*> b;
		Tab<float> w;
		b.ZeroCount();
		w.ZeroCount();

		if (pw->x > 0.0f) {
			w.Append(1, &pw->x);
			size_t jointIdx = (size_t)pb->x;
			if (jointIdx < (size_t)numBone) {
				INode* pN = m_NodeMap[skin->joints[jointIdx]];
				if (pN) b.Append(1, &pN);
			}
		}
		if (pw->y > 0.0f) {
			w.Append(1, &pw->y);
			size_t jointIdx = (size_t)pb->y;
			if (jointIdx < (size_t)numBone) {
				INode* pN = m_NodeMap[skin->joints[jointIdx]];
				if (pN) b.Append(1, &pN);
			}
		}
		if (pw->z > 0.0f) {
			w.Append(1, &pw->z);
			size_t jointIdx = (size_t)pb->z;
			if (jointIdx < (size_t)numBone) {
				INode* pN = m_NodeMap[skin->joints[jointIdx]];
				if (pN) b.Append(1, &pN);
			}
		}
		if (pw->w > 0.0f) {
			w.Append(1, &pw->w);
			size_t jointIdx = (size_t)pb->w;
			if (jointIdx < (size_t)numBone) {
				INode* pN = m_NodeMap[skin->joints[jointIdx]];
				if (pN) b.Append(1, &pN);
			}
		}
		pSkinImp->AddWeights(pNode, idx, b, w);
		pb++;
		pw++;

		SetSkinImportStatus(1);
	}

}
