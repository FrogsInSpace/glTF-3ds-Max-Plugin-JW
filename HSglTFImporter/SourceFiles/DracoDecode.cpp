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

#undef max
#undef min
#include "draco/compression/decode.h"

#if _MSC_VER >= 1930    // Visual Studio 2022 (v143)
#ifdef NDEBUG
#pragma comment(lib, "draco/lib/vs2022/Release/draco.lib")
#else
#pragma comment(lib, "draco/lib/vs2022/Debug/draco.lib")
#endif
#elif _MSC_VER >= 1920    // Visual Studio 2019 (v142)
#ifdef NDEBUG
#pragma comment(lib, "draco/lib/vs2019/Release/draco.lib")
#else
#pragma comment(lib, "draco/lib/vs2019/Debug/draco.lib")
#endif
#else    //
#ifdef NDEBUG
#pragma comment(lib, "draco/lib/vs2017/Release/draco.lib")
#else
#pragma comment(lib, "draco/lib/vs2017/Debug/draco.lib")
#endif
#endif


//=======================================================================
// Create vertex ID list from compressed buffer
//=======================================================================
void GetDracoMeshIndexList(cgltf_buffer_view* bufferView, std::vector<float> &tbl)
{
	tbl.clear();

	cgltf_buffer *data = bufferView->buffer;
	if (!data || !data->data) return;
	if (bufferView->offset >= data->size) return;

	draco::Decoder decoder;
	draco::DecoderBuffer buffer;
	buffer.Init((char*)data->data+ bufferView->offset, data->size);
	const draco::StatusOr<draco::EncodedGeometryType> geom_type = decoder.GetEncodedGeometryType(&buffer);
	if (geom_type.value() == draco::TRIANGULAR_MESH) {
		auto statusor = decoder.DecodeMeshFromBuffer(&buffer);
		if (!statusor.ok()) { tbl.clear(); return; }
		std::unique_ptr<draco::Mesh> in_mesh = std::move(statusor).value();

		if (in_mesh) {
			draco::Mesh *pMesh = in_mesh.get();
			for (draco::FaceIndex i(0); i < pMesh->num_faces(); i++) {
				draco::Mesh::Face face = pMesh->face(i);
				tbl.push_back(face[0].value());
				tbl.push_back(face[1].value());
				tbl.push_back(face[2].value());
			}
		}
	}
}

