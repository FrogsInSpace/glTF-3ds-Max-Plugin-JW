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
#include <splshape.h>

//----------------------------------------------------------
//----------------------------------------------------------
int SetVertTable(Point3& p, std::vector<Point3>& vertTable)
{
	auto it = std::find(vertTable.begin(), vertTable.end(), p);
	if (it != vertTable.end())
		return (int)std::distance(vertTable.begin(), it);
	vertTable.push_back(p);

	return vertTable.size() - 1;
}

#if 0
//----------------------------------------------------------
//----------------------------------------------------------
void glTFExporter_Core::CreateShapeData(INode* pNode, tinygltf::Node& node)
{
	BezierShape* pShape = NULL;
	BOOL deleteIt;
	{
		SplineShape* pShapeObj = GetShapeObjectFromNode(pNode, m_time, deleteIt);
		pShape = &pShapeObj->shape;
	}

	BitArray ClosureBit;
	pShape->GetClosures(ClosureBit);

	tinygltf::Mesh mesh;// = Create_glTFMesh(pNode->GetName());
	SetName(&mesh, pNode->GetName());
	mesh.name = WStringToString(pNode->GetName());

	int primitiveId = 0;

	int AttributeIndex = 0;
	tinygltf::Primitive primitive;
	primitive.mode = TINYGLTF_MODE_LINE;
	primitive.material = -1;

	std::vector<Point3> vertTable;
	//----------- Create Vertex table
/*
	{
		vertTable.clear();
		for (int i = 0; i < pShape->GetTotalVerts(); i++) {
			Point3 p = pShape->GetVert(i);
			vertTable.push_back(p);
		}
	}
*/

//	if (m_DracoCompress) {
	if (FALSE) {
		//CreateDracoMeshProp(primitive, pMesh, pNrmSpec, vertTable, faceIDTable, CVertMode, pMtl, pSkinMC, pMorphMod);
		//GetMeshInfoXX(pMesh);
	}
	else {

		//----------- Index
		{
			tinygltf::Accessor acc;// = Create_glTFAccessor();
			acc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
			acc.type = TINYGLTF_TYPE_SCALAR;
			acc.count = pShape->GetNumSegs() * 2;
			tinygltf::BufferView bfView;// = Create_glTFBufferView();
			bfView.buffer = 0;
			bfView.byteOffset = m_BufferByteOffset;
			bfView.byteLength = acc.count * sizeof(UINT);
			bfView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

			void* ptr = SecureMemory(bfView.byteLength);
			UINT* pShortIdx = (UINT*)((char*)ptr + bfView.byteOffset);

			int faceIdx = 0;
			for (int poly = 0; poly < pShape->SplineCount(); poly++) {
				Spline3D* pSpline = pShape->GetSpline(poly);
				int vert = 0;
				int NumSeg = pSpline->Segments();
				if (pSpline->Closed()) NumSeg -= 1;
				for (int seg = 0; seg < NumSeg; seg++) {
					SplineKnot knot1 = pSpline->GetKnot(vert++);
					SplineKnot knot2 = pSpline->GetKnot(vert);
					Point3 p1 = knot1.Knot();
					Point3 p2 = knot2.Knot();
					int idx1 = SetVertTable(p1, vertTable);
					int idx2 = SetVertTable(p2, vertTable);
					*pShortIdx++ = idx1;
					*pShortIdx++ = idx2;
				}
				if (pSpline->Closed()) {
					SplineKnot knot1 = pSpline->GetKnot(pSpline->Segments() - 1);
					SplineKnot knot2 = pSpline->GetKnot(0);
					Point3 p1 = knot1.Knot();
					Point3 p2 = knot2.Knot();
					int idx1 = SetVertTable(p1, vertTable);
					int idx2 = SetVertTable(p2, vertTable);
					*pShortIdx++ = idx1;
					*pShortIdx++ = idx2;
				}
			}

			m_model.bufferViews.push_back(bfView);
			acc.bufferView = m_model.bufferViews.size() - 1;
			m_model.accessors.push_back(acc);
			primitive.indices = m_model.accessors.size() - 1;
		}

		//----------- Position
		{
			tinygltf::Accessor acc;// = Create_glTFAccessor();
			acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
			acc.type = TINYGLTF_TYPE_VEC3;
			acc.count = vertTable.size();
			tinygltf::BufferView bfView;// = Create_glTFBufferView();
			bfView.buffer = 0;
			bfView.byteOffset = m_BufferByteOffset;
			bfView.byteLength = acc.count * sizeof(float) * 3;
			bfView.target = TINYGLTF_TARGET_ARRAY_BUFFER;

			Point3 minPos = vertTable[0];
			Point3 maxPos = minPos;

			void* ptr = SecureMemory(bfView.byteLength);
			float* pPos = (float*)((char*)ptr + bfView.byteOffset);
			for (auto p : vertTable) {
				if (p.x > maxPos.x)	maxPos.x = p.x;
				if (p.y > maxPos.y)	maxPos.y = p.y;
				if (p.z > maxPos.z)	maxPos.z = p.z;
				if (p.x < minPos.x)	minPos.x = p.x;
				if (p.y < minPos.y)	minPos.y = p.y;
				if (p.z < minPos.z)	minPos.z = p.z;

				*pPos++ = p.x;
				*pPos++ = p.y;
				*pPos++ = p.z;
			}

			acc.maxValues.push_back(maxPos.x);
			acc.maxValues.push_back(maxPos.y);
			acc.maxValues.push_back(maxPos.z);
			acc.minValues.push_back(minPos.x);
			acc.minValues.push_back(minPos.y);
			acc.minValues.push_back(minPos.z);

			m_model.bufferViews.push_back(bfView);
			acc.bufferView = m_model.bufferViews.size() - 1;
			m_model.accessors.push_back(acc);
			primitive.attributes.insert(std::make_pair("POSITION", m_model.accessors.size() - 1));
		}
		mesh.primitives.push_back(primitive);
	}


	m_model.meshes.push_back(mesh);
	node.mesh = m_model.meshes.size() - 1;

	m_MeshMap[pNode->GetObjectRef()] = node.mesh;
}
#else
//----------------------------------------------------------
//----------------------------------------------------------
void glTFExporter_Core::CreateShapeData(INode* pNode, tinygltf::Node& node)
{
	node.mesh = -1;

	BOOL deleteIt;
	LinearShape* pShapeObj = GetShapeObjectFromNode(pNode, m_time, deleteIt);
	if (!pShapeObj) return;

	PolyShape* polyShape = &pShapeObj->shape;

	tinygltf::Mesh mesh;// = Create_glTFMesh(pNode->GetName());
	SetName(&mesh, pNode->GetName());
	mesh.name = WStringToString(pNode->GetName());

	int primitiveId = 0;

	//int AttributeIndex = 0;


	//	if (m_DracoCompress) {
	if (FALSE) {
		//CreateDracoMeshProp(primitive, pMesh, pNrmSpec, vertTable, faceIDTable, CVertMode, pMtl, pSkinMC, pMorphMod);
		//GetMeshInfoXX(pMesh);
	}
	else {
		int vertOffset = 0;
		for (int curve = 0; curve < pShapeObj->NumberOfCurves(m_time); curve++) {
			tinygltf::Primitive primitive;

			if (pShapeObj->CurveClosed(m_time, curve))
				primitive.mode = TINYGLTF_MODE_LINE_LOOP;
			else
				primitive.mode = TINYGLTF_MODE_LINE;

			primitive.mode = TINYGLTF_MODE_LINE_STRIP;

			primitive.material = -1;

			std::vector<Point3> vertTable;
			vertTable.clear();

			size_t NumVerts = pShapeObj->NumberOfVertices(m_time, curve);
			size_t NumPiece = pShapeObj->NumberOfPieces(m_time, curve);
			if (pShapeObj->CurveClosed(m_time, curve)) NumPiece -= 1;

			//----------- Index
			{
				tinygltf::Accessor acc;// = Create_glTFAccessor();
				acc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
				acc.type = TINYGLTF_TYPE_SCALAR;
				acc.count = NumVerts;// *2;
				if (pShapeObj->CurveClosed(m_time, curve))
					acc.count += 1;

				tinygltf::BufferView bfView;// = Create_glTFBufferView();
				bfView.buffer = 0;
				bfView.byteOffset = m_BufferByteOffset;
				bfView.byteLength = acc.count * sizeof(UINT);
				bfView.target = TINYGLTF_TARGET_ELEMENT_ARRAY_BUFFER;

				void* ptr = SecureMemory(bfView.byteLength);
				UINT* pShortIdx = (UINT*)((char*)ptr + bfView.byteOffset);

#if 0
				int vert = vertOffset;
				int startIdx, endIdx;
				for (int seg = 0; seg < NumPiece; seg++) {
					Point3 p1 = pShapeObj->GetPoint(vert++);
					Point3 p2 = pShapeObj->GetPoint(vert);
					int idx1 = SetVertTable(p1, vertTable);
					int idx2 = SetVertTable(p2, vertTable);
					*pShortIdx++ = idx1;
					*pShortIdx++ = idx2;

					//if (seg == 0)			startIdx = idx1;
					//if (seg == NumPiece - 1)	endIdx = idx2;
				}
				//if (pShapeObj->CurveClosed(m_time, curve)) {
			//		*pShortIdx++ = endIdx;
			//		*pShortIdx++ = startIdx;
			//	}
#else
				//int vert = vertOffset;
				int startIdx=0;
				for (int vert = 0; vert < NumVerts; vert++) {
					Point3 p1 = pShapeObj->GetPoint(vert + vertOffset);
					int idx1 = SetVertTable(p1, vertTable);
					*pShortIdx++ = idx1;

					if (vert == 0)	startIdx = idx1;
				}
				if (pShapeObj->CurveClosed(m_time, curve)) {
					*pShortIdx++ = startIdx;
				}
#endif

				vertOffset += NumVerts;

				CreateShapePositionData(primitive, vertTable);

				m_model.bufferViews.push_back(bfView);
				acc.bufferView = m_model.bufferViews.size() - 1;
				m_model.accessors.push_back(acc);
				primitive.indices = m_model.accessors.size() - 1;
			}

			mesh.primitives.push_back(primitive);
		}
	}

	m_model.meshes.push_back(mesh);
	node.mesh = m_model.meshes.size() - 1;

	m_MeshMap[pNode->GetObjectRef()] = node.mesh;
}
#endif

