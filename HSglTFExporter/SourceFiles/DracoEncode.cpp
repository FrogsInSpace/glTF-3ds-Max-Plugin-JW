//**************************************************************************/
//
//
//**************************************************************************/

#define TINYGLTF_ENABLE_DRACO

#include "HSglTFExporter.h"
#include <include\MorpherApi.h>

#ifdef ENABLE_BUILD_WITH_DRACO

#undef max
#undef min

#include <draco/core/encoder_buffer.h>
#include <draco/compression/encode.h>
#include <draco/compression/decode.h>

#pragma comment(lib, "draco.lib")

static Mesh* s_pBaseMesh = NULL;
static draco::Mesh* s_pDracoMesh = NULL;
static draco::EncoderBuffer s_buffer;



//======================================================================
// Draco圧縮バッファの圧縮前後頂点座標テーブル及びインデックス変換マップを作る
//======================================================================
void GetDracoMeshIndexList(draco::EncoderBuffer &EncoBuffer, std::vector<int>& tbl, std::vector<Point3> &mappedPos, std::vector<Point3>& dracoPos)
{
	tbl.clear();

	draco::Decoder decoder;
	draco::DecoderBuffer buffer;
	buffer.Init((char*)EncoBuffer.data(), EncoBuffer.size());

	auto statusor = decoder.DecodeMeshFromBuffer(&buffer);
	std::unique_ptr<draco::Mesh> in_mesh = std::move(statusor).value();
	draco::Mesh* pMesh = in_mesh.get();
	auto attr = pMesh->GetNamedAttribute(draco::GeometryAttribute::POSITION);
	for (draco::PointIndex i(0); i < attr->indices_map_size(); i++) {
		tbl.push_back(attr->mapped_index(i).value());
		Point3 val;
		attr->GetMappedValue(i, &val);
		mappedPos.push_back(val);
		float v[3];
		attr->GetValue((draco::AttributeValueIndex)i.value(), v);
		dracoPos.push_back(Point3(v));
	}
}

//======================================================================
// 元メッシュ座標とDraco圧縮バッファの圧縮前頂点インデックス変換マップを作る
//======================================================================
void GetDracoMeshIndexList2(std::vector<int>& tbl, std::vector<Point3>& dracoPos, std::vector<Point3>& orgPos)
{
	tbl.clear();
	int i = 0;
	for (auto p : dracoPos) {
		int j = 0;
		for (auto pb : orgPos) {
			if ((p - pb).FLength() < 0.001f) {
				tbl.push_back(j);
				break;
			}
			j++;
		}
		i++;
	}
}

//======================================================================
//　Morphターゲットウエイトテーブルを作る
//======================================================================
void SetDracoMorphTargetPositionTable(Modifier* pMod, int chID, const std::vector<VertexProp>& VertPropTable, std::vector<Point3>& targetPtTbl)
{
	std::vector<Point3> basevertTable;
	std::vector<Point3> targetvertTable;
	std::vector<int> mapTable;

	{
		MaxMorphModifier maxMorphModifier(pMod);
		MaxMorphChannel mc = maxMorphModifier.GetMorphChannel(chID);

		std::vector<int> vIDMapTable;
		std::vector<Point3> mappedPos;
		std::vector<Point3> dracoPos;
		GetDracoMeshIndexList(s_buffer, vIDMapTable, mappedPos, dracoPos);

		for (auto v: VertPropTable) {
			basevertTable.push_back(s_pBaseMesh->verts[v.originalIdx]);
			targetvertTable.push_back(mc.GetMorphPoint(v.originalIdx));
		}
		std::vector<int> list2Table;
		GetDracoMeshIndexList2(list2Table, dracoPos, basevertTable);

		for (int i = 0; i < vIDMapTable.size(); i++) {
			int idx = list2Table[vIDMapTable[i]];
			mapTable.push_back(idx);
		}
	}

	//auto attr = s_pDracoMesh->GetNamedAttribute(draco::GeometryAttribute::POSITION);
	//int attrsize = attr->indices_map_size();
	//for (int i = 0; i < attrsize; i++)targetPtTbl.push_back(Point3());

	targetPtTbl.clear();
	for (auto i: mapTable) {
		Point3 BasePT = basevertTable[i];
		Point3 pt = targetvertTable[i];
		targetPtTbl.push_back(pt - BasePT);
	}
}

