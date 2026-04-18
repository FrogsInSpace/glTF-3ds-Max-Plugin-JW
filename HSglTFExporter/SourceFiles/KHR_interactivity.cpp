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
#include <sstream>
#include <string>

static int s_ExportMode = 0;
static float s_scale = 1.0f;

#define ITEM_TYPE (_T("ItemType"))
#define GRAPH_INDEX (_T("Index"))
#define GRAPH_TYPE (_T("GraphType"))
#define GRAPH_TYPESTR (_T("GraphTypeStr"))
#define GRAPH_NUMOUT (_T("NumOutSocket"))
#define GRAPH_NUMIN (_T("NumInSocket"))
#define GRAPH_NUMCONFIG (_T("NumConfigParams"))
#define GRAPH_EXTENSIONSTR (_T("GraphExtensionStr"))

#define HSglTFImporter_CLASS_ID		Class_ID(0x87a91a65, 0xa2560866)
#define HSGLTFIMP_INTERFACE_ID		Interface_ID(0xc230161, 0x37060a2b)


static std::string TypeStrArray[] = {
	("nan"),
	("bool"),
	("int"),
	("float"),
	("float2"),
	("float3"),
	("float4"),
	("float2x2"),
	("float3x3"),
	("float4x4"),
	("string"),
};
enum socketDataType {
	dataFlow = -1, dataNan = 0, dataBool = 1, dataInt = 2, dataFloat = 3, dataFloat2 = 4, dataFloat3 = 5, dataFloat4 = 6, dataFloat2x2 = 7, dataFloat3x3 = 8, dataFloat4x4 = 9, dataString = 10, dataPointer = 11, dataNodeIndex = 12,
};
enum ItemType { none, node, variable, event };

//======================================
struct socketInfo {
	std::string name;
	int type;
};
struct nodeInfo {
	std::string extensionName;
	std::vector<socketInfo> InSocket;
	std::vector<socketInfo> OutSocket;
};
std::map<std::string, nodeInfo> extensionMapTable;
//======================================


static std::vector<std::string> declarationTable;
static std::vector<socketDataType> typeTable;
static std::vector<INode*> nodeNodeTbl;
static std::vector<INode*> eventNodeTbl;
static std::vector<INode*> variableNodeTbl;

static void SetActiveCameraInfo(tinygltf::Value::Object& camera);


//======================================================================
//======================================================================
bool isNumber(const tstring& s)
{
	if (s.empty()) return false;

	try {
		size_t pos;
		[[maybe_unused]] double val = std::stod(s, &pos);		// float  int  OK
		//(void)val;
		return pos == s.size(); // Numeric
	}
	catch (...) {
		return false;
	}
}

//======================================================================
//======================================================================
Animatable* GetAnimByUniqueID(DWORD id)
{
	FPInterface* p = GetInterface(SCENE_IMPORT_CLASS_ID, HSglTFImporter_CLASS_ID, HSGLTFIMP_INTERFACE_ID);
	if (!p) return 0;
	FunctionID fid = p->FindFn(_T("GetObjByUniqueID"));

	FPValue result;
	FPParams params(1, TYPE_DWORD, id);
	p->Invoke(fid, result, &params);

	return result.r;
}

//======================================================================
//======================================================================
void ReplacePointerIndex(tstring &str, int index)
{
	tstring from = _T("/0/");
	tstring to = _T("/") + to_tstring(index)+ _T("/");

	size_t start_pos = str.find(from);
	if (start_pos != std::string::npos) {
		str.replace(start_pos, from.length(), to);
	}
}

//======================================================================
//======================================================================
void InitTypeTable(void)
{
	typeTable.clear();

	if (s_ExportMode == 1) {
		typeTable.push_back(socketDataType::dataBool);
		typeTable.push_back(socketDataType::dataInt);
		typeTable.push_back(socketDataType::dataFloat);
		typeTable.push_back(socketDataType::dataFloat2);
		typeTable.push_back(socketDataType::dataFloat3);
		typeTable.push_back(socketDataType::dataFloat4);
		typeTable.push_back(socketDataType::dataFloat2x2);
		typeTable.push_back(socketDataType::dataFloat3x3);
		typeTable.push_back(socketDataType::dataFloat4x4);
		typeTable.push_back(socketDataType::dataString);
	}
}

