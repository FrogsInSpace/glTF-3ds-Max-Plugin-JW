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

//======================================================================
// テーブル内のトップノードを見つける
//======================================================================
INode *findRootNode(std::vector<INode*> &tbl)
{
	if (tbl.size() == 0)return NULL;

	for (auto n : tbl) if (std::find(tbl.begin(), tbl.end(), n->GetParentNode()) == tbl.end()) return n;
	return *tbl.begin();
}
//======================================================================
// Skin ボーンの最上位ボーンインデクスを返す
//======================================================================
int glTFExporter_Core::GetRootNodeBoneID(ISkin* pISkin)
{
	int BoneNum = pISkin->GetNumBones();
	INodeTab nodeTab;
	for (int i = 0; i < BoneNum; i++) {
		nodeTab.AppendNode(pISkin->GetBone(i));
	}
	for (int i = 0; i < BoneNum; i++) {
		INode* pNode = nodeTab[i]->GetParentNode();
		if (!nodeTab.Contains(pNode)) return i;
	}
	return 0;
}


//======================================================================
// Skin割り当て
//======================================================================
void glTFExporter_Core::CreateSkin(INode *pNode, Modifier *pSkinMod)
{
	//ISkinImportData *pSkinImp = (ISkinImportData *)pSkinMod->GetInterface(I_SKINIMPORTDATA);
	ISkin *pISkin = (ISkin *)pSkinMod->GetInterface(I_SKIN);
	ISkin2 *pISkin2 = (ISkin2*)pSkinMod->GetInterface(I_SKIN2);
	int BoneNum = pISkin->GetNumBones();

	tinygltf::Skin skin;
	std::vector<INode*> tempTbl;
	for (int i = 0; i < BoneNum; i++) {
		INode *pBone = pISkin->GetBone(i);
		skin.joints.push_back(findNodeIndex(pBone));
		tempTbl.push_back(pBone);
	}
	INode *pRootBone = findRootNode(tempTbl);
	if (!pRootBone) return;
	skin.skeleton = findNodeIndex(pRootBone);

	tinygltf::Accessor acc;// = Create_glTFAccessor();
	acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
	acc.type = TINYGLTF_TYPE_MAT4;
	acc.count = BoneNum;
	tinygltf::BufferView bfView;// = Create_glTFBufferView();
	bfView.buffer = 0;
	bfView.byteOffset = m_BufferByteOffset;
	bfView.byteLength = acc.count * sizeof(float) * 4*4;

	void *ptr = SecureMemory(bfView.byteLength);
	float *pmtx = (float*)((char*)ptr + bfView.byteOffset);
	for (int i = 0; i < BoneNum;i++) {
		Matrix3 tm = pISkin->GetBoneTm(i);
		tm = Inverse(tm);
		tm = tm * Inverse(pNode->GetNodeTM(m_time));// *Inverse(pNode->GetParentTM(m_time)));
		tm = Inverse(tm);

		std::vector<double> m;
		Matrix3ToFloat(tm, m, m_scale);
		for(int j=0;j<16; j++)	*pmtx++ = (float)m[j];
	}
	m_model.bufferViews.push_back(bfView);
	acc.bufferView = m_model.bufferViews.size() - 1;
	m_model.accessors.push_back(acc);
	skin.inverseBindMatrices = m_model.accessors.size() - 1;

	m_model.skins.push_back(skin);

	UINT idx = m_NodeMap[pNode];
	m_model.nodes[idx].skin = m_model.skins.size() - 1;
}


