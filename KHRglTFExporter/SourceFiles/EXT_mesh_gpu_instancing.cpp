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
#include <iInstanceMgr.h>

extern Matrix3 GetOffsetTM(INode* pNode);

BOOL glTFExporter_Core::IsInstanced(INode* pNode)
{
	INodeTab instances;
	IInstanceMgr::GetInstanceMgr()->GetInstances(*pNode, instances);
	return instances.Count() > 1;
}

void glTFExporter_Core::ExportGPUInstanceSection(tinygltf::Scene& scene)
{
	size_t cnt = m_GPUInstanceNodeList.size();

	auto map_p = m_GPUInstanceMap.begin();
	auto node_p = m_GPUInstanceNodeList.begin();
	for (UINT i = 0; i < cnt; i++) {
		tinygltf::Node node = *node_p;
		auto map = map_p->second;
		std::vector<Point3> pos;
		std::vector<Quat> rot;
		std::vector<Point3> scl;
		for(auto pNode : map) {
			Matrix3 tm(pNode->GetObjTMAfterWSM(m_time));

			if (pNode->GetParentNode()->IsRootNode()) {
				tm = tm * YupTM;
			}
			else {
				tm = Inverse(GetOffsetTM(pNode)) * tm;
				tm = tm * Inverse(pNode->GetParentTM(m_time));
			}
			AffineParts parts;
			decomp_affine(tm, &parts);
			pos.push_back(parts.t);
			rot.push_back(parts.q);
			scl.push_back(parts.k);
		}
		int a = CreateInstanceTranslationSection(pos);
		int b = CreateInstanceRotationSection(rot);
		int c = CreateInstanceScaleSection(scl);

		tinygltf::Value::Object val;
		val.insert(std::make_pair("TRANSLATION", tinygltf::Value(a)));
		val.insert(std::make_pair("ROTATION", tinygltf::Value(b)));
		val.insert(std::make_pair("SCALE", tinygltf::Value(c)));

		tinygltf::Value::Object extension;
		extension.insert(std::make_pair("attributes", val));

		node.extensions.insert(std::make_pair("EXT_mesh_gpu_instancing", extension));

		m_model.nodes.push_back(node);
		scene.nodes.push_back((int)(m_model.nodes.size() - 1));

		map_p++;
		node_p++;
	}
	
	if (cnt > 0) m_Mesh_gpu_instancing_Used = TRUE;
}

int glTFExporter_Core::CreateInstanceTranslationSection(std::vector<Point3> &pos)
{
	tinygltf::Accessor accOut;
	accOut.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
	accOut.type = TINYGLTF_TYPE_VEC3;
	accOut.count = pos.size();

	tinygltf::BufferView bfViewOut;
	bfViewOut.buffer = 0;
	bfViewOut.byteOffset = m_BufferByteOffset;
	bfViewOut.byteLength = (int)(accOut.count * sizeof(float) * 3);

	void* ptr = SecureMemory((int)(bfViewOut.byteLength));
	float* pData = (float*)((char*)ptr + bfViewOut.byteOffset);
	//Control *pPosC = pC->GetPositionController();
	for (auto p : pos) {
		*pData++ = p.x;
		*pData++ = p.y;
		*pData++ = p.z;
	}

	m_model.bufferViews.push_back(bfViewOut);
	accOut.bufferView = (int)(m_model.bufferViews.size() - 1);
	m_model.accessors.push_back(accOut);

	return (int)(m_model.accessors.size() - 1);
}

int glTFExporter_Core::CreateInstanceRotationSection(std::vector<Quat>& rot)
{
	tinygltf::Accessor accOut;
	accOut.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
	accOut.type = TINYGLTF_TYPE_VEC4;
	accOut.count = rot.size();

	tinygltf::BufferView bfViewOut;
	bfViewOut.buffer = 0;
	bfViewOut.byteOffset = m_BufferByteOffset;
	bfViewOut.byteLength = accOut.count * sizeof(float) * 4;

	void* ptr = SecureMemory((int)(bfViewOut.byteLength));
	float* pData = (float*)((char*)ptr + bfViewOut.byteOffset);
	for (auto r : rot) {
		*pData++ = r.x;
		*pData++ = r.y;
		*pData++ = r.z;
		*pData++ = -r.w;
	}

	m_model.bufferViews.push_back(bfViewOut);
	accOut.bufferView = (int)(m_model.bufferViews.size() - 1);
	m_model.accessors.push_back(accOut);

	return (int)(m_model.accessors.size() - 1);
}

int glTFExporter_Core::CreateInstanceScaleSection(std::vector<Point3>& scl)
{
	tinygltf::Accessor accOut;
	accOut.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
	accOut.type = TINYGLTF_TYPE_VEC3;
	accOut.count = scl.size();

	tinygltf::BufferView bfViewOut;
	bfViewOut.buffer = 0;
	bfViewOut.byteOffset = m_BufferByteOffset;
	bfViewOut.byteLength = accOut.count * sizeof(float) * 3;

	void* ptr = SecureMemory((int)(bfViewOut.byteLength));
	float* pData = (float*)((char*)ptr + bfViewOut.byteOffset);
	for (auto s : scl) {
		*pData++ = s.x;
		*pData++ = s.y;
		*pData++ = s.z;
	}
	m_model.bufferViews.push_back(bfViewOut);
	accOut.bufferView = (int)(m_model.bufferViews.size() - 1);
	m_model.accessors.push_back(accOut);

	return (int)(m_model.accessors.size() - 1);
}