//======================================================================
// 使用しているtypeテーブルの生成
//======================================================================
int SetTypeTable(socketDataType type)
{
	if (type == (socketDataType)dataFlow) return -1;

	for (int i = 0; i < typeTable.size(); i++) {
		if (type == typeTable[i]) return i;
	}
	typeTable.push_back(type);

	return (int)(typeTable.size()-1);
}

//======================================================================
// typeテーブルをtinygltf::Value::Arrayにして返す
//======================================================================
void SetTypesTable(tinygltf::Value::Array& types)
{
	for (auto p: typeTable) {
		tinygltf::Value::Object o;
		if (p == socketDataType::dataString) {
			std::string str("custom");
			o.insert(std::make_pair("signature", tinygltf::Value(str)));

			tinygltf::Value::Object dmy;
			//dmy.pu(tinygltf::Value(0));
			tinygltf::Value::Object val;
			val.insert(std::make_pair("AMZN_interactivity_string", dmy));
			o.insert(std::make_pair("extensions", val));

			types.push_back(tinygltf::Value(o));
		}
		else {
			std::string str(TypeStrArray[(int)p]);
			o.insert(std::make_pair("signature", tinygltf::Value(str)));
			types.push_back(tinygltf::Value(o));
		}
	}
}

//======================================================================
// Operationを登録する。すでにある場合はインデクスを返す
//=====================================================================
int SetDeclarationTable(const std::string &op)
{
	auto it = std::find(declarationTable.begin(), declarationTable.end(), op);

	// すでに存在する場合、そのインデックスを返す
	if (it != declarationTable.end()) {
		return (int)std::distance(declarationTable.begin(), it);
	}

	// 存在しない場合、新しく追加し、そのインデックスを返す
	declarationTable.push_back(op);
	return (int)(declarationTable.size() - 1);
}

//======================================================================
// ノードのindexを返す(プロパティのindexでなく、出力順位のindex)
//======================================================================
int GetNodeIndex(INode *pNode)
{
	if (!pNode) return -1;

	for (int i = 0; i < nodeNodeTbl.size(); i++) {
		if (nodeNodeTbl.at(i) == pNode) return i;
	}

	return -1;
}

//======================================================================
// 指定インデックスを持つノードを返す
//======================================================================
INode *GetIndexedNode(int index)
{
	for (auto p : nodeNodeTbl) {
		int idx = -1;
		p->GetUserPropInt(GRAPH_INDEX, idx);
		if (index == idx) return p;
	}

	return NULL;
}

//======================================================================
// 指定ノードの指定ソケットに接続する親側ノードを返す
//======================================================================
INode *GetParentNode(int nodeIdx, int socketIdx, tstring &socketName)
{
	for (auto pNode : nodeNodeTbl) {
		int numOutConnect;
		pNode->GetUserPropInt(GRAPH_NUMOUT, numOutConnect);
		for (int OutConnectIdx = 0; OutConnectIdx < numOutConnect; OutConnectIdx++) {
			tstring str = _T("NumOutSub") + to_tstring(OutConnectIdx);
			int NumOutSub = 0;
			pNode->GetUserPropInt(str.c_str(), NumOutSub);
			tstring SocketStr = _T("OutSocketParam") + to_tstring(OutConnectIdx);
			tstring SocketStr1 = SocketStr + _T("_name");
			tstring SocketStr2 = SocketStr + _T("_type");
			tstring SocketStr3 = SocketStr + _T("_value");
			TSTR sname;
			pNode->GetUserPropString(SocketStr1.c_str(), sname);
			socketName = tstring(sname);
			for (int subCnt = 0; subCnt < NumOutSub; subCnt++) {
				tstring str = _T("OutConnect") + to_tstring(OutConnectIdx) + _T("_") + to_tstring(subCnt);
				tstring str2 = str + +_T("_") + _T("node");
				tstring str3 = str + +_T("_") + _T("index");
				int node2;
				int index2;
				pNode->GetUserPropInt(str2.c_str(), node2);
				pNode->GetUserPropInt(str3.c_str(), index2);
				if (node2 == nodeIdx && index2 == socketIdx) return pNode;
			}
		}
	}
	return NULL;
}

