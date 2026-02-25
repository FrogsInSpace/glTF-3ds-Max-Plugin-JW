
#include "HSglTFExporter.h"
#include <AssetManagement/iassetmanager.h>

#if 0
std::vector<tstring> AssetTable;
std::map<INode*, UINT> nodeIndexMap;

//======================================================================
// 山椒シーンファイルテーブル(AssetTable)を作成
//======================================================================
int CreateAssetTable(const tstring& filename)
{
	INode* pRootNode = GetCOREInterface()->GetRootNode();
	int RefCnt = pRootNode->GetXRefFileCount();

	AssetTable.clear();
	nodeIndexMap.clear();

	AssetTable.push_back(filename);

	for (int i = 0; i < RefCnt; i++)
	{
		INode* pNode = pRootNode->GetXRefParent(i);
		MaxSDK::AssetManagement::AssetUser asset = pRootNode->GetXRefFile(i);
		tstring fname = tstring(asset.GetFileName());
		auto it = std::find(AssetTable.begin(), AssetTable.end(), fname);
		if (it == AssetTable.end()) {
			nodeIndexMap.insert(std::make_pair(pNode, AssetTable.size()));
			AssetTable.push_back(fname);
		}
		else {
			auto idx = std::distance(AssetTable.begin(), it);
			nodeIndexMap.insert(std::make_pair(pNode, idx));
		}
	}

	return AssetTable.size();
}

//======================================================================
// 指定ノードを含む階層内にnodeIndexMapリスト内のノードが含まれるか
// 含まれる場合にtrueを返すが、指定ノード自体がリスト内にある場合は
// assetIdxにインデクスを渡す
//======================================================================
BOOL findNodeRec(INode* pNode, int &assetIdx)
{

	for (auto p : nodeIndexMap) {
		if (p.first == pNode) {
			if(assetIdx == -1)	assetIdx = p.second;
			return TRUE;
		}
	}

	assetIdx = -2;
	for (int i = 0; i < pNode->NumChildren(); i++) {
		int ret = findNodeRec(pNode->GetChildNode(i), assetIdx);
		if (ret) return ret;
	}

	return FALSE;
}

//======================================================================
// ノードツリーを構築
//======================================================================
int CreateNodeTreeRec(tinygltf::Model &model, INode* pParentNode)
{
	tinygltf::Node node;

	int assetIdx = -1;
	if (findNodeRec(pParentNode, assetIdx)) {
		node.name = WStringToString(pParentNode->GetName());
		if (assetIdx >= 0) {
			node.asset = assetIdx;
		}
		model.nodes.push_back(node);

		for (int i = 0; i < pParentNode->NumChildren(); i++) {
			int idx = CreateNodeTreeRec(model, pParentNode->GetChildNode(i));
			if (idx >= 0) node.children.push_back(idx);
		}

		return (model.nodes.size() - 1);
	}

	return -1;
}

//======================================================================
//======================================================================
void CreateAssets(tinygltf::Model &model)
{
	for (auto asset : AssetTable){
		tinygltf::AssetItem item;
		item.uri = WStringToString(asset);
		model.assets.push_back(item);
	}
}

//======================================================================
//======================================================================
BOOL glTFExporter_Core::CreateGLTFXFile(const tstring& filename)
{

	if (CreateAssetTable(filename) == 0) return FALSE;

	tinygltf::Model model;
	tinygltf::Scene scene;

	INode* pRootNode = GetCOREInterface()->GetRootNode();
	for (int i = 0; i < pRootNode->NumChildren(); i++) {
		int idx = CreateNodeTreeRec(model, pRootNode->GetChildNode(i));
		if(idx>=0) scene.nodes.push_back(idx);
	}

	CreateAssets(model);

	//CreateSceneData(scene);
	//SetSceneExtras(scene);

	model.scenes.push_back(scene);

	tinygltf::TinyGLTF gltf;
	//gltf.SetImageWriter(nullptr, nullptr);

	std::filesystem::path fname(filename);

	Class_ID cid = HSglTFExporter_CLASS_ID;
	//GetCOREInterface()->FileHold();
	int RefCnt = pRootNode->GetXRefFileCount();
	for (int i = 0; i < RefCnt; i++) {
		pRootNode->SetXRefFlags(i, XREF_HIDDEN, TRUE);
	}

	GetCOREInterface()->ExportToFile(filename.c_str(), TRUE, 0, &cid);
	//GetCOREInterface()->FileFetch();
	for (auto asset: AssetTable) {
	}

	fname.replace_extension(".gltfx");
	gltf.WriteGltfSceneToFile(
		&model,
		fname.string(),
		false,	// embedImages
		false,	// embedBuffers
		true,	// pretty print
		false); // write binary

	return TRUE;
}
#else
BOOL glTFExporter_Core::CreateGLTFXFile(const tstring& filename) { return FALSE; }
#endif