//=======================================================================
//
//=======================================================================
void DracoDecodeProc(cgltf_buffer_view *bufferView, std::vector<float> &tbl, DracoDecodeType type)
{
	tbl.clear();

	cgltf_buffer *data = bufferView->buffer;

	draco::Decoder decoder;
	draco::DecoderBuffer buffer;
	buffer.Init((char*)data->data+ bufferView->offset, data->size);
	const draco::StatusOr<draco::EncodedGeometryType> geom_type = decoder.GetEncodedGeometryType(&buffer);
	if (!geom_type.ok()) { tbl.clear(); return; }

	if (geom_type.value() == draco::TRIANGULAR_MESH) {
		auto statusor = decoder.DecodeMeshFromBuffer(&buffer);
		if (!statusor.ok()) { tbl.clear(); return; }
		std::unique_ptr<draco::Mesh> in_mesh = std::move(statusor).value();
		if (in_mesh) {
			draco::Mesh *pMesh = in_mesh.get();
			if(type== DracoDecodeType::POSITION) {
				auto attr = pMesh->GetNamedAttribute(draco::GeometryAttribute::POSITION);
				if (!attr) return;
				if (attr->is_mapping_identity()) {
					for (draco::AttributeValueIndex  i(0); i < attr->size(); ++i) {
						std::array<float, 3> value;
						attr->ConvertValue<float, 3>(i, &value[0]);
						tbl.push_back(value[0]);
						tbl.push_back(value[1]);
						tbl.push_back(value[2]);
					}
				} else {
					for (draco::PointIndex i(0); i < attr->indices_map_size(); ++i) {
						draco::AttributeValueIndex idx = attr->mapped_index(i);
						std::array<float, 3> value;
						attr->ConvertValue<float, 3>(idx, &value[0]);
						tbl.push_back(value[0]);
						tbl.push_back(value[1]);
						tbl.push_back(value[2]);
					}
				}
			}

			if (type == DracoDecodeType::NORMAL) {
				auto attr = pMesh->GetNamedAttribute(draco::GeometryAttribute::NORMAL);
				if (!attr) return;
				if (attr->is_mapping_identity()) {
					for (draco::AttributeValueIndex i(0); i < attr->size(); ++i) {
						std::array<float, 3> value;
						attr->ConvertValue<float, 3>(i, &value[0]);
						tbl.push_back(value[0]);
						tbl.push_back(value[1]);
						tbl.push_back(value[2]);
					}
				}
				else {
					for (draco::PointIndex i(0); i < attr->indices_map_size(); ++i) {
						draco::AttributeValueIndex idx = attr->mapped_index(i);
						std::array<float, 3> value;
						attr->ConvertValue<float, 3>(idx, &value[0]);
						tbl.push_back(value[0]);
						tbl.push_back(value[1]);
						tbl.push_back(value[2]);
					}
				}
			}

			if (type == DracoDecodeType::COLOR) {
				auto attr = pMesh->GetNamedAttribute(draco::GeometryAttribute::COLOR);
				if (!attr) return;
				if (attr->is_mapping_identity()) {
					for (draco::AttributeValueIndex i(0); i < attr->size(); ++i) {
						std::array<float, 4> value;
						attr->ConvertValue<float, 4>(i, &value[0]);
						tbl.push_back(value[0]);
						tbl.push_back(value[1]);
						tbl.push_back(value[2]);
						tbl.push_back(value[3]);
					}
				}
				else {
					for (draco::PointIndex i(0); i < attr->indices_map_size(); ++i) {
						draco::AttributeValueIndex idx = attr->mapped_index(i);
						std::array<float, 4> value;
						attr->ConvertValue<float, 4>(idx, &value[0]);
						tbl.push_back(value[0]);
						tbl.push_back(value[1]);
						tbl.push_back(value[2]);
						tbl.push_back(value[3]);
					}
				}
			}

			if (type == DracoDecodeType::TEX_COORD) {
				auto attr = pMesh->GetNamedAttribute(draco::GeometryAttribute::TEX_COORD);
				if (!attr) return;
				if (attr->is_mapping_identity()) {
					for (draco::AttributeValueIndex i(0); i < attr->size(); ++i) {
						std::array<float, 2> value;
						attr->ConvertValue<float, 2>(i, &value[0]);
						tbl.push_back(value[0]);
						tbl.push_back(value[1]);
					}
				}
				else {
					for (draco::PointIndex i(0); i < attr->indices_map_size(); ++i) {
						draco::AttributeValueIndex idx = attr->mapped_index(i);
						std::array<float, 2> value;
						attr->ConvertValue<float, 2>(idx, &value[0]);
						tbl.push_back(value[0]);
						tbl.push_back(value[1]);
					}
				}
			}

			if (type == DracoDecodeType::WEIGHTS) {
				auto attr = pMesh->GetNamedAttribute(draco::GeometryAttribute::GENERIC, 1);
				if (!attr) return;
				if (attr->is_mapping_identity()) {
					for (draco::AttributeValueIndex i(0); i < attr->size(); ++i) {
						std::array<float, 4> value;
						attr->ConvertValue<float, 4>(i, &value[0]);
						tbl.push_back(value[0]);
						tbl.push_back(value[1]);
						tbl.push_back(value[2]);
						tbl.push_back(value[3]);
					}
				}
				else {
					for (draco::PointIndex i(0); i < attr->indices_map_size(); ++i) {
						draco::AttributeValueIndex idx = attr->mapped_index(i);
						std::array<float, 4> value;
						attr->ConvertValue<float, 4>(idx, &value[0]);
						tbl.push_back(value[0]);
						tbl.push_back(value[1]);
						tbl.push_back(value[2]);
						tbl.push_back(value[3]);
					}
				}
			}

			if (type == DracoDecodeType::JOINTS) {
				auto attr = pMesh->GetNamedAttribute(draco::GeometryAttribute::GENERIC, 0);
				if (!attr) return;
				if (attr->is_mapping_identity()) {
					for (draco::AttributeValueIndex i(0); i < attr->size(); ++i) {
						std::array<float, 4> value;
						attr->ConvertValue<float, 4>(i, &value[0]);
						tbl.push_back(value[0]);
						tbl.push_back(value[1]);
						tbl.push_back(value[2]);
						tbl.push_back(value[3]);
					}
				}
				else {
					for (draco::PointIndex i(0); i < attr->indices_map_size(); ++i) {
						draco::AttributeValueIndex idx = attr->mapped_index(i);
						std::array<float, 4> value;
						attr->ConvertValue<float, 4>(idx, &value[0]);
						tbl.push_back(value[0]);
						tbl.push_back(value[1]);
						tbl.push_back(value[2]);
						tbl.push_back(value[3]);
					}
				}
			}
		}
	}
	else if (geom_type.value() == draco::POINT_CLOUD) {
		auto pc = decoder.DecodePointCloudFromBuffer(&buffer);
		//std::unique_ptr< std::unique_ptr<draco::PointCloud> > pc = draco::Decoder::DecodePointCloudFromBuffer(&buffer);
	}
}