//======================================================================
// 指定ノードの指定ソケットに接続する子側ノードを返す(対象のソケット名も返す)
//======================================================================
INode *GetChildNode(INode *pNode, int socketIdx, int subIdx, TSTR &name)
{
	tstring str = _T("OutConnect") + to_tstring(socketIdx) + _T("_") + to_tstring(subIdx);
	tstring str2 = str +_T("_node");
	tstring str3 = str +_T("_index");

	int node = -1;
	pNode->GetUserPropInt(str2.c_str(), node);
	int index = -1;
	pNode->GetUserPropInt(str3.c_str(), index);
	INode *pChNode = GetIndexedNode(node);
	if(pChNode){
		tstring str = _T("InSocketParam") + to_tstring(index);
		tstring str3 = str + _T("_name");
		pChNode->GetUserPropString(str3.c_str(), name);
	}

	return pChNode;
}

//======================================================================
// 文字列より数字配列
// "0,0,0" -> [0,0,0]
//======================================================================
void SetValueArray(tstring &str, socketDataType type, tinygltf::Value::Array &ary)
{
	wchar_t delimiter = ',';
	std::wistringstream wiss(str);
	tstring token;

	ary.clear();
	if (type == socketDataType::dataBool) {
		bool b = str == _T("true");
		ary.push_back(tinygltf::Value(b));
		return;
	}
	while (std::getline(wiss, token, delimiter)) {
		if (type == socketDataType::dataInt) {
			int i = std::stoi(token);
			ary.push_back(tinygltf::Value(i));
		}
		if ((type == socketDataType::dataFloat) ||
			(type == socketDataType::dataFloat2) ||
			(type == socketDataType::dataFloat3) ||
			(type == socketDataType::dataFloat4) ||
			(type == socketDataType::dataFloat2x2) ||
			(type == socketDataType::dataFloat3x3) ||
			(type == socketDataType::dataFloat4x4)) {
			float i = 0.0;
			if(isNumber(token))	i = std::stof(token);
			ary.push_back(tinygltf::Value(i));
		}
	}
}
//======================================================================
//======================================================================
int findVariableIndex(const TSTR &name, int type)
{
	for (int i = 0; i < variableNodeTbl.size(); i++) {
		auto p = variableNodeTbl[i];
		TSTR id;
		p->GetUserPropString(_T("HSVariableID"), id);
		if (id == name) return i;
	}
	return - 1;
}
//======================================================================
//======================================================================
int findEventIndex(const TSTR &name, int type)
{
	for (int i = 0; i < eventNodeTbl.size(); i++) {
		auto p = eventNodeTbl[i];
		TSTR id;
		p->GetUserPropString(_T("HSEventID"), id);
		if (id == name) return i;
	}
	return -1;
}
//======================================================================
//
//======================================================================
void glTFExporter_Core::CreateDeclarationExtensionInfo(INode *pNode, const std::string &str)
{
	TSTR extensionStr;
	pNode->GetUserPropString(GRAPH_EXTENSIONSTR, extensionStr);
	if (extensionStr.length() > 1) {
		nodeInfo nInfo;
		nInfo.extensionName = WStringToString(extensionStr.data());

		int numInConnect = 0;
		pNode->GetUserPropInt(GRAPH_NUMIN, numInConnect);
		for (int i = 0; i < numInConnect; i++) {
			tstring str = _T("InSocketParam") + to_tstring(i);
			tstring str2 = str + _T("_name");
			tstring str3 = str + _T("_type");
			TSTR data1;
			int type = 0;
			pNode->GetUserPropString(str2.c_str(), data1);
			pNode->GetUserPropInt(str3.c_str(), type);
			socketInfo info;
			info.name = WStringToString(data1.data());
			info.type = SetTypeTable((socketDataType)type);
			if (info.type != -1) nInfo.InSocket.push_back(info);
		}

		int numOutConnect;
		pNode->GetUserPropInt(GRAPH_NUMOUT, numOutConnect);
		for (int OutConnectIdx = 0; OutConnectIdx < numOutConnect; OutConnectIdx++) {
			tstring str = _T("OutSocketParam") + to_tstring(OutConnectIdx);
			tstring str2 = str + _T("_name");
			tstring str3 = str + _T("_type");
			TSTR data1;
			int type = 0;
			pNode->GetUserPropString(str2.c_str(), data1);
			pNode->GetUserPropInt(str3.c_str(), type);
			socketInfo info;
			info.name = WStringToString(data1.data());
			info.type = SetTypeTable((socketDataType)type);
			if (info.type != -1) nInfo.OutSocket.push_back(info);
		}
		extensionMapTable.insert(std::make_pair(str, nInfo));
	}

}

