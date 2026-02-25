//**************************************************************************/
//
//
//**************************************************************************/

#include "HSglTFImporter.h"

#ifdef DRACO_ENABLED

#pragma comment(lib, "draco.lib")

#undef max
#undef min
//#include <cinttypes>
//#include "draco/core/decoder_buffer.h"
//#include "draco/compression/config/compression_shared.h"
#include "draco/compression/decode.h"
//#include "draco/compression/mesh/mesh_decoder.h"

//=======================================================================
// 圧縮バッファよりメッシュを取り出し、構成面より頂点IDリストを作る
//=======================================================================
void GetDracoMeshIndexList(cgltf_buffer_view* bufferView, std::vector<float> &tbl)
{
	tbl.clear();

	cgltf_buffer *data = bufferView->buffer;

	draco::Decoder decoder;
	draco::DecoderBuffer buffer;
	buffer.Init((char*)data->data+ bufferView->offset, data->size);
	const draco::StatusOr<draco::EncodedGeometryType> geom_type = decoder.GetEncodedGeometryType(&buffer);
	if (geom_type.value() == draco::TRIANGULAR_MESH) {
		auto statusor = decoder.DecodeMeshFromBuffer(&buffer);
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
void DracoTest(cgltf_buffer_view *bufferView, std::vector<float> &tbl, DracoDecodeType type)
{
	tbl.clear();

	cgltf_buffer *data = bufferView->buffer;

	draco::Decoder decoder;
	draco::DecoderBuffer buffer;
	buffer.Init((char*)data->data+ bufferView->offset, data->size);
	const draco::StatusOr<draco::EncodedGeometryType> geom_type = decoder.GetEncodedGeometryType(&buffer);
	if (geom_type.value() == draco::TRIANGULAR_MESH) {
		auto statusor = decoder.DecodeMeshFromBuffer(&buffer);
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

#else
void GetDracoMeshIndexList(cgltf_buffer_view* bufferView, std::vector<float>& tbl) {}
void DracoTest(cgltf_buffer_view* bufferView, std::vector<float>& tbl, DracoDecodeType type){}
#endif