#if 0
//----------------------------------------------------------
//----------------------------------------------------------
void glTFExporter_Core::CreateSkin(void)
{
	for (auto tbl : m_skinTable) {
		std::map<INode*, std::map<int, float> > wTable;
		std::map<INode*, aiBone*> boneTable;

		Modifier *pMod;
		INode *pNode = tbl.first;
		aiNode *node = tbl.second;

		BOOL deleteIt = FALSE;
		TriObject *pTri = GetTriObjectFromNode(pNode, m_time, deleteIt);
		Mesh *pMesh = &pTri->mesh;

		FindModifier(pNode, SKIN_CLASSID, &pMod);
		ISkin *pISkin = (ISkin *)pMod->GetInterface(I_SKIN);
		int n = pISkin->GetNumBones();
		aiMesh *mesh = m_scene.mMeshes[node->mMeshes[0]];
		mesh->mNumBones = n;
		mesh->mBones = new aiBone*[n];
		for (int i = 0; i < n; i++) {
			INode *pBoneNode = pISkin->GetBone(i);
			aiBone *bone = new aiBone();
			bone->mName = WStringToString(pBoneNode->GetName());
			Matrix3 tm = (pISkin->GetBoneTm(i));
			bone->mOffsetMatrix = Matrix3ToMtx4x4(tm);
			mesh->mBones[i] = bone;
			boneTable.insert(std::make_pair(pBoneNode, bone));
		}
		/*
				BonesDefMod *pboneDef = (BonesDefMod*)pMod;
				BoneModData *bmd = pboneDef->GetBMD(pNode);
				int vn = pMesh->numVerts;
				for (int i = 0; i < vn; i++) {
					if (bmd->VertexData[i]->IsModified()) {
						int bn = bmd->VertexData[i]->WeightCount();
						for (int j = 0; j < bn;j++) {
							int boneIdx = bmd->VertexData[i]->GetBoneIndex(j);
							float w = bmd->VertexData[i]->GetWeight(j);
							INode *pBone = pISkin->GetBone(boneIdx);
							auto tbl = wTable[pBone];
							tbl.insert(std::make_pair(i, w));
							wTable[pBone] = tbl;
						}
					}
				}

				ISkinContextData *pSkinMC = pISkin->GetContextInterface(pNode);
				int numv = pSkinMC->GetNumPoints();
				for (int i = 0; i < numv; i++) {
					//findVertTbl(i, pMesh, idxTable, 1);
					int numb = pSkinMC->GetNumAssignedBones(i);
					for (int j = 0; j < numb; j++) {
						int boneIdx = pSkinMC->GetAssignedBone(i, j);
						float w = pSkinMC->GetBoneWeight(i, boneIdx);
						INode *pBone = pISkin->GetBone(boneIdx);
						auto tbl = wTable[pBone];
						tbl.insert(std::make_pair(i, w));
						wTable[pBone] = tbl;
					}
				}
		*/
		for (auto wt : wTable) {
			INode *pBoneNode = wt.first;
			auto p = wt.second;
			aiBone *bone = boneTable[pBoneNode];
			bone->mNumWeights = p.size();
			bone->mWeights = new aiVertexWeight[p.size()];
			int i = 0;
			for (auto wp : p) {
				int idx = wp.first;
				float w = wp.second;
				bone->mWeights[i++] = aiVertexWeight(idx, w);
			}
		}
		if (deleteIt) delete pTri;
	}

}
/*
//----------------------------------------------------------
//----------------------------------------------------------
void Matrix3ToMtx4x4(Matrix3 &m, float *retTM)
{
	Point3 r1 = m.GetRow(0);
	Point3 r2 = m.GetRow(1);
	Point3 r3 = m.GetRow(2);
	Point3 r4 = m.GetRow(3);

	retTM[0] = r1.x;
	retTM[1] = r2.x;
	retTM[2] = r3.x;
	retTM[3] = r4.x;
	retTM[4] = r1.y;
	retTM[5] = r2.y;
	retTM[6] = r3.y;
	retTM[7] = r4.y;
	retTM[8] = r1.z;
	retTM[9] = r2.z;
	retTM[10] = r3.z;
	retTM[11] = r4.z;
	retTM[12] = 0.0f;
	retTM[13] = 0.0f;
	retTM[14] = 0.0f;
	retTM[15] = 1.0f;
}
*/
#endif