//======================================================================
// ノードの生成
//======================================================================
void glTFExporter_Core::SetNodeTable(INode *pNode, tinygltf::Value::Array& nodes)
{
	size_t s = m_NodeMap.size();


	BOOL PointerSegmentIDFound = FALSE;

	TSTR tstr;
	pNode->GetUserPropString(GRAPH_TYPESTR, tstr);
	std::string str = WStringToString(tstring(tstr));

	tstring partA;
	size_t pos = tstring(tstr).find('/');
	if (pos != std::string::npos)	partA = tstring(tstr).substr(0, pos);

	int nodeIdx = -1;
	pNode->GetUserPropInt(GRAPH_INDEX, nodeIdx);

	tinygltf::Value::Object node;

	if (s_ExportMode == 1) {
		if (str == "event/onSelect") str = "node/onSelect";
	}

	int opIndex = SetDeclarationTable(str);
	node.insert(std::make_pair("declaration", tinygltf::Value(opIndex)));
	CreateDeclarationExtensionInfo(pNode, str);

	tinygltf::Value::Object values;
	int numInConnect = 0;
	pNode->GetUserPropInt(GRAPH_NUMIN, numInConnect);
	for (int i = 0; i < numInConnect; i++) {
		tstring str = _T("InSocketParam") + to_tstring(i);
		tstring str2 = str + _T("_name");
		tstring str3 = str + _T("_type");
		tstring str4 = str + _T("_value");

		tstring socketName;
		INode *pParentNode = GetParentNode(nodeIdx, i, socketName);

		TSTR vl(_T(""));
		if (pNode->GetUserPropString(str4.c_str(), vl)) {
			//if (vl == _T("") && !pParentNode)continue;
		}
		TSTR nm;
		pNode->GetUserPropString(str2.c_str(), nm);
		std::string name = WStringToString(tstring(nm));

		tinygltf::Value::Object in_0;

		if(nm == _T("segment") && partA == _T("pointer"))
			PointerSegmentIDFound = TRUE;


		int type = 0;
		int typeIdx = 0;
		pNode->GetUserPropInt(str3.c_str(), type);
		if (type == dataFlow) {
			//int id = GetNodeIndex(pParentNode);
			//in_0.insert(std::make_pair("node", tinygltf::Value(id)));
			//flows.push_back(tinygltf::Value(in_0));
			//numFlowConnect++;
		}
		else {
			if (type == dataNodeIndex) {
				type = dataInt;
				ConvertNodeIndexToInt(vl);
			}
			typeIdx = SetTypeTable((socketDataType)type);
			if (pParentNode) {
				int id = GetNodeIndex(pParentNode);
				in_0.insert(std::make_pair("node", tinygltf::Value(id)));
				if (((socketDataType)type) == dataFlow) {
					std::string str("flow");
					in_0.insert(std::make_pair("socket", tinygltf::Value(str)));
				}
				else {
					std::string str = WStringToString(socketName);// ("value");
					in_0.insert(std::make_pair("socket", tinygltf::Value(str)));
				}
			}
			else {
				in_0.insert(std::make_pair("type", tinygltf::Value(typeIdx)));
				if (type == dataString) {
					std::string val = WStringToString(tstring(vl));
					in_0.insert(std::make_pair("value", tinygltf::Value(val)));
				}
				else {
					tinygltf::Value::Array val;
					SetValueArray(tstring(vl), (socketDataType)type, val);
					if(val.size())	in_0.insert(std::make_pair("value", tinygltf::Value(val)));
				}
			}

			//tinygltf::Value::Object in_x;
			//in_x.insert(std::make_pair(name, tinygltf::Value(in_0)));

			values.insert(std::make_pair(name, tinygltf::Value(in_0)));
			//values.push_back(tinygltf::Value(in_0));
		}
	}
	if (values.size()>0) {
		node.insert(std::make_pair("values", tinygltf::Value(values)));
	}

	//tinygltf::Value::Array flows;
	//int numFlowConnect = 0;
	//int numValueConnect = 0;
	int numOutConnect = 0;
	pNode->GetUserPropInt(GRAPH_NUMOUT, numOutConnect);
	tinygltf::Value::Object in_xf;
	for (int i = 0; i < numOutConnect; i++) {
		tstring str = _T("OutSocketParam") + to_tstring(i);
		tstring str2 = str + _T("_name");
		tstring str3 = str + _T("_type");
		tstring str4 = str + _T("_value");
		TSTR nm;
		pNode->GetUserPropString(str2.c_str(), nm);
		std::string name = WStringToString(tstring(nm));
		int type = 0;
		pNode->GetUserPropInt(str3.c_str(), type);

		int subNum = 0;
		tstring str5 = _T("NumOutSub") + to_tstring(i);
		pNode->GetUserPropInt(str5.c_str(), subNum);
		for (int j = 0; j < subNum; j++) {
			if (type == dataFlow) {
				tinygltf::Value::Object in_0;

				TSTR n;
				INode *pChildNode = GetChildNode(pNode, i, j, n);
				std::string nn = WStringToString(tstring(n));
				int id = GetNodeIndex(pChildNode);
				in_0.insert(std::make_pair("node", tinygltf::Value(id)));
				in_0.insert(std::make_pair("socket", tinygltf::Value(nn)));

				in_xf.insert(std::make_pair(name, tinygltf::Value(in_0)));

				//flows.push_back(tinygltf::Value(in_x));
				//numFlowConnect++;
			}
			else {
				//numValueConnect++;
			}
		}
	}
	node.insert(std::make_pair("flows", tinygltf::Value(in_xf)));

	tinygltf::Value::Object configurs;
	int numConfig = 0;
	pNode->GetUserPropInt(GRAPH_NUMCONFIG, numConfig);
	for (int i = 0; i < numConfig; i++) {
		tstring str = _T("ConfigParam") + to_tstring(i);
		tstring str2 = str + _T("_name");
		tstring str3 = str + _T("_type");
		tstring str4 = str + _T("_value");

		tinygltf::Value::Object in_0;

		int type = 0;
		pNode->GetUserPropInt(str3.c_str(), type);

		TSTR nm;
		pNode->GetUserPropString(str2.c_str(), nm);

		TSTR vl;
		pNode->GetUserPropString(str4.c_str(), vl);
		if (type == dataString) {
			std::string val = WStringToString(tstring(vl));
			if (nm == _T("pointer")) {
				if (PointerSegmentIDFound) {
					size_t pos = val.find("/0/");
					if(pos != std::string::npos) val.replace(pos+1, 1, "{segment}");
				}
				else {
					int index = -1;
					TSTR AnimHandle;
					pNode->GetUserPropString(_T("PointerHandle"), AnimHandle);
					Animatable *pAnim = GetAnimByUniqueID(std::stoul(tstring(AnimHandle)));
					if (pAnim) {
						if (pAnim->SuperClassID() == BASENODE_CLASS_ID) {
							index = findNodeIndex((INode*)pAnim);
						}
						else if (pAnim->SuperClassID() == MATERIAL_CLASS_ID)
							index = findMaterialIndex((Mtl*)pAnim);
					}
					if (index > 0) {
						tstring str(vl);
						ReplacePointerIndex(str, index);
						val = WStringToString(str);
					}
				}
			}
			tinygltf::Value::Array a;
			a.push_back(tinygltf::Value(val));
			in_0.insert(std::make_pair("value", tinygltf::Value(a)));
//			in_0.insert(std::make_pair("value", tinygltf::Value(val)));
		}
		else {
			int typeIdx;
			if(type < sizeof(socketDataType))
				typeIdx = SetTypeTable((socketDataType)type);
			//if(s_ExportMode==1)
			//	in_0.insert(std::make_pair("type", tinygltf::Value(typeIdx)));


			//
			if (partA == _T("variable")) {
				if (nm == _T("variables")) {
					tinygltf::Value::Array a;
					SetValueArray(tstring(vl), (socketDataType)dataInt, a);
					in_0.insert(std::make_pair("value", tinygltf::Value(a)));
				}
				else
				{
					int ii = findVariableIndex(vl, type);
					{
						tinygltf::Value::Array a;
						a.push_back(tinygltf::Value(ii));
						in_0.insert(std::make_pair("value", tinygltf::Value(a)));
					}
					//in_0.insert(std::make_pair("value", tinygltf::Value(ii)));
				}
			}
			else if (partA == _T("event")) {
				if (nm == _T("pointer")) {
					int ii = findEventIndex(vl, type);
					in_0.insert(std::make_pair("value", tinygltf::Value(ii)));
				}
				else if (nm == _T("event")) {
					int index = findEventIndex(vl, type);
					tinygltf::Value::Array val;
					SetValueArray(to_tstring(index), (socketDataType)dataInt, val);
					in_0.insert(std::make_pair("value", tinygltf::Value(val)));
				}
				else if (nm == _T("nodeIndex")) {
					type = dataInt;
					int index = ConvertNodeIndexToInt(vl);

					tinygltf::Value::Array val;
					SetValueArray(to_tstring(index), (socketDataType)type, val);
					in_0.insert(std::make_pair("value", tinygltf::Value(val)));
				}
				else {
					tinygltf::Value::Array val;
					SetValueArray(tstring(vl), (socketDataType)type, val);
					in_0.insert(std::make_pair("value", tinygltf::Value(val)));
				}
			}
			else if (partA == _T("pointer")) {
				if (nm == _T("type")) {
					int x = SetTypeTable((socketDataType)_wtol(vl.data()));
					vl = to_tstring(x).c_str();
					tinygltf::Value::Array val;
					SetValueArray(tstring(vl), (socketDataType)type, val);
					in_0.insert(std::make_pair("value", tinygltf::Value(val)));
				}
			}
			else {
				tinygltf::Value::Array val;
				SetValueArray(tstring(vl), (socketDataType)type, val);
				in_0.insert(std::make_pair("value", tinygltf::Value(val)));
			}
		}

		std::string name = WStringToString(tstring(nm));
		//tinygltf::Value::Object in_x;
		//in_x.insert(std::make_pair(name, tinygltf::Value(in_0)));
		configurs.insert(std::make_pair(name, in_0));

		//node.insert(std::make_pair("configuration", tinygltf::Value(in_x)));
	}

	if (numConfig > 0)
		node.insert(std::make_pair("configuration", tinygltf::Value(configurs)));
	//if (numFlowConnect)
	//	node.insert(std::make_pair("flows", tinygltf::Value(flows)));
	//if(values.size()>0)
	//	node.insert(std::make_pair("values", tinygltf::Value(values)));

	nodes.push_back(tinygltf::Value(node));
}