void glTFExporter_Core::CreateShapePositionData(tinygltf::Primitive &primitive, std::vector<Point3> &vertTable)
{
	//----------- Position
	{
		tinygltf::Accessor acc;// = Create_glTFAccessor();
		acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
		acc.type = TINYGLTF_TYPE_VEC3;
		acc.count = vertTable.size();
		tinygltf::BufferView bfView;// = Create_glTFBufferView();
		bfView.buffer = 0;
		bfView.byteOffset = m_BufferByteOffset;
		bfView.byteLength = acc.count * sizeof(float) * 3;
		bfView.target = TINYGLTF_TARGET_ARRAY_BUFFER;

		Point3 minPos = vertTable[0];
		Point3 maxPos = minPos;

		void* ptr = SecureMemory(bfView.byteLength);
		float* pPos = (float*)((char*)ptr + bfView.byteOffset);
		for (auto p : vertTable) {
			if (p.x > maxPos.x)	maxPos.x = p.x;
			if (p.y > maxPos.y)	maxPos.y = p.y;
			if (p.z > maxPos.z)	maxPos.z = p.z;
			if (p.x < minPos.x)	minPos.x = p.x;
			if (p.y < minPos.y)	minPos.y = p.y;
			if (p.z < minPos.z)	minPos.z = p.z;

			*pPos++ = p.x;
			*pPos++ = p.y;
			*pPos++ = p.z;
		}

		acc.maxValues.push_back(maxPos.x);
		acc.maxValues.push_back(maxPos.y);
		acc.maxValues.push_back(maxPos.z);
		acc.minValues.push_back(minPos.x);
		acc.minValues.push_back(minPos.y);
		acc.minValues.push_back(minPos.z);

		m_model.bufferViews.push_back(bfView);
		acc.bufferView = m_model.bufferViews.size() - 1;
		m_model.accessors.push_back(acc);
		primitive.attributes.insert(std::make_pair("POSITION", m_model.accessors.size() - 1));
	}
}