//========================================================================
// 　PrimitiveのMorph部分の生成
//========================================================================
void glTFExporter_Core::CreateDracoMorphPrimiteve(tinygltf::Primitive& primitive, Modifier* pMorphMod, std::vector<VertexProp>& VertPropTable)
{
	primitive.targets.clear();
	int cnt = GetMorphTargetNum(pMorphMod);
	for (int i = 0; i < cnt; i++) {
		//Mesh* pTargetMesh = GetMorphTargetMesh(pMorphMod, i);
		std::vector<Point3> targetPtTbl;

		SetDracoMorphTargetPositionTable(pMorphMod, i, VertPropTable, targetPtTbl);

		//----------- Morph TargetPosition
		{
			tinygltf::Accessor acc;// = Create_glTFAccessor();
			acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
			acc.type = TINYGLTF_TYPE_VEC3;
			acc.count = VertPropTable.size();
			tinygltf::BufferView bfView;// = Create_glTFBufferView();
			bfView.buffer = 0;
			bfView.byteOffset = m_BufferByteOffset;
			bfView.byteLength = acc.count * sizeof(float) * 3;
			bfView.target = TINYGLTF_TARGET_ARRAY_BUFFER;

			Point3 minPos = targetPtTbl[0] * m_scale;// pTargetMesh->verts[pFace[faceIDTable[0]].v[0]];
			Point3 maxPos = targetPtTbl[0] * m_scale;//pTargetMesh->verts[pFace[faceIDTable[0]].v[0]];

			void* ptr = SecureMemory(bfView.byteLength);
			float* pPos = (float*)((char*)ptr + bfView.byteOffset);
			for (auto p : targetPtTbl) {
				p *= m_scale;
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
			std::map<std::string, int> w;
			w.insert(std::make_pair("POSITION", m_model.accessors.size() - 1));
			primitive.targets.push_back(w);
		}
	}
}


//========================================================================
// 座標 アトリビュートバッファの登録
//========================================================================
int CreatePosDracoBuffer(draco::Mesh &dracoMesh, Mesh *pMesh, std::vector<int> &faceIDTable, std::vector<VertexProp>& VertPropTable, std::map<int, int>& vertPropMap, const Matrix3 &OffsetTM, float scale)
{
	int numv = VertPropTable.size();// pMesh->numVerts;
	int numf = faceIDTable.size();

	draco::GeometryAttribute dracoGeomAttr;
	dracoGeomAttr.Init(
		draco::GeometryAttribute::POSITION, // attribute type (like POSITION or NORMAL)
		nullptr, // buffer
		3, // number of component
		draco::DT_FLOAT32, // data type
		false, // normalized
		sizeof(float) * 3, // byte stride
		0); // byte offset

	const int posAttrId = dracoMesh.AddAttribute(dracoGeomAttr, false, numv);
	dracoMesh.attribute(posAttrId)->SetExplicitMapping(numv);

	uint32_t vIndex = 0;
	for (auto v : VertPropTable) {
		Point3 p = pMesh->verts[v.originalIdx] *  scale * OffsetTM;
		float v[3];
		v[0] = p.x;
		v[1] = p.y;
		v[2] = p.z;
		dracoMesh.attribute(posAttrId)->SetAttributeValue(draco::AttributeValueIndex(vIndex++), v);
	}

#if 0
	vIndex = 0;
	std::vector<int> indices;
	for (auto fId : faceIDTable) {
		for (int j = 0; j < 3; j++) {
			indices.push_back(vIndex++);
		}
	}

	for (uint32_t vIndex = 0; vIndex < indices.size(); ++vIndex) {
		const draco::PointIndex pIndex(vIndex);
		const draco::AttributeValueIndex avIndex(indices[vIndex]);
		dracoMesh.attribute(posAttrId)->SetPointMapEntry(pIndex, avIndex);
	}

	for (draco::FaceIndex fIndex(0); fIndex < static_cast<uint32_t>(indices.size() / 3); ++fIndex) {
		draco::Mesh::Face dracoFace;
		for (uint32_t cIndex = 0; cIndex < 3; ++cIndex)
			dracoFace[cIndex] = fIndex.value() * 3 + cIndex;
		dracoMesh.SetFace(fIndex, dracoFace);
	}
#else
	auto ptr = VertPropTable.begin();
	for (uint32_t vIndex = 0; vIndex < VertPropTable.size(); ++vIndex, ++ptr) {
		const draco::PointIndex pIndex(vIndex);
		int idx = ptr->faceID * 10 + ptr->corner;
		const draco::AttributeValueIndex avIndex(vertPropMap[idx]);
		dracoMesh.attribute(posAttrId)->SetPointMapEntry(pIndex, avIndex);
	}

	auto fIDptr = faceIDTable.begin();
	for (draco::FaceIndex fIndex(0); fIndex < static_cast<uint32_t>(numf); ++fIndex, ++fIDptr) {
		draco::Mesh::Face dracoFace;
		for (uint32_t cIndex = 0; cIndex < 3; ++cIndex)
			dracoFace[cIndex] = vertPropMap[(*fIDptr) * 10 + cIndex];
		dracoMesh.SetFace(fIndex, dracoFace);
	}

#endif

	return posAttrId;
}

//========================================================================
// 法線 アトリビュートバッファの登録
//========================================================================
int CreateNrmDracoBuffer(draco::Mesh &dracoMesh, Mesh *pMesh, MeshNormalSpec *pNrmSpec, std::vector<int> &faceIDTable, std::vector<VertexProp>& VertPropTable, std::map<int, int>& vertPropMap)
{
	int numv = VertPropTable.size();
	int numf = faceIDTable.size();

	draco::GeometryAttribute dracoNrmAtr;
	dracoNrmAtr.Init(
		draco::GeometryAttribute::NORMAL, // attribute type (like POSITION or NORMAL)
		nullptr, // buffer
		3, // number of component
		draco::DT_FLOAT32, // data type
		false, // normalized
		sizeof(float) * 3, // byte stride
		0); // byte offset
	const int nrmAttrId = dracoMesh.AddAttribute(dracoNrmAtr, false, numv);
	dracoMesh.attribute(nrmAttrId)->SetExplicitMapping(numv);

	uint32_t vIndex = 0;
	for (auto v : VertPropTable) {
		Point3 &nrm = v.normal;
		float v[3];
		v[0] = nrm.x;
		v[1] = nrm.y;
		v[2] = nrm.z;
		dracoMesh.attribute(nrmAttrId)->SetAttributeValue(draco::AttributeValueIndex(vIndex++), v);
	}

#if 0
	vIndex = 0;
	std::vector<int> indices;
	for (auto fId : faceIDTable) {
		for (int j = 0; j < 3; j++) {
			indices.push_back(vIndex++);
		}
	}

	for (uint32_t vIndex = 0; vIndex < indices.size(); ++vIndex) {
		const draco::PointIndex pIndex(vIndex);
		const draco::AttributeValueIndex avIndex(indices[vIndex]);
		dracoMesh.attribute(nrmAttrId)->SetPointMapEntry(pIndex, avIndex);
	}

	for (draco::FaceIndex fIndex(0); fIndex < static_cast<uint32_t>(indices.size() / 3); ++fIndex) {
		draco::Mesh::Face dracoFace;
		for (uint32_t cIndex = 0; cIndex < 3; ++cIndex)
			dracoFace[cIndex] = fIndex.value() * 3 + cIndex;
		dracoMesh.SetFace(fIndex, dracoFace);
	}
#else
	auto ptr = VertPropTable.begin();
	for (uint32_t vIndex = 0; vIndex < VertPropTable.size(); ++vIndex,++ptr) {
		const draco::PointIndex pIndex(vIndex);
		int idx = ptr->faceID * 10 + ptr->corner;
		const draco::AttributeValueIndex avIndex(vertPropMap[idx]);
		dracoMesh.attribute(nrmAttrId)->SetPointMapEntry(pIndex, avIndex);
	}

	auto fIDptr = faceIDTable.begin();
	for (draco::FaceIndex fIndex(0); fIndex < static_cast<uint32_t>(numf); ++fIndex, ++fIDptr) {
		draco::Mesh::Face dracoFace;
		for (uint32_t cIndex = 0; cIndex < 3; ++cIndex)
			dracoFace[cIndex] = vertPropMap[(*fIDptr) * 10 + cIndex];
		dracoMesh.SetFace(fIndex, dracoFace);
	}
#endif

	return nrmAttrId;
}

//========================================================================
// UV座標 アトリビュートバッファの登録
// ※ mapCh=1のみ対応
//========================================================================
int CreateUVDracoBuffer(draco::Mesh &dracoMesh, Mesh *pMesh, std::vector<int> &faceIDTable, std::vector<VertexProp>& VertPropTable, std::map<int, int>& vertPropMap, int mapCh)
{
	int numv = vertPropMap.size();
	int numf = faceIDTable.size();

	draco::GeometryAttribute dracoUVAttr;
	dracoUVAttr.Init(
		draco::GeometryAttribute::TEX_COORD, // attribute type (like POSITION or NORMAL)
		nullptr, // buffer
		2, // number of component
		draco::DT_FLOAT32, // data type
		false, // normalized
		sizeof(float) * 2, // byte stride
		0); // byte offset

	const int uvAttrId = dracoMesh.AddAttribute(dracoUVAttr, false, numv);
	dracoMesh.attribute(uvAttrId)->SetExplicitMapping(numv);

	uint32_t vIndex = 0;
	//MeshMap *pMap = &pMesh->Map(mapCh);
	//UVVert *pSrcUV = pMap->tv;
	for (auto v : VertPropTable) {
		UVVert p = pMesh->mapVerts(mapCh)[v.uv1];
		float v[2];
		v[0] = p.x;
		v[1] = -p.y;
		dracoMesh.attribute(uvAttrId)->SetAttributeValue(draco::AttributeValueIndex(vIndex++), v);
	}

#if 0
	vIndex = 0;
	std::vector<int> indices;
	for (auto fId : faceIDTable) {
		for (int j = 0; j < 3; j++) {
			indices.push_back(vIndex++);
		}
	}

	for (uint32_t vIndex = 0; vIndex < indices.size(); ++vIndex) {
		const draco::PointIndex pIndex(vIndex);
		const draco::AttributeValueIndex avIndex(indices[vIndex]);
		dracoMesh.attribute(uvAttrId)->SetPointMapEntry(pIndex, avIndex);
	}

	for (draco::FaceIndex fIndex(0); fIndex < static_cast<uint32_t>(indices.size() / 3); ++fIndex) {
		draco::Mesh::Face dracoFace;
		for (uint32_t cIndex = 0; cIndex < 3; ++cIndex)
			dracoFace[cIndex] = fIndex.value() * 3 + cIndex;
		dracoMesh.SetFace(fIndex, dracoFace);
	}
#else
	auto ptr = VertPropTable.begin();
	for (uint32_t vIndex = 0; vIndex < VertPropTable.size(); ++vIndex,++ptr) {
		const draco::PointIndex pIndex(vIndex);
		int idx = ptr->faceID * 10 + ptr->corner;
		const draco::AttributeValueIndex avIndex(vertPropMap[idx]);
		dracoMesh.attribute(uvAttrId)->SetPointMapEntry(pIndex, avIndex);
	}

	auto fIDptr = faceIDTable.begin();
	for (draco::FaceIndex fIndex(0); fIndex < static_cast<uint32_t>(numf); ++fIndex, ++fIDptr) {
		draco::Mesh::Face dracoFace;
		for (uint32_t cIndex = 0; cIndex < 3; ++cIndex)
			dracoFace[cIndex] = vertPropMap[(*fIDptr) * 10 + cIndex];
		dracoMesh.SetFace(fIndex, dracoFace);
	}
#endif

	return uvAttrId;
}

//========================================================================
// Tangent アトリビュートバッファの登録
//========================================================================
int CreateTangentDracoBuffer(draco::Mesh& dracoMesh, IGameMesh* pGameMesh, Mtl *pMtl, std::vector<int>& faceIDTable, std::vector<VertexProp>& VertPropTable, std::map<int, int>& vertPropMap, int mapCh)
{
	Matrix3 TangentTM;
	GetTangentTM(pMtl, TangentTM);

	int numv = VertPropTable.size();
	int numf = faceIDTable.size();

	draco::GeometryAttribute dracoTanAttr;
	dracoTanAttr.Init(
		draco::GeometryAttribute::GENERIC, // attribute type (like POSITION or NORMAL)
		nullptr, // buffer
		4, // number of component
		draco::DT_FLOAT32, // data type
		true, // normalized
		sizeof(float) * 4, // byte stride
		0); // byte offset

	const int tanAttrId = dracoMesh.AddAttribute(dracoTanAttr, false, numv);
	dracoMesh.attribute(tanAttrId)->SetExplicitMapping(numv);

	uint32_t vIndex = 0;
	for (auto v : VertPropTable) {
		Point3 normal = pGameMesh->GetNormal(v.faceID, v.corner, TRUE);
		normal.FNormalize();

		int indexTangentBinormal = pGameMesh->GetFaceVertexTangentBinormal(v.faceID, v.corner, mapCh);
		Point3 tangent = pGameMesh->GetTangent(indexTangentBinormal, mapCh) * TangentTM;
		tangent.FNormalize();

		Point3 bitangent = pGameMesh->GetBinormal(indexTangentBinormal, mapCh) * TangentTM;
		bitangent.FNormalize();

		float w = GetW(normal, tangent, bitangent);
		float t[4];
		t[0] = tangent.x;
		t[1] = tangent.y;
		t[2] = tangent.z;
		t[3] = w;
		dracoMesh.attribute(tanAttrId)->SetAttributeValue(draco::AttributeValueIndex(vIndex++), t);
	}

#if 0
	vIndex = 0;
	std::vector<int> indices;
	for (auto fId : faceIDTable) {
		for (int j = 0; j < 3; j++) {
			indices.push_back(vIndex++);
		}
	}

	for (uint32_t vIndex = 0; vIndex < indices.size(); ++vIndex) {
		const draco::PointIndex pIndex(vIndex);
		const draco::AttributeValueIndex avIndex(indices[vIndex]);
		dracoMesh.attribute(tanAttrId)->SetPointMapEntry(pIndex, avIndex);
	}

	for (draco::FaceIndex fIndex(0); fIndex < static_cast<uint32_t>(indices.size() / 3); ++fIndex) {
		draco::Mesh::Face dracoFace;
		for (uint32_t cIndex = 0; cIndex < 3; ++cIndex)
			dracoFace[cIndex] = fIndex.value() * 3 + cIndex;
		dracoMesh.SetFace(fIndex, dracoFace);
	}
#else
	auto ptr = VertPropTable.begin();
	for (uint32_t vIndex = 0; vIndex < VertPropTable.size(); ++vIndex,++ptr) {
		const draco::PointIndex pIndex(vIndex);
		int idx = ptr->faceID * 10 + ptr->corner;
		const draco::AttributeValueIndex avIndex(vertPropMap[idx]);
		dracoMesh.attribute(tanAttrId)->SetPointMapEntry(pIndex, avIndex);
	}

	auto fIDptr = faceIDTable.begin();
	for (draco::FaceIndex fIndex(0); fIndex < static_cast<uint32_t>(numf); ++fIndex, ++fIDptr) {
		draco::Mesh::Face dracoFace;
		for (uint32_t cIndex = 0; cIndex < 3; ++cIndex)
			dracoFace[cIndex] = vertPropMap[(*fIDptr) * 10 + cIndex];
		dracoMesh.SetFace(fIndex, dracoFace);
	}
#endif

	return tanAttrId;
}

//========================================================================
// 頂点カラー アトリビュートバッファの登録
//========================================================================
int CreateColDracoBuffer(draco::Mesh &dracoMesh, Mesh *pMesh, std::vector<int> &faceIDTable, std::vector<VertexProp>& VertPropTable, std::map<int, int>& vertPropMap)
{
	int numv = VertPropTable.size();// pMesh->numVerts;
	int numf = faceIDTable.size();

	draco::GeometryAttribute dracoColAttr;
	dracoColAttr.Init(
		draco::GeometryAttribute::COLOR, // attribute type (like POSITION or NORMAL)
		nullptr, // buffer
		3, // number of component
		draco::DT_FLOAT32, // data type
		false, // normalized
		sizeof(float) * 3, // byte stride
		0); // byte offset

	const int colAttrId = dracoMesh.AddAttribute(dracoColAttr, false, numv);
	dracoMesh.attribute(colAttrId)->SetExplicitMapping(numv);

	uint32_t vIndex = 0;
	//MeshMap *pMap = &pMesh->Map(0);
	//UVVert *pSrcUV = pMap->tv;
	for (auto v : VertPropTable) {
		UVVert p = pMesh->mapVerts(0)[v.vc];
		float v[3];
		v[0] = p.x;
		v[1] = p.y;
		v[2] = p.z;
		dracoMesh.attribute(colAttrId)->SetAttributeValue(draco::AttributeValueIndex(vIndex++), v);
	}

#if 0
	vIndex = 0;
	std::vector<int> indices;
	for (auto fId : faceIDTable) {
		for (int j = 0; j < 3; j++) {
			indices.push_back(vIndex++);
		}
	}

	for (uint32_t vIndex = 0; vIndex < indices.size(); ++vIndex) {
		const draco::PointIndex pIndex(vIndex);
		const draco::AttributeValueIndex avIndex(indices[vIndex]);
		dracoMesh.attribute(colAttrId)->SetPointMapEntry(pIndex, avIndex);
	}

	for (draco::FaceIndex fIndex(0); fIndex < static_cast<uint32_t>(indices.size() / 3); ++fIndex) {
		draco::Mesh::Face dracoFace;
		for (uint32_t cIndex = 0; cIndex < 3; ++cIndex)
			dracoFace[cIndex] = fIndex.value() * 3 + cIndex;
		dracoMesh.SetFace(fIndex, dracoFace);
	}
#else
	auto ptr = VertPropTable.begin();
	for (uint32_t vIndex = 0; vIndex < VertPropTable.size(); ++vIndex) {
		const draco::PointIndex pIndex(vIndex);
		int idx = ptr->faceID * 10 + ptr->corner;
		const draco::AttributeValueIndex avIndex(vertPropMap[idx]);
		dracoMesh.attribute(colAttrId)->SetPointMapEntry(pIndex, avIndex);
	}

	auto fIDptr = faceIDTable.begin();
	for (draco::FaceIndex fIndex(0); fIndex < static_cast<uint32_t>(numf); ++fIndex, ++fIDptr) {
		draco::Mesh::Face dracoFace;
		for (uint32_t cIndex = 0; cIndex < 3; ++cIndex)
			dracoFace[cIndex] = vertPropMap[(*fIDptr) * 10 + cIndex];
		dracoMesh.SetFace(fIndex, dracoFace);
	}
#endif

	return colAttrId;
}

//========================================================================
// Skin Joint アトリビュートバッファの登録
//========================================================================
int CreateJointDracoBuffer(draco::Mesh &dracoMesh, Mesh *pMesh, std::map<int, std::array<USHORT, 4> > &bTable, std::vector<int> &faceIDTable, ISkinContextData *pSkinMC, std::vector<VertexProp>& VertPropTable, std::map<int, int>& vertPropMap)
{
	unsigned int numv = VertPropTable.size();
	unsigned int numf = faceIDTable.size();

	draco::GeometryAttribute dracoJointAtr;
	dracoJointAtr.Init(
		draco::GeometryAttribute::GENERIC, // attribute type (like POSITION or NORMAL)
		nullptr, // buffer
		4, // number of component
		draco::DT_UINT16, // data type
		false, // normalized
		sizeof(USHORT) * 4, // byte stride
		0); // byte offset

	const int jointAttrId = dracoMesh.AddAttribute(dracoJointAtr, false, numv);
	dracoMesh.attribute(jointAttrId)->SetExplicitMapping(numv);

	for (uint32_t vIndex = 0; vIndex < numv; vIndex++) {
		auto b = bTable[vIndex];
		USHORT v[4];
		v[0] = b[0];
		v[1] = b[1];
		v[2] = b[2];
		v[3] = b[3];
		dracoMesh.attribute(jointAttrId)->SetAttributeValue(draco::AttributeValueIndex(vIndex), v);
	}

#if 0
	std::vector<int> indices;
	for (auto fId : faceIDTable) {
		for (int j = 0; j < 3; j++) {
			indices.push_back(vertPropMap[fId * 10 + j]);
		}
	}

	for (uint32_t vIndex = 0; vIndex < indices.size(); ++vIndex) {
		const draco::PointIndex pIndex(vIndex);
		const draco::AttributeValueIndex avIndex(indices[vIndex]);
		dracoMesh.attribute(jointAttrId)->SetPointMapEntry(pIndex, avIndex);
	}

	for (draco::FaceIndex fIndex(0); fIndex < static_cast<uint32_t>(indices.size() / 3); ++fIndex) {
		draco::Mesh::Face dracoFace;
		for (uint32_t cIndex = 0; cIndex < 3; ++cIndex)
			dracoFace[cIndex] = fIndex.value() * 3 + cIndex;
		dracoMesh.SetFace(fIndex, dracoFace);
	}
#else
	auto ptr = VertPropTable.begin();
	for (uint32_t vIndex = 0; vIndex < VertPropTable.size(); ++vIndex, ++ptr) {
		const draco::PointIndex pIndex(vIndex);
		int idx = ptr->faceID * 10 + ptr->corner;
		const draco::AttributeValueIndex avIndex(vertPropMap[idx]);
		dracoMesh.attribute(jointAttrId)->SetPointMapEntry(pIndex, avIndex);
	}

	auto fIDptr = faceIDTable.begin();
	for (draco::FaceIndex fIndex(0); fIndex < static_cast<uint32_t>(numf); ++fIndex, ++fIDptr) {
		draco::Mesh::Face dracoFace;
		for (uint32_t cIndex = 0; cIndex < 3; ++cIndex)
			dracoFace[cIndex] = vertPropMap[(*fIDptr) * 10 + cIndex];
		dracoMesh.SetFace(fIndex, dracoFace);
	}
#endif

	return jointAttrId;
}

//========================================================================
// Skin Weightアトリビュートバッファの登録
//========================================================================
int CreateWeightDracoBuffer(draco::Mesh &dracoMesh, Mesh *pMesh, std::map<int, std::array<float, 4> > &wTable, std::vector<int> &faceIDTable, std::vector<VertexProp>& VertPropTable, std::map<int, int>& vertPropMap)
{
	unsigned int numv = VertPropTable.size();
	unsigned int numf = faceIDTable.size();

	draco::GeometryAttribute dracoWeightAtr;
	dracoWeightAtr.Init(
		draco::GeometryAttribute::GENERIC, // attribute type (like POSITION or NORMAL)
		nullptr, // buffer
		4, // number of component
		draco::DT_FLOAT32, // data type
		false, // normalized
		sizeof(float) * 4, // byte stride
		0); // byte offset
	const int weighttAttrId = dracoMesh.AddAttribute(dracoWeightAtr, false, numv);

	for (uint32_t vIndex = 0; vIndex < numv; vIndex++) {
		auto b = wTable[vIndex];
		float v[4];
		v[0] = b[0];
		v[1] = b[1];
		v[2] = b[2];
		v[3] = b[3];
		dracoMesh.attribute(weighttAttrId)->SetAttributeValue(draco::AttributeValueIndex(vIndex), v);
	}
	dracoMesh.attribute(weighttAttrId)->SetExplicitMapping(numv);

#if 0
	std::vector<int> indices;
	for (auto fId : faceIDTable) {
		for (int j = 0; j < 3; j++) {
			indices.push_back(vertPropMap[fId * 10 + j]);
		}
	}

	for (uint32_t vIndex = 0; vIndex < indices.size(); ++vIndex) {
		const draco::PointIndex pIndex(vIndex);
		const draco::AttributeValueIndex avIndex(indices[vIndex]);
		dracoMesh.attribute(weighttAttrId)->SetPointMapEntry(pIndex, avIndex);
	}

	for (draco::FaceIndex fIndex(0); fIndex < static_cast<uint32_t>(indices.size() / 3); ++fIndex) {
		draco::Mesh::Face dracoFace;
		for (uint32_t cIndex = 0; cIndex < 3; ++cIndex)
			dracoFace[cIndex] = fIndex.value() * 3 + cIndex;
		dracoMesh.SetFace(fIndex, dracoFace);
	}
#else
	auto ptr = VertPropTable.begin();
	for (uint32_t vIndex = 0; vIndex < VertPropTable.size(); ++vIndex, ++ptr) {
		const draco::PointIndex pIndex(vIndex);
		int idx = ptr->faceID * 10 + ptr->corner;
		const draco::AttributeValueIndex avIndex(vertPropMap[idx]);
		dracoMesh.attribute(weighttAttrId)->SetPointMapEntry(pIndex, avIndex);
	}

	auto fIDptr = faceIDTable.begin();
	for (draco::FaceIndex fIndex(0); fIndex < static_cast<uint32_t>(numf); ++fIndex, ++fIDptr) {
		draco::Mesh::Face dracoFace;
		for (uint32_t cIndex = 0; cIndex < 3; ++cIndex)
			dracoFace[cIndex] = vertPropMap[(*fIDptr) * 10 + cIndex];
		dracoMesh.SetFace(fIndex, dracoFace);
	}
#endif

	return weighttAttrId;
}

//========================================================================
//
//
//========================================================================
void glTFExporter_Core::CreateDracoMeshProp(tinygltf::Primitive &primitive, Mesh *pMesh, MeshNormalSpec *pNrmSpec, std::vector<int> &faceIDTable, std::vector<VertexProp>& VertPropTable, std::map<int, int>& vertPropMap, vertPropFlag& flag, Mtl *pMtl, ISkinContextData *pSkinMC, Modifier *pMorphMod, const Matrix3 &OffsetTM)
{
	draco::Mesh dracoMesh;
	tinygltf::Value::Object dracoAttr;

	//----------- Index
	{
		tinygltf::Accessor acc;// = Create_glTFAccessor();
		acc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
		acc.type = TINYGLTF_TYPE_SCALAR;
		acc.count = faceIDTable.size() * 3;

		m_model.accessors.push_back(acc);
		primitive.indices = m_model.accessors.size() - 1;
	}

	//----------- Position
	{
		tinygltf::Accessor acc;// = Create_glTFAccessor();
		acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
		acc.type = TINYGLTF_TYPE_VEC3;
		acc.count = VertPropTable.size();

		Face *pFace = pMesh->faces;
		Point3 minPos = pMesh->verts[pFace[faceIDTable[0]].v[0]] * OffsetTM;;
		Point3 maxPos = pMesh->verts[pFace[faceIDTable[0]].v[0]] * OffsetTM;;

		for (auto id : VertPropTable) {
			Point3 p = pMesh->verts[id.originalIdx] * OffsetTM;
			if (p.x > maxPos.x)	maxPos.x = p.x;
			if (p.y > maxPos.y)	maxPos.y = p.y;
			if (p.z > maxPos.z)	maxPos.z = p.z;
			if (p.x < minPos.x)	minPos.x = p.x;
			if (p.y < minPos.y)	minPos.y = p.y;
			if (p.z < minPos.z)	minPos.z = p.z;
		}

		acc.maxValues.push_back(maxPos.x);
		acc.maxValues.push_back(maxPos.y);
		acc.maxValues.push_back(maxPos.z);
		acc.minValues.push_back(minPos.x);
		acc.minValues.push_back(minPos.y);
		acc.minValues.push_back(minPos.z);

		m_model.accessors.push_back(acc);
		primitive.attributes.insert(std::make_pair("POSITION", m_model.accessors.size() - 1));

		int id = CreatePosDracoBuffer(dracoMesh, pMesh, faceIDTable, VertPropTable, vertPropMap, OffsetTM, m_scale);
		dracoAttr.insert(std::make_pair("POSITION", id));
		//s_posAttrId = id;
	}

	//----------- Normal
	{
		tinygltf::Accessor acc;// = Create_glTFAccessor();
		acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
		acc.type = TINYGLTF_TYPE_VEC3;
		acc.count = VertPropTable.size();

		m_model.accessors.push_back(acc);
		primitive.attributes.insert(std::make_pair("NORMAL", m_model.accessors.size() - 1));

		int id = CreateNrmDracoBuffer(dracoMesh, pMesh, pNrmSpec, faceIDTable, VertPropTable, vertPropMap);
		dracoAttr.insert(std::make_pair("NORMAL", id));
	}
	
	//----------- Tangent
	if (flag.pGameMesh) {
		int mapCh = 1;
		tinygltf::Accessor acc;// = Create_glTFAccessor();
		acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
		acc.type = TINYGLTF_TYPE_VEC4;
		acc.count = VertPropTable.size();

		m_model.accessors.push_back(acc);
		primitive.attributes.insert(std::make_pair("TANGENT", m_model.accessors.size() - 1));

		int id = CreateTangentDracoBuffer(dracoMesh, flag.pGameMesh, pMtl, faceIDTable, VertPropTable, vertPropMap, mapCh);
		dracoAttr.insert(std::make_pair("TANGENT", id));
	}

	//----------- UV1 map
	if (flag.mapCh1Used) {
		int mapCh = 1;
		tinygltf::Accessor acc;// = Create_glTFAccessor();
		acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
		acc.type = TINYGLTF_TYPE_VEC2;
		acc.count = VertPropTable.size();

		MeshMap *pMap = &pMesh->Map(mapCh);
		UVVert *pSrcUV = pMap->tv;
		TVFace *pTVFace = pMesh->mapFaces(mapCh);
		Point3 minUV = pSrcUV[pTVFace[faceIDTable[0]].t[0]] * Point3(1.0f, -1.0f, 0.0f);
		Point3 maxUV = minUV;

		for (auto id : faceIDTable) {
			DWORD *v = pTVFace[id].t;
			for (int pp = 0; pp < 3; pp++) {
				UVVert p = pSrcUV[v[pp]];
				p.y = -p.y;
				if (p.x > maxUV.x)	maxUV.x = p.x;
				if (p.y > maxUV.y)	maxUV.y = p.y;
				if (p.x < minUV.x)	minUV.x = p.x;
				if (p.y < minUV.y)	minUV.y = p.y;
			}
		}

		acc.maxValues.push_back(maxUV.x);
		acc.maxValues.push_back(maxUV.y);
		acc.minValues.push_back(minUV.x);
		acc.minValues.push_back(minUV.y);

		m_model.accessors.push_back(acc);
		primitive.attributes.insert(std::make_pair("TEXCOORD_0", m_model.accessors.size() - 1));

		int id = CreateUVDracoBuffer(dracoMesh, pMesh, faceIDTable, VertPropTable, vertPropMap, mapCh);
		dracoAttr.insert(std::make_pair("TEXCOORD_0", id));
	}

	//----------- UV2 map
	if (flag.mapCh2Used) {
		int mapCh = 2;
		tinygltf::Accessor acc;// = Create_glTFAccessor();
		acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
		acc.type = TINYGLTF_TYPE_VEC2;
		acc.count = VertPropTable.size();

		MeshMap* pMap = &pMesh->Map(mapCh);
		UVVert* pSrcUV = pMap->tv;
		TVFace* pTVFace = pMesh->mapFaces(mapCh);
		Point3 minUV = pSrcUV[pTVFace[faceIDTable[0]].t[0]] * Point3(1.0f, -1.0f, 0.0f);
		Point3 maxUV = minUV;

		for (auto id : faceIDTable) {
			DWORD* v = pTVFace[id].t;
			for (int pp = 0; pp < 3; pp++) {
				UVVert p = pSrcUV[v[pp]];
				p.y = -p.y;
				if (p.x > maxUV.x)	maxUV.x = p.x;
				if (p.y > maxUV.y)	maxUV.y = p.y;
				if (p.x < minUV.x)	minUV.x = p.x;
				if (p.y < minUV.y)	minUV.y = p.y;
			}
		}

		acc.maxValues.push_back(maxUV.x);
		acc.maxValues.push_back(maxUV.y);
		acc.minValues.push_back(minUV.x);
		acc.minValues.push_back(minUV.y);

		m_model.accessors.push_back(acc);
		primitive.attributes.insert(std::make_pair("TEXCOORD_0", m_model.accessors.size() - 1));

		int id = CreateUVDracoBuffer(dracoMesh, pMesh, faceIDTable, VertPropTable, vertPropMap, mapCh);
		dracoAttr.insert(std::make_pair("TEXCOORD_1", id));
	}

	//----------- Vertex Color
	if (flag.VColorUsed) {
		tinygltf::Accessor acc;// = Create_glTFAccessor();
		acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
		acc.type = TINYGLTF_TYPE_VEC3;
		acc.count = VertPropTable.size();

		MeshMap* pMap = &pMesh->Map(0);
		UVVert* pSrcUV = pMap->tv;
		TVFace* pTVFace = pMesh->mapFaces(0);
		Point3 minUV = pSrcUV[pTVFace[faceIDTable[0]].t[0]];
		Point3 maxUV = minUV;

		for (auto id : faceIDTable) {
			DWORD* v = pTVFace[id].t;
			for (int pp = 0; pp < 3; pp++) {
				UVVert p = pSrcUV[v[pp]];
				if (p.x > maxUV.x)	maxUV.x = p.x;
				if (p.y > maxUV.y)	maxUV.y = p.y;
				if (p.z > maxUV.z)	maxUV.z = p.z;
				if (p.x < minUV.x)	minUV.x = p.x;
				if (p.y < minUV.y)	minUV.y = p.y;
				if (p.z < minUV.z)	minUV.z = p.z;
			}
		}

		acc.maxValues.push_back(maxUV.x);
		acc.maxValues.push_back(maxUV.y);
		acc.maxValues.push_back(maxUV.z);
		acc.minValues.push_back(minUV.x);
		acc.minValues.push_back(minUV.y);
		acc.minValues.push_back(minUV.z);

		m_model.accessors.push_back(acc);
		primitive.attributes.insert(std::make_pair("COLOR_0", m_model.accessors.size() - 1));

		int id = CreateColDracoBuffer(dracoMesh, pMesh, faceIDTable, VertPropTable, vertPropMap);
		dracoAttr.insert(std::make_pair("COLOR_0", id));
	}

	//----------- Skin Joint/Weight
	if (pSkinMC) {
		Face *pFace = pMesh->faces;

		std::map<int, std::array<float, 4> > wTable;
		std::map<int, std::array<USHORT, 4> > bTable;
		wTable.clear();
		bTable.clear();

		for (auto id : faceIDTable) {
			DWORD *v = pFace[id].v;
			for (int pp = 0; pp < 3; pp++) {
				int numb = pSkinMC->GetNumAssignedBones(v[pp]);
				if (numb > 4) {
					numb = 4;
					m_IncorrectSkinDataFound = TRUE;
				}
				std::array<float, 4> wa{ 0.0f, 0.0f, 0.0f, 0.0f };
				std::array<USHORT, 4> ba{ 0, 0, 0, 0 };
				for (int j = 0; j < numb; j++) {
					USHORT boneIdx = pSkinMC->GetAssignedBone(v[pp], j);
					float w = pSkinMC->GetBoneWeight(v[pp], j);
					wa[j] = w;
					ba[j] = boneIdx;
				}
				wTable.insert(std::make_pair(v[pp], wa));
				bTable.insert(std::make_pair(v[pp], ba));
			}
		}

		//---------------------------------
		{
			tinygltf::Accessor acc;// = Create_glTFAccessor();
			acc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
			acc.type = TINYGLTF_TYPE_VEC4;
			acc.count = VertPropTable.size();

			m_model.accessors.push_back(acc);
			primitive.attributes.insert(std::make_pair("JOINTS_0", m_model.accessors.size() - 1));

			int id = CreateJointDracoBuffer(dracoMesh, pMesh, bTable, faceIDTable, pSkinMC, VertPropTable, vertPropMap);
			dracoAttr.insert(std::make_pair("JOINTS_0", id));
		}
		//---------------------------------
		{
			tinygltf::Accessor acc;// = Create_glTFAccessor();
			acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
			acc.type = TINYGLTF_TYPE_VEC4;
			acc.count = VertPropTable.size();

			m_model.accessors.push_back(acc);
			primitive.attributes.insert(std::make_pair("WEIGHTS_0", m_model.accessors.size() - 1));

			int id = CreateWeightDracoBuffer(dracoMesh, pMesh, wTable, faceIDTable, VertPropTable, vertPropMap);
			dracoAttr.insert(std::make_pair("WEIGHTS_0", id));
		}

	}

	// バッファをエンコードする
	draco::Encoder encoder;
	encoder.SetSpeedOptions(m_EncodeSpeed, 0);
	//draco::EncoderBuffer buffer;
	const draco::Status status = encoder.EncodeMeshToBuffer(dracoMesh, &s_buffer);

	std::vector<unsigned char> buff;
	buff.clear();
	for (auto d : *s_buffer.buffer()) buff.push_back(d);

	//エンコードバッファサイズを４の倍数にする
	int padding = 4-(buff.size() % 4);
	for(int i=0; i< padding; i++) buff.push_back('0');

	// glTF bufferにコピー
	int offset = m_BufferByteOffset;
	char* ptr = (char*)SecureMemory(buff.size());
	ptr += offset;
	for (auto p : buff) *ptr++ = p;

	tinygltf::BufferView bfView;// = Create_glTFBufferView();
	bfView.buffer = 0;
	bfView.byteOffset = offset;
	bfView.byteLength = buff.size();
	m_model.bufferViews.push_back(bfView);

	//
	tinygltf::Value::Object ary;
	ary.insert(std::make_pair("attributes", dracoAttr));
	ary.insert(std::make_pair("bufferView", tinygltf::Value((int)m_model.bufferViews.size()-1)));

	primitive.extensions.insert(std::make_pair("KHR_draco_mesh_compression", tinygltf::Value(ary)));

	if (pMorphMod) {
		s_pDracoMesh = &dracoMesh;
		s_pBaseMesh = pMesh;
		CreateDracoMorphPrimiteve(primitive, pMorphMod, VertPropTable);
	}
	s_buffer.Clear();
}

#if 0
//========================================================================
//
//
//========================================================================
void glTFExporter_Core::ExCreateDracoMeshProp(tinygltf::Primitive& primitive, Mesh* pMesh, MeshNormalSpec* pNrmSpec, std::vector<VertexProp>& vertPropTable, vertPropFlag& flag, Mtl* pMtl, ISkinContextData* pSkinMC, Modifier* pMorphMod, IGameMesh* pGameMesh, const Matrix3& OffsetTM)
{
	draco::Mesh dracoMesh;
	tinygltf::Value::Object dracoAttr;

	//----------- Index
	{
		tinygltf::Accessor acc;// = Create_glTFAccessor();
		acc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT;
		acc.type = TINYGLTF_TYPE_SCALAR;
		acc.count = vertPropTable.size();

		m_model.accessors.push_back(acc);
		primitive.indices = m_model.accessors.size() - 1;
	}

	//----------- Position
	{
		tinygltf::Accessor acc;// = Create_glTFAccessor();
		acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
		acc.type = TINYGLTF_TYPE_VEC3;
		acc.count = vertPropTable.size();

		//Face* pFace = pMesh->faces;
		Point3 minPos = pMesh->verts[vertPropTable[0].originalIdx] * m_scale * OffsetTM;
		Point3 maxPos = minPos;

		for (auto v : vertPropTable) {
			Point3 p = pMesh->verts[v.originalIdx] * m_scale * OffsetTM;
			if (p.x > maxPos.x)	maxPos.x = p.x;
			if (p.y > maxPos.y)	maxPos.y = p.y;
			if (p.z > maxPos.z)	maxPos.z = p.z;
			if (p.x < minPos.x)	minPos.x = p.x;
			if (p.y < minPos.y)	minPos.y = p.y;
			if (p.z < minPos.z)	minPos.z = p.z;
		}

		acc.maxValues.push_back(maxPos.x);
		acc.maxValues.push_back(maxPos.y);
		acc.maxValues.push_back(maxPos.z);
		acc.minValues.push_back(minPos.x);
		acc.minValues.push_back(minPos.y);
		acc.minValues.push_back(minPos.z);

		m_model.accessors.push_back(acc);
		primitive.attributes.insert(std::make_pair("POSITION", m_model.accessors.size() - 1));

		int id = CreatePosDracoBuffer(dracoMesh, pMesh, vertTable, faceIDTable, OffsetTM, m_scale);
		dracoAttr.insert(std::make_pair("POSITION", id));
		//s_posAttrId = id;
	}

	//----------- Normal
	{
		tinygltf::Accessor acc;// = Create_glTFAccessor();
		acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
		acc.type = TINYGLTF_TYPE_VEC3;
		acc.count = vertPropTable.size();

		//for (auto v : vertPropTable) {
		//}

		m_model.accessors.push_back(acc);
		primitive.attributes.insert(std::make_pair("NORMAL", m_model.accessors.size() - 1));

		int id = CreateNrmDracoBuffer(dracoMesh, pMesh, pNrmSpec, faceIDTable);
		dracoAttr.insert(std::make_pair("NORMAL", id));
	}

	//----------- Tangent
	if (pGameMesh) {
		tinygltf::Accessor acc;// = Create_glTFAccessor();
		acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
		acc.type = TINYGLTF_TYPE_VEC4;
		acc.count = vertPropTable.size();

		m_model.accessors.push_back(acc);
		primitive.attributes.insert(std::make_pair("TANGENT", m_model.accessors.size() - 1));

		int id = CreateTangentDracoBuffer(dracoMesh, pGameMesh, pMtl, faceIDTable);
		dracoAttr.insert(std::make_pair("TANGENT", id));
	}

	//----------- UV1 map
	if (flag.mapCh1Used) {
		tinygltf::Accessor acc;// = Create_glTFAccessor();
		acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
		acc.type = TINYGLTF_TYPE_VEC2;
		acc.count = vertPropTable.size();

		MeshMap* pMap = &pMesh->Map(mapCh);
		UVVert* pSrcUV = pMap->tv;
		//TVFace* pTVFace = pMesh->mapFaces(mapCh);
		Point3 minUV = pSrcUV[vertPropTable[0].uv1] * Point3(1.0f, -1.0f, 0.0f);
		Point3 maxUV = minUV;

		for (auto v : vertPropTable) {
			UVVert p = pSrcUV[v.uv1];
			p.y = -p.y;
			if (p.x > maxUV.x)	maxUV.x = p.x;
			if (p.y > maxUV.y)	maxUV.y = p.y;
			if (p.x < minUV.x)	minUV.x = p.x;
			if (p.y < minUV.y)	minUV.y = p.y;
		}

		acc.maxValues.push_back(maxUV.x);
		acc.maxValues.push_back(maxUV.y);
		acc.minValues.push_back(minUV.x);
		acc.minValues.push_back(minUV.y);

		m_model.accessors.push_back(acc);
		primitive.attributes.insert(std::make_pair("TEXCOORD_0", m_model.accessors.size() - 1));

		int id = CreateUVDracoBuffer(dracoMesh, pMesh, faceIDTable);
		dracoAttr.insert(std::make_pair("TEXCOORD_0", id));
	}

	//----------- UV2 map
	if (flag.mapCh2Used) {
		tinygltf::Accessor acc;// = Create_glTFAccessor();
		acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
		acc.type = TINYGLTF_TYPE_VEC2;
		acc.count = vertPropTable.size();

		MeshMap* pMap = &pMesh->Map(2);
		UVVert* pSrcUV = pMap->tv;
		//TVFace* pTVFace = pMesh->mapFaces(mapCh);
		Point3 minUV = pSrcUV[vertPropTable[0].uv1] * Point3(1.0f, -1.0f, 0.0f);
		Point3 maxUV = minUV;

		for (auto v : vertPropTable) {
			UVVert p = pSrcUV[v.uv2];
			p.y = -p.y;
			if (p.x > maxUV.x)	maxUV.x = p.x;
			if (p.y > maxUV.y)	maxUV.y = p.y;
			if (p.x < minUV.x)	minUV.x = p.x;
			if (p.y < minUV.y)	minUV.y = p.y;
		}

		acc.maxValues.push_back(maxUV.x);
		acc.maxValues.push_back(maxUV.y);
		acc.minValues.push_back(minUV.x);
		acc.minValues.push_back(minUV.y);

		m_model.accessors.push_back(acc);
		primitive.attributes.insert(std::make_pair("TEXCOORD_0", m_model.accessors.size() - 1));

		int id = CreateUVDracoBuffer(dracoMesh, pMesh, faceIDTable);
		dracoAttr.insert(std::make_pair("TEXCOORD_0", id));
	}

	//----------- Vertex Color
	if (flag.VColorUsed) {
		tinygltf::Accessor acc;// = Create_glTFAccessor();
		acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
		acc.type = TINYGLTF_TYPE_VEC3;
		acc.count = vertPropTable.size();

		MeshMap* pMap = &pMesh->Map(0);
		UVVert* pSrcUV = pMap->tv;
		//TVFace* pTVFace = pMesh->mapFaces(0);
		Point3 minUV = pSrcUV[vertPropTable[0].vc];
		Point3 maxUV = minUV;

		for (auto v : vertPropTable) {
			UVVert p = pSrcUV[v.vc];
			if (p.x > maxUV.x)	maxUV.x = p.x;
			if (p.y > maxUV.y)	maxUV.y = p.y;
			if (p.z > maxUV.z)	maxUV.z = p.z;
			if (p.x < minUV.x)	minUV.x = p.x;
			if (p.y < minUV.y)	minUV.y = p.y;
			if (p.z < minUV.z)	minUV.z = p.z;
		}

		acc.maxValues.push_back(maxUV.x);
		acc.maxValues.push_back(maxUV.y);
		acc.maxValues.push_back(maxUV.z);
		acc.minValues.push_back(minUV.x);
		acc.minValues.push_back(minUV.y);
		acc.minValues.push_back(minUV.z);

		m_model.accessors.push_back(acc);
		primitive.attributes.insert(std::make_pair("COLOR_0", m_model.accessors.size() - 1));

		int id = CreateColDracoBuffer(dracoMesh, pMesh, faceIDTable);
		dracoAttr.insert(std::make_pair("COLOR_0", id));
	}

	//----------- Skin Joint/Weight
	if (pSkinMC) {
		Face* pFace = pMesh->faces;

		std::map<int, std::array<float, 4> > wTable;
		std::map<int, std::array<USHORT, 4> > bTable;
		wTable.clear();
		bTable.clear();

		for (auto v : vertPropTable) {
			int numb = pSkinMC->GetNumAssignedBones(v.originalIdx);
			if (numb > 4) numb = 4;
			std::array<float, 4> wa{ 0.0f, 0.0f, 0.0f, 0.0f };
			std::array<USHORT, 4> ba{ 0, 0, 0, 0 };
			for (int j = 0; j < numb; j++) {
				USHORT boneIdx = pSkinMC->GetAssignedBone(v.originalIdx, j);
				float w = pSkinMC->GetBoneWeight(v.originalIdx, j);
				wa[j] = w;
				ba[j] = boneIdx;
			}
			wTable.insert(std::make_pair(v.originalIdx, wa));
			bTable.insert(std::make_pair(vv.originalIdx, ba));
		}

		//---------------------------------
		{
			tinygltf::Accessor acc;// = Create_glTFAccessor();
			acc.componentType = TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT;
			acc.type = TINYGLTF_TYPE_VEC4;
			acc.count = vertPropTable.size();

			m_model.accessors.push_back(acc);
			primitive.attributes.insert(std::make_pair("JOINTS_0", m_model.accessors.size() - 1));

			int id = CreateJointDracoBuffer(dracoMesh, pMesh, bTable, faceIDTable, pSkinMC);
			dracoAttr.insert(std::make_pair("JOINTS_0", id));
		}
		//---------------------------------
		{
			tinygltf::Accessor acc;// = Create_glTFAccessor();
			acc.componentType = TINYGLTF_COMPONENT_TYPE_FLOAT;
			acc.type = TINYGLTF_TYPE_VEC4;
			acc.count = vertPropTable.size();

			m_model.accessors.push_back(acc);
			primitive.attributes.insert(std::make_pair("WEIGHTS_0", m_model.accessors.size() - 1));

			int id = CreateWeightDracoBuffer(dracoMesh, pMesh, wTable, faceIDTable);
			dracoAttr.insert(std::make_pair("WEIGHTS_0", id));
		}

	}

	// バッファをエンコードする
	draco::Encoder encoder;
	encoder.SetSpeedOptions(m_EncodeSpeed, 0);
	//draco::EncoderBuffer buffer;
	const draco::Status status = encoder.EncodeMeshToBuffer(dracoMesh, &s_buffer);

	std::vector<unsigned char> buff;
	buff.clear();
	for (auto d : *s_buffer.buffer()) buff.push_back(d);

	//エンコードバッファサイズを４の倍数にする
	int padding = 4 - (buff.size() % 4);
	for (int i = 0; i < padding; i++) buff.push_back('0');

	// glTF bufferにコピー
	int offset = m_BufferByteOffset;
	char* ptr = (char*)SecureMemory(buff.size());
	ptr += offset;
	for (auto p : buff) *ptr++ = p;

	tinygltf::BufferView bfView;// = Create_glTFBufferView();
	bfView.buffer = 0;
	bfView.byteOffset = offset;
	bfView.byteLength = buff.size();
	m_model.bufferViews.push_back(bfView);

	//
	tinygltf::Value::Object ary;
	ary.insert(std::make_pair("attributes", dracoAttr));
	ary.insert(std::make_pair("bufferView", tinygltf::Value((int)m_model.bufferViews.size() - 1)));

	primitive.extensions.insert(std::make_pair("KHR_draco_mesh_compression", tinygltf::Value(ary)));

	if (pMorphMod) {
		s_pDracoMesh = &dracoMesh;
		s_pBaseMesh = pMesh;
		CreateDracoMorphPrimiteve(primitive, pMorphMod, faceIDTable);
	}
	s_buffer.Clear();
}
#endif

#else
void glTFExporter_Core::CreateDracoMeshProp(tinygltf::Primitive &primitive, Mesh *pMesh, MeshNormalSpec *pNrmSpec, std::vector<Point3> &vertTable, std::vector<int> &faceIDTable, BOOL CVertMode, Mtl *pMtl, ISkinContextData *pSkinMC)
{
}
#endif