//======================================================================
//======================================================================
BOOL CheckIfExtension(const std::string &op, nodeInfo &info)
{
	for (auto p : extensionMapTable) {
		if (p.first == op) {
			info = p.second;
			return TRUE;
		}
	}
	return FALSE;
}

//======================================================================
// Declarationsノードの生成
//======================================================================
void glTFExporter_Core::SetDeclarationsTypesTable(tinygltf::Value::Array& declarations, std::vector<std::string>&interactiveExtensionList)
{
	for (auto p : declarationTable) {
		tinygltf::Value::Object o;
		o.insert(std::make_pair("op", tinygltf::Value(p)));
		nodeInfo info;
		if (CheckIfExtension(p, info)) {
			if(std::find(interactiveExtensionList.begin(), interactiveExtensionList.end(), info.extensionName)== interactiveExtensionList.end())
				interactiveExtensionList.push_back(info.extensionName);
			o.insert(std::make_pair("extension", tinygltf::Value(info.extensionName)));
			if (info.OutSocket.size() > 0) {
				tinygltf::Value::Object oo;
				for (auto out : info.OutSocket) {
					tinygltf::Value::Object o1;
					o1.insert(std::make_pair("type", tinygltf::Value(out.type)));
					oo.insert(std::make_pair(out.name.c_str(), tinygltf::Value(o1)));
				}
				o.insert(std::make_pair("outputValueSockets", tinygltf::Value(oo)));
			}
			if (info.InSocket.size() > 0) {
				tinygltf::Value::Object oo;
				for (auto in : info.InSocket) {
					tinygltf::Value::Object o1;
					o1.insert(std::make_pair("type", tinygltf::Value(in.type)));
					oo.insert(std::make_pair(in.name.c_str(), tinygltf::Value(o1)));
				}
				o.insert(std::make_pair("inputValueSockets", tinygltf::Value(oo)));
			}
		}
		declarations.push_back(tinygltf::Value(o));
	}

}

//======================================================================
// Variableノードの生成
//======================================================================
void glTFExporter_Core::SetVariableTable(INode *pNode, tinygltf::Value::Array& variables)
{
	//tinygltf::Value::Array variable;

	TSTR name;
	TSTR value;
	TSTR desc;
	pNode->GetUserPropString(_T("HSVariableID"), name);
	pNode->GetUserPropString(_T("HSVariableValue"), value);
	pNode->GetUserPropString(_T("HSVariableDesc"), desc);
	int type = 0;
	pNode->GetUserPropInt(_T("HSVariableType"), type);

	if (type == dataNodeIndex) {
		type = dataInt;
		ConvertNodeIndexToInt(value);
	}

	int typeIdx = SetTypeTable((socketDataType)type);

	tinygltf::Value::Object in_0;
	std::string n = WStringToString(tstring(name));
	in_0.insert(std::make_pair("name", tinygltf::Value(n)));
	in_0.insert(std::make_pair("type", tinygltf::Value(typeIdx)));
	if (value != _T("")) {
		tinygltf::Value::Array val;
		SetValueArray(tstring(value), (socketDataType)type, val);
		in_0.insert(std::make_pair("value", tinygltf::Value(val)));
	}
	variables.push_back(tinygltf::Value(in_0));

	//variables.push_back(tinygltf::Value(variable));
}

//======================================================================
// Eventノードの生成
//======================================================================
void glTFExporter_Core::SetEventTable(INode *pNode, tinygltf::Value::Array& events)
{
	TSTR name;
	TSTR value;
	TSTR desc;
	pNode->GetUserPropString(_T("HSEventID"), name);

	tinygltf::Value::Object eventItem;
	std::string n = WStringToString(tstring(name));
	eventItem.insert(std::make_pair("id", tinygltf::Value(n)));

	tinygltf::Value::Object values;

	int numValue;
	pNode->GetUserPropInt(_T("HSValueCount"), numValue);
	for (int i = 0; i < numValue; i++) {
		tstring str = _T("HSEventValue") + to_tstring(i);
		tstring str2 = str + _T("_name");
		tstring str3 = str + _T("_type");
		tstring str4 = str + _T("_value");
		//tstring str5 = str + _T("_description");

		TSTR t1, t2;
		int type;
		pNode->GetUserPropString(str2.c_str(), t1);
		pNode->GetUserPropInt(str3.c_str(), type);
		pNode->GetUserPropString(str4.c_str(), t2);

		if (type == dataNodeIndex) {
			type = dataInt;
			ConvertNodeIndexToInt(t2);
		}

		tinygltf::Value::Object in_0;
		//in_0.insert(std::make_pair("id", tinygltf::Value(v)));
		int typeIdx = SetTypeTable((socketDataType)type);
		in_0.insert(std::make_pair("type", tinygltf::Value(typeIdx)));
		if (t2.length() > 0) {
			tinygltf::Value::Array val;
			SetValueArray(tstring(t2), (socketDataType)type, val);
			in_0.insert(std::make_pair("value", tinygltf::Value(val)));
		}

		std::string v = WStringToString(tstring(t1));
		values.insert(std::make_pair(v, in_0));
	}

	if (numValue>0) {
		//values.push_back(tinygltf::Value(in_x));
		eventItem.insert(std::make_pair("values", values));
	}

	events.push_back(tinygltf::Value(eventItem));
}

typedef std::pair<INode*, int> Pair;
bool compareBySecond(const Pair &a, const Pair &b) {
	return a.second < b.second;
}

//======================================================================
//
//======================================================================
void SetOrderMap(std::vector<Pair> &orderMap)
{
	for (int i = 0; i < GetCOREInterface()->GetSelNodeCount(); i++) {
		INode *pNode = GetCOREInterface()->GetSelNode(i);
		int order;
		pNode->GetUserPropInt(_T("GraphNodeOrder"), order);
		orderMap.push_back(Pair(pNode, order));
	}

	std::sort(orderMap.begin(), orderMap.end(), compareBySecond);

}

//======================================================================
//
//======================================================================
void glTFExporter_Core::GetInteractivityNodeList(std::vector<std::string>& interactiveExtensionList)
{
	//if (!m_pInteractiveGraphLayer) return;
	interactiveExtensionList.clear();

	tinygltf::Value::Array graph;

	for(auto pLayer: m_InteractiveLayerTable)
	{
		declarationTable.clear();
		typeTable.clear();
		InitTypeTable();
		nodeNodeTbl.clear();
		eventNodeTbl.clear();
		variableNodeTbl.clear();
		std::map<int, INode*> workValTbl;
		std::map<int, INode*> workEventTbl;

		GetCOREInterface()->ClearNodeSelection();
		pLayer->SelectObjects();

		//int s = m_NodeMap.size();

		std::vector<Pair> orderMap;
		SetOrderMap(orderMap);

		for (auto p: orderMap) {
			INode *pNode = p.first;
			tstring name = pNode->GetName();
			auto pos = name.rfind('_');
			int index = std::stoi(name.substr(pos + 1));

			int itemType = -1;
			pNode->GetUserPropInt(ITEM_TYPE, itemType);
			if (itemType == ItemType::variable) {
				workValTbl[index] = pNode;
			}
			else if (itemType == ItemType::event) {
				workEventTbl[index] = pNode;
			}
			else if (itemType == ItemType::node)
				nodeNodeTbl.push_back(pNode);
		}

		tinygltf::Value::Array events;
		for (auto p : workEventTbl) {
			eventNodeTbl.push_back(p.second);
			SetEventTable(p.second, events);
		}

		tinygltf::Value::Array variables;
		for (auto p : workValTbl) {
			variableNodeTbl.push_back(p.second);
			SetVariableTable(p.second, variables);
		}

		tinygltf::Value::Array nodes;
		for (auto p : nodeNodeTbl) {
			SetNodeTable(p, nodes);
		}

		tinygltf::Value::Array types;
		SetTypesTable(types);

		tinygltf::Value::Array declarations;
		SetDeclarationsTypesTable(declarations, interactiveExtensionList);


		tinygltf::Value::Object tbl;
		tbl.insert(std::make_pair("events", tinygltf::Value(events)));
		tbl.insert(std::make_pair("variables", tinygltf::Value(variables)));
		tbl.insert(std::make_pair("nodes", tinygltf::Value(nodes)));
		tbl.insert(std::make_pair("types", types));
		tbl.insert(std::make_pair("declarations", declarations));

		if (tbl.size()) {
			graph.push_back(tinygltf::Value(tbl));
		}
	}

	if (graph.size()) {
		tinygltf::Value::Object camera;
		SetActiveCameraInfo(camera);

		tinygltf::Value::Object graphs;
		graphs.insert(std::make_pair("graphs", graph));
		graphs.insert(std::make_pair("graph", m_InteractiveGraphID));
		graphs.insert(std::make_pair("activeCamera", camera));
		m_model.extensions.insert(std::make_pair("KHR_interactivity", graphs));
		m_Interactivity_Used = TRUE;
	}
}


//======================================================================
// DWORDで表記されるnodeIndexよりノードドリストインデックス)int変換
// val：_nodeIndexを渡してintに変える
//======================================================================
int glTFExporter_Core::ConvertNodeIndexToInt(TSTR& val)
{
	int index = 0;
	if(!isNumber(tstring(val))) return index;

	Animatable* pAnim = GetAnimByUniqueID(std::stoul(tstring(val)));
	if (pAnim) {
		if (pAnim->SuperClassID() == BASENODE_CLASS_ID) {
			index = findNodeIndex((INode*)pAnim);
		}
		else if (pAnim->SuperClassID() == MATERIAL_CLASS_ID)
			index = findMaterialIndex((Mtl*)pAnim);

		val = to_tstring(index).c_str();
	}

	return index;
}

//=================================================================================
//=================================================================================
void SetActiveCameraInfo(tinygltf::Value::Object& camera)
{
	ViewExp& v = GetCOREInterface()->GetActiveViewExp();
	Matrix3 tm;
	v.GetAffineTM(tm);
	tm = Inverse(tm) * YupTM;
	Point3 viewPt = tm.GetRow(3);
	Point3 viewDir = tm.GetRow(2);
	Quat q;
	q.SetEuler(viewDir.x, viewDir.y, viewDir.z);

	AffineParts parts;
	decomp_affine(tm, &parts);

	tinygltf::Value::Array pos;
	pos.push_back(tinygltf::Value(viewPt.x * s_scale));
	pos.push_back(tinygltf::Value(viewPt.y * s_scale));
	pos.push_back(tinygltf::Value(viewPt.z * s_scale));
	camera.insert(std::make_pair("position", tinygltf::Value(pos)));

	tinygltf::Value::Array rot;
	rot.push_back(tinygltf::Value(parts.q.x));
	rot.push_back(tinygltf::Value(parts.q.y));
	rot.push_back(tinygltf::Value(parts.q.z));
	rot.push_back(tinygltf::Value(-parts.q.w));
	camera.insert(std::make_pair("rotation", tinygltf::Value(rot)));
}
