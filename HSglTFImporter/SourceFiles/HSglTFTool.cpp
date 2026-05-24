/**********************************************************************
 *<
	FILE: Test.cpp

	DESCRIPTION:	Appwizard generated plugin

	CREATED BY: 

	HISTORY: 

 *>	Copyright (c) 2000, All Rights Reserved.
 **********************************************************************/

#include "HSglTFImporter.h"
#include <utilapi.h>
#include <ilayer.h>

#define KHR_MATERIALS_ANISOTROPY             (1<<0)
#define KHR_MATERIALS_CLEARCOAT              (1<<1)
#define KHR_MATERIALS_DISPERSION             (1<<2)
#define KHR_MATERIALS_EMISSIVE_STRENGTH      (1<<3)
#define KHR_MATERIALS_IOR                    (1<<4)
#define KHR_MATERIALS_IRIDESCENCE            (1<<5)
#define KHR_MATERIALS_SHEEN                  (1<<6)
#define KHR_MATERIALS_SPECULAR               (1<<7)
#define KHR_MATERIALS_TRANSMISSION           (1<<8)
#define KHR_MATERIALS_VOLUME                 (1<<9)
#define KHR_MATERIALS_DIFFUSE_TRANSMISSION   (1<<10)
#define KHR_MATERIALS_SUBSURFACE             (1<<11)
#define KHR_MATERIALS_UNLIT		             (1<<12)

#define TEST_CLASS_ID		Class_ID(0xa91564c, 0x29f7754d)
#define HSGLTFTOOL_INTERFACE_ID		Interface_ID(0x5e9360fd, 0x5d74691e)

//#define THECLASSNAME	"HSglTFTool"
//#define THIS_VERASION	100


static tstring BaseLayerName = _T("HSInteractiveGraphLayer");

static void SetAttributes(Mtl* pSmat, ULONG flag = 0xffffffff, BOOL enableFlag=FALSE);


//===================================================
// Define Plugin class
//===================================================
class HSglTFTool : public UtilityObj {
public:
	//Constructor/Destructor
	HSglTFTool();
	~HSglTFTool();

	void BeginEditParams(Interface *ip,IUtil *iu);
	void EndEditParams(Interface *ip,IUtil *iu);

	void Init(HWND hWnd);
	void Destroy(HWND hWnd);
	void DeleteThis(void);

	void DoTest(HWND hWnd);
	void SetNodeAttr(HWND hWnd);
	void SetNodeExtentionValue(HWND hWnd);
	//void RemoveNodeAttr(HWND hWnd);
	void NodeIndexDlg(HWND hWnd);
	void RemoveAttr(HWND hWnd);
	FPInterfaceDesc* GetDesc();

	HWND			hPanel;
	IUtil			*iu;
	Interface		*ip;
};
static HSglTFTool theHSglTFToolt;

//===================================================
// クラス記述子
//===================================================
class HSglTFToolClassDesc:public ClassDesc2 {
public:
	int 			IsPublic() { return TRUE; }
	void *			Create(BOOL loading = FALSE) { return &theHSglTFToolt; }
	const TCHAR *	ClassName() { return GetString(IDS_CLASS_NAME2); }
	SClass_ID		SuperClassID() { return UTILITY_CLASS_ID; }
	Class_ID		ClassID() { return TEST_CLASS_ID; }
	const TCHAR* 	Category() { return GetString(IDS_CATEGORY); }

	const TCHAR*	InternalName() { return _T("HSglTF Tool"); }
	HINSTANCE		HInstance() { return hInstance; }
#if MAX_RELEASE>=24000
	const wchar_t* ClassDesc::NonLocalizedClassName(void) { return GetString(IDS_CLASS_NAME); }
#endif
};
ClassDesc2* GetHSglTFToolDesc()
{
	static HSglTFToolClassDesc theHSglTFToolDesc;
	return &theHSglTFToolDesc;
}


//======================================================================
//======================================================================
enum {
	fnIdAttacheExtensionAttrFn,
};
class HSglTFToolActions : public FPStaticInterface {
public:
	virtual BOOL AttacheExtensionAttrFn(ReferenceTarget*, TSTR) = 0;
};
class HSglTFToolActionsIMP : public HSglTFToolActions {
public:
	DECLARE_DESCRIPTOR(HSglTFToolActionsIMP)

	BEGIN_FUNCTION_MAP
	FN_2(fnIdAttacheExtensionAttrFn, TYPE_BOOL, AttacheExtensionAttrFn, TYPE_REFTARG, TYPE_STRING);
	END_FUNCTION_MAP

	BOOL AttacheExtensionAttrFn(ReferenceTarget* pRef, TSTR str) {
		if (str== _T("KHR_materials_anisotropy")) {
			SetAttributes((Mtl*)pRef, KHR_MATERIALS_ANISOTROPY, TRUE);
		}
		if (str == _T("KHR_materials_clearcoat")) {
			SetAttributes((Mtl*)pRef, KHR_MATERIALS_CLEARCOAT, TRUE);
		}
		if (str == _T("KHR_materials_dispersion")) {
			SetAttributes((Mtl*)pRef, KHR_MATERIALS_DISPERSION, TRUE);
		}
		if (str == _T("KHR_materials_emissive_strength")) {
			SetAttributes((Mtl*)pRef, KHR_MATERIALS_EMISSIVE_STRENGTH, TRUE);
		}
		if (str == _T("KHR_materials_ior")) {
			SetAttributes((Mtl*)pRef, KHR_MATERIALS_IOR, TRUE);
		}
		if (str == _T("KHR_materials_iridescence")) {
			SetAttributes((Mtl*)pRef, KHR_MATERIALS_IRIDESCENCE, TRUE);
		}
		if (str == _T("KHR_materials_sheen")) {
			SetAttributes((Mtl*)pRef, KHR_MATERIALS_SHEEN, TRUE);
		}
		if (str == _T("KHR_materials_specular")) {
			SetAttributes((Mtl*)pRef, KHR_MATERIALS_SPECULAR, TRUE);
		}
		if (str == _T("KHR_materials_transmission")) {
			SetAttributes((Mtl*)pRef, KHR_MATERIALS_TRANSMISSION, TRUE);
		}
		if (str == _T("KHR_materials_volume")) {
			SetAttributes((Mtl*)pRef, KHR_MATERIALS_VOLUME, TRUE);
		}
		if (str == _T("KHR_materials_diffuse_transmission")) {
			SetAttributes((Mtl*)pRef, KHR_MATERIALS_DIFFUSE_TRANSMISSION, TRUE);
		}
		if (str == _T("KHR_materials_subsurface")) {
			SetAttributes((Mtl*)pRef, KHR_MATERIALS_SUBSURFACE, TRUE);
		}
		return TRUE;
	}
};
static HSglTFToolActionsIMP HSglTFToolActionsFP(HSGLTFTOOL_INTERFACE_ID, _T("HSglTFTools"), 0, GetHSglTFToolDesc(), 0,
	fnIdAttacheExtensionAttrFn, _T("AttacheExtensionAttr"), 0, TYPE_BOOL, 0, 2, _T("target"), 0, TYPE_REFTARG, _T("extension"), 0, TYPE_STRING,
	p_end
);
FPInterfaceDesc* HSglTFTool::GetDesc() { return &HSglTFToolActionsFP; }


//===================================================
// Tool Panel callback
//===================================================
static BOOL CALLBACK MyDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg) {
	case WM_INITDIALOG:
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDC_ATCATTR_BTN:
			theHSglTFToolt.DoTest(hWnd);
			break;

		case IDC_NODEINDEX_BTN:
			theHSglTFToolt.NodeIndexDlg(hWnd);
			break;

		case IDC_REMOVE_SELECTED_BTN:
			theHSglTFToolt.RemoveAttr(hWnd);
			break;
		}
		break;

	default:
		return FALSE;
	}
	return TRUE;
}


//===================================================
//===================================================
//--- HSglTFTool -------------------------------------------------------
HSglTFTool::HSglTFTool()
{
	iu = NULL;
	ip = NULL;	
	hPanel = NULL;
}

HSglTFTool::~HSglTFTool()
{
}

void HSglTFTool::BeginEditParams(Interface *ip,IUtil *iu)
{
	this->iu = iu;
	this->ip = ip;

	hPanel = ip->AddRollupPage(hInstance, MAKEINTRESOURCE(IDD_TOOL_DLG),
		(DLGPROC)MyDlgProc, _T("HS glTF Tool"), 0);

	//DoTest(ip->GetMAXHWnd());
}
	
void HSglTFTool::EndEditParams(Interface *ip,IUtil *iu)
{
	this->iu = NULL;
	this->ip = NULL;
	ip->DeleteRollupPage(hPanel);
	hPanel = NULL;
}

void HSglTFTool::Init(HWND hWnd)
{
}

void HSglTFTool::Destroy(HWND hWnd)
{
}

void HSglTFTool::DeleteThis(void)
{
}

//===================================================
//===================================================
void SetAttributes(Mtl* pSmat, ULONG flag, BOOL enableFlag)
{
	if (!pSmat) return;

	glTFImporter_Core app;

	cgltf_texture_view texView;
	texView.texture = NULL;

	cgltf_clearcoat clearcoat;
	clearcoat.clearcoat_factor = 0.0f;
	clearcoat.clearcoat_roughness_factor = 0.0f;
	clearcoat.clearcoat_texture = texView;
	clearcoat.clearcoat_roughness_texture = texView;
	clearcoat.clearcoat_normal_texture = texView;

	cgltf_ior ior;
	ior.ior = 1.5f;

	cgltf_specular specular;
	specular.specular_factor = 1.0f;
	specular.specular_texture = texView;
	specular.specular_color_texture = texView;

	cgltf_sheen sheen;
	sheen.sheen_roughness_factor = 0.0f;
	sheen.sheen_color_texture = texView;
	sheen.sheen_roughness_texture = texView;

	cgltf_transmission transmission;
	transmission.transmission_factor = 0.0f;
	transmission.transmission_texture = texView;

	cgltf_volume volume;
	volume.attenuation_distance = 10000.0f;
	volume.thickness_factor = 0.0f;
	volume.thickness_texture = texView;

	cgltf_emissive_strength emissive_strength;
	emissive_strength.emissive_strength = 1.0f;

	cgltf_iridescence iridescence;
	iridescence.iridescence_factor = 0.0f;
	iridescence.iridescence_ior = 1.3f;
	iridescence.iridescence_thickness_max = 400.0f;
	iridescence.iridescence_thickness_min = 100.0f;
	iridescence.iridescence_texture = texView;
	iridescence.iridescence_thickness_texture = texView;

	cgltf_dispersion dispersion;
	dispersion.dispersion = 0.0f;

	cgltf_anisotropy anisotropy;
	anisotropy.anisotropy_rotation = 0.0f;
	anisotropy.anisotropy_strength = 0.0f;
	anisotropy.anisotropy_texture = texView;

	cgltf_diffuse_transmission diffTrans;
	*(diffTrans.diffuseTransmissionColorFactor+0) = 0.0f;;
	*(diffTrans.diffuseTransmissionColorFactor + 1) = 0.0f;;
	*(diffTrans.diffuseTransmissionColorFactor + 2) = 0.0f;;
	diffTrans.diffuseTransmissionFactor = 0.0f;
	diffTrans.diffuseTransmissionColorTexture = texView;
	diffTrans.diffuseTransmissionTexture = texView;

	if (pSmat->ClassID() == StandardMtlID) {
		app.AttacheAlphaModeCustAttr(pSmat, 0);
	}
	else if (pSmat->ClassID() == PBRMetalMtlID) {
		app.AttacheAlphaModeCustAttr(pSmat, 0);
		if (flag & KHR_MATERIALS_IOR)					app.CreateIORAttr(pSmat, &ior, enableFlag);
		if (flag & KHR_MATERIALS_DIFFUSE_TRANSMISSION)	app.CreateTransmissionAttr(pSmat, &transmission, enableFlag);
		if (flag & KHR_MATERIALS_VOLUME)				app.CreateVolumeAttr(pSmat, &volume, enableFlag);
		if (flag & KHR_MATERIALS_IRIDESCENCE)			app.CreateIridescenceAttr(pSmat, &iridescence, enableFlag);
		if (flag & KHR_MATERIALS_SHEEN)					app.CreateSheenAttr(pSmat, &sheen, enableFlag);
		if (flag & KHR_MATERIALS_CLEARCOAT)				app.CreateClearcoatAttr(pSmat, &clearcoat, enableFlag);
		if (flag & KHR_MATERIALS_UNLIT)					app.CreateUnlitAttr(pSmat, enableFlag);
		if(flag & KHR_MATERIALS_EMISSIVE_STRENGTH)		app.CreateEmissiveStrengthAttr(pSmat, &emissive_strength, enableFlag);
		if (flag & KHR_MATERIALS_SPECULAR)				app.CreateSpecularAttr(pSmat, &specular, enableFlag);
		if (flag & KHR_MATERIALS_DISPERSION)			app.CreateDispersionAttr(pSmat, &dispersion, enableFlag);
		if(flag & KHR_MATERIALS_ANISOTROPY)				app.CreateAnisotropyAttr(pSmat, &anisotropy, enableFlag);
		if (flag & KHR_MATERIALS_DIFFUSE_TRANSMISSION) app.CreateDiffuseTransmissionAttr(pSmat, &diffTrans, enableFlag);
	}
	else if (pSmat->ClassID() == PBRSpecGlossMtlID) {
	}
	else if (pSmat->ClassID() == PHYSICALMATERIAL_CLASS_ID) {
		app.AttacheAlphaModeCustAttr(pSmat, 0);
		if(flag & KHR_MATERIALS_VOLUME)					app.CreateVolumeAttr(pSmat, &volume, enableFlag);
		if (flag & KHR_MATERIALS_UNLIT)					app.CreateUnlitAttr(pSmat, enableFlag);
		if (flag & KHR_MATERIALS_SPECULAR)				app.CreateSpecularAttr(pSmat, &specular, enableFlag);
		if (flag & KHR_MATERIALS_DIFFUSE_TRANSMISSION)	app.CreateDiffuseTransmissionAttr(pSmat, &diffTrans, enableFlag);
	}
	else  if (pSmat->ClassID() == Class_ID(DMTL_CLASS_ID, 0)) {
	}
	else  if (pSmat->ClassID() == glTFMaterialID) {
		if (flag & KHR_MATERIALS_IRIDESCENCE)			app.CreateIridescenceAttr(pSmat, &iridescence, enableFlag);
		if (flag & KHR_MATERIALS_EMISSIVE_STRENGTH)		app.CreateEmissiveStrengthAttr(pSmat, &emissive_strength, enableFlag);
		if (flag & KHR_MATERIALS_DISPERSION)			app.CreateDispersionAttr(pSmat, &dispersion, enableFlag);
		if (flag & KHR_MATERIALS_ANISOTROPY)			app.CreateAnisotropyAttr(pSmat, &anisotropy, enableFlag);
		if (flag & KHR_MATERIALS_DIFFUSE_TRANSMISSION)	app.CreateDiffuseTransmissionAttr(pSmat, &diffTrans, enableFlag);
	}
	else  if (pSmat->ClassID() == USDMaterialID) {
	}
	else  if (pSmat->ClassID() == Arnold_StandardSufaceID) {
		app.AttacheAlphaModeCustAttr(pSmat, 0);
		if (flag & KHR_MATERIALS_DIFFUSE_TRANSMISSION) app.CreateDiffuseTransmissionAttr(pSmat, &diffTrans, enableFlag);
	}
	else  if (pSmat->ClassID() == VRayMaterialID) {
	}
	else  if (pSmat->ClassID() == CoronaMaterialID) {
	}

	//---------------------------------------------------------------
	for (int i = 0; i < pSmat->NumSubTexmaps(); i++) {
		Texmap* pTex = pSmat->GetSubTexmap(i);
		if (!pTex) continue;
		if (pTex->ClassID() == bmptexClassID) {
			app.CreateWebpEncodingAttr(pTex, _T(""), FALSE);
			app.CreateKTX2EncodingAttr(pTex, _T(""), FALSE);
		}
		else if (pTex->ClassID() == VRayBitmapID) {
			app.CreateWebpEncodingAttr(pTex, _T(""), FALSE);
			app.CreateKTX2EncodingAttr(pTex, _T(""), FALSE);
		}
		else if (pTex->ClassID() == CoronaBitmapID) {
			app.CreateWebpEncodingAttr(pTex, _T(""), FALSE);
			app.CreateKTX2EncodingAttr(pTex, _T(""), FALSE);
		}
		else if (GetOSLMapType(pTex) == OSL_UberBitmap) {
			app.CreateWebpEncodingAttr(pTex, _T(""), FALSE);
			app.CreateKTX2EncodingAttr(pTex, _T(""), FALSE);
		}
	}

}

//===================================================
//===================================================
void HSglTFTool::DoTest(HWND hWnd)
{

	MtlBaseLib* pLib = ip->GetSceneMtls();
	for (int i = 0; i < pLib->Count(); i++) {
		MtlBase* pMtl = *pLib->Addr(i);
		SetAttributes((Mtl*)pMtl);
	}
}

//===================================================
//===================================================
void RemoveNodeAttributes(INode *pNode,  const tstring &ExtentionName)
{
	if (!pNode) return;
	ICustAttribContainer* pContainer = pNode->GetObjectRef()->GetCustAttribContainer();
	if (!pContainer) return;

	glTFImporter_Core app;
	IParamBlock2 *pBlock = NULL;
	int idx = app.GetCustAttrPBlock(pNode->GetObjectRef(), ExtentionName, pBlock);
	if (idx >= 0) {
		pContainer->RemoveCustAttrib(idx);
	}

/*
	int idx = app.GetCustAttrPBlock(pNode->GetObjectRef(), tstring(_T("Selectability")), pBlock);
	if (idx>=0) {
		pContainer->RemoveCustAttrib(idx);
	}
	idx = app.GetCustAttrPBlock(pNode->GetObjectRef(), tstring(_T("Hoverability")), pBlock);
	if (idx >= 0) {
		pContainer->RemoveCustAttrib(idx);
	}
	idx = app.GetCustAttrPBlock(pNode->GetObjectRef(), tstring(_T("Visibility")), pBlock);
	if (idx >= 0) {
		pContainer->RemoveCustAttrib(idx);
	}
*/
}

/*
//===================================================
// Remove Extensions from selected Nodes
//===================================================
void HSglTFTool::RemoveNodeAttr(HWND hWnd)
{
	//GetCOREInterface()->ClearNodeSelection();
	//if (!GetCOREInterface()->DoHitByNameDialog()) return;

	//BOOL sel = IsDlgButtonChecked(hWnd, IDC_SEL_CHK);
	//BOOL hov = IsDlgButtonChecked(hWnd, IDC_HOV_CHK);
	//BOOL vis = FALSE;
	for (int i = -0; i < GetCOREInterface()->GetSelNodeCount(); i++) {
		INode* pNode = GetCOREInterface()->GetSelNode(i);
		RemoveNodeAttributes(pNode);
	}
}
*/
//===================================================
//===================================================
void HSglTFTool::RemoveAttr(HWND hWnd)
{
	TrackViewPick tvp;
	tvp.anim = tvp.client = NULL;
	BOOL ok = GetCOREInterface()->TrackViewPickDlg(hWnd, &tvp);
	if (!ok) return;
	if (!tvp.anim || !tvp.client) return;

	//if (tvp.anim->SuperClassID() == BASENODE_CLASS_ID) {
	//	RemoveNodeAttributes((INode*)tvp.anim);
	//	return;
	//}

	if (tvp.anim->SuperClassID() == MATERIAL_CLASS_ID ||
		tvp.anim->SuperClassID() == TEXMAP_CLASS_ID) {

		ReferenceTarget* pRef = tvp.anim;
		ICustAttribContainer* pContainer = pRef->GetCustAttribContainer();
		if (!pContainer) return;
		int cnt = pContainer->GetNumCustAttribs();
		for (int i = cnt - 1; i >= 0; i--) {
			CustAttrib* pAttr = pContainer->GetCustAttrib(i);
			if (GetCustomAttrName(pAttr) != TSTR(_T("Custom_Attributes"))) continue;

			pContainer->RemoveCustAttrib(i);
		}
	}

}

//***************************************************
//
//
//
//***************************************************

//===================================================
// Struct definitions
//===================================================
// Paramerts for sorting
struct SortParam {
	HWND hListView;
	int  column;
	bool ascending;
};

struct nodePropStr {
	int Sel;
	int Hov;
	int Vis;
	//DWORD index;

	nodePropStr(): Sel(-1), Hov(-1),Vis(-1)  {}
};

static std::map<INode*, nodePropStr> s_NodeIndexMap;

//===================================================
// Local functions
//===================================================
static void NodeIndexDlgInit(HWND hWnd);
//static void SelectNodeIndexItem(HWND hWnd);
//static void SetSelectability(HWND hWnd);
//static void FindDupIndex(HWND hWnd);


//======================================================================
//======================================================================
int CALLBACK CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParamSort) {
	SortParam* pSort = (SortParam*)lParamSort;

	TCHAR buf1[MAX_PATH], buf2[MAX_PATH];
	ListView_GetItemText(pSort->hListView, (int)lParam1, pSort->column, buf1, MAX_PATH);
	ListView_GetItemText(pSort->hListView, (int)lParam2, pSort->column, buf2, MAX_PATH);

	int cmp = lstrcmp(buf1, buf2);

	return pSort->ascending ? cmp : -cmp;
}

//=============================================================================
// Unique Index setting dialog
//=============================================================================
static LRESULT CALLBACK NodeIndexDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	//int typeIdx;
	tstring valStr;

	switch (msg) {
	case WM_INITDIALOG:
		NodeIndexDlgInit(hWnd);
		return (INT_PTR)TRUE;

	case WM_DESTROY:
		EndDialog(hWnd, 1);
		break;

	case WM_COMMAND:
		switch (wParam) {
		case IDC_NODEATTR_BTN:
			theHSglTFToolt.SetNodeAttr(hWnd);
			break;
		case IDC_SETVAL_BUTTON:
			theHSglTFToolt.SetNodeExtentionValue(hWnd);
			break;

		case IDOK:
		case IDCANCEL:
			EndDialog(hWnd, 1);
			return TRUE;
		}
		break;
	case WM_NOTIFY:
		if (LOWORD(wParam) == IDC_NODEINDEX_LIST) {
			switch ((UINT)((LPNMHDR)lParam)->code) {
			case NM_CLICK:
				//SelectNodeIndexItem(hWnd);
				break;
			case LVN_COLUMNCLICK:
				{
					HWND hListView = GetDlgItem(hWnd, IDC_NODEINDEX_LIST);
					NMLISTVIEW* pnmv = (NMLISTVIEW*)lParam;

					static bool ascending = true;  // クリックごとに昇順/降順切替
					SortParam param = { hListView, pnmv->iSubItem, ascending };

					ListView_SortItems(hListView, CompareFunc, (LPARAM)&param);

					ascending = !ascending; // 次回は逆順
				}
				break;
			}
		}
	}

	return FALSE;
}

//=============================================================================
//=============================================================================
void SetNodeIndexListRec(INode* pNode)
{
	if (!pNode) return;

	ILayer* pLayer = (ILayer*)pNode->GetReference(NODE_LAYER_REF);
	if (pLayer) {
		tstring name = tstring(pLayer->GetName().data());
		if (name.find(BaseLayerName) != std::string::npos) return;
	}

	nodePropStr prop;

	//DWORD id = 0;
	//if (GetUniqueID(pNode, id)) {
	//	prop.index = id;
	//}

	if (pNode->GetObjectRef()) {
		glTFImporter_Core app;
		IParamBlock2* pBlock = NULL;
		app.GetCustAttrPBlock(pNode->GetObjectRef(), tstring(_T("Selectability")), pBlock);
		if (pBlock) {
			pBlock->GetValueByName(_T("Selectable"), 0, prop.Sel, FOREVER, 0);
		}
		app.GetCustAttrPBlock(pNode->GetObjectRef(), tstring(_T("Hoverability")), pBlock);
		if (pBlock) {
			pBlock->GetValueByName(_T("Hoverable"), 0, prop.Hov, FOREVER, 0);
		}
		app.GetCustAttrPBlock(pNode->GetObjectRef(), tstring(_T("Visibility")), pBlock);
		if (pBlock) {
			pBlock->GetValueByName(_T("visible"), 0, prop.Vis, FOREVER, 0);
		}

		s_NodeIndexMap[pNode] = prop;
	}

	for (int i = 0; i < pNode->NumChildren();i++) {
		SetNodeIndexListRec(pNode->GetChildNode(i));
	}
}
//=============================================================================
// Launch Unique Index Dialog
//=============================================================================
void HSglTFTool::NodeIndexDlg(HWND hWnd)
{
	INT_PTR ret = ::DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_NODEEXTENSION_DLG), hWnd, NodeIndexDlgProc, 0);
}

//=============================================================================
//=============================================================================
void SetList(HWND hListView)
{
	int index = 0;
	for (auto p : s_NodeIndexMap) {
		LVITEM item;
		item.iItem = index++;
		item.mask = LVIF_TEXT | LVIF_PARAM;
		item.cchTextMax = MAX_PATH;

		item.iSubItem = 0;
		item.pszText = (LPWSTR)p.first->GetName();
		item.lParam = (LPARAM)p.first;
		ListView_InsertItem(hListView, &item);

		item.iSubItem = 1;
		item.mask = LVIF_TEXT;
		tstring str = p.second.Sel > 0 ? _T("On") : (p.second.Sel == 0 ? _T("Off") : _T(""));
		item.pszText = (LPWSTR)str.c_str();
		ListView_SetItem(hListView, &item);

		item.iSubItem = 2;
		item.mask = LVIF_TEXT;
		str = p.second.Hov > 0 ? _T("On") : (p.second.Hov == 0 ? _T("Off") : _T(""));
		item.pszText = (LPWSTR)str.c_str();
		ListView_SetItem(hListView, &item);

		item.iSubItem = 3;
		item.mask = LVIF_TEXT;
		str = p.second.Vis > 0 ? _T("On") : (p.second.Vis == 0 ? _T("Off") : _T(""));
		item.pszText = (LPWSTR)str.c_str();
		ListView_SetItem(hListView, &item);
	}
}

//=============================================================================
//=============================================================================
void NodeIndexDlgInit(HWND hWnd)
{
	HWND hListView = GetDlgItem(hWnd, IDC_NODEINDEX_LIST);
	ListView_DeleteAllItems(hListView);

	ListView_SetExtendedListViewStyleEx(hListView, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	LV_COLUMN lColumn;
	lColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_TEXT; //| LVCF_DEFAULTWIDTH;
	lColumn.fmt = LVCFMT_RIGHT;
	lColumn.pszText = _T("Name");
	lColumn.cx = 90;
	lColumn.iSubItem = 0;
	ListView_InsertColumn(hListView, 0, &lColumn);
	lColumn.pszText = _T("Sel");
	lColumn.cx = 35;
	lColumn.iSubItem = 1;
	ListView_InsertColumn(hListView, 1, &lColumn);
	lColumn.pszText = _T("Hov");
	lColumn.cx = 35;
	lColumn.iSubItem = 2;
	ListView_InsertColumn(hListView, 2, &lColumn);	lColumn.pszText = _T("Vis");
	lColumn.cx = 35;
	lColumn.iSubItem = 3;
	ListView_InsertColumn(hListView, 3, &lColumn);

	s_NodeIndexMap.clear();
	SetNodeIndexListRec(GetCOREInterface()->GetRootNode());

	SetList(hListView);

	//ListView_SortItems(hListViewWnd, CompareFunc, (LPARAM)0);
}

//===================================================
//===================================================
void GetSelectedListViewItemTable(HWND hListView, std::vector<INode*> &NodeTbl, std::vector<int> &SelIdxTbl)
{
	int iPos = -1;
	while (true) {
		iPos = ListView_GetNextItem(hListView, iPos, LVNI_SELECTED);
		if (iPos == -1) break;

		LVITEM lvi = { 0 };
		lvi.mask = LVIF_PARAM;
		lvi.iItem = iPos;
		lvi.iSubItem = 0;

		if (ListView_GetItem(hListView, &lvi)) {
			LPARAM lParam = lvi.lParam;

			if (lParam != NULL) {
				NodeTbl.push_back(reinterpret_cast<INode*>(lParam));
				SelIdxTbl.push_back(iPos);
			}
		}
	}
}

//===================================================
// Add Extensions to selected Nodes
//===================================================
void HSglTFTool::SetNodeAttr(HWND hWnd)
{
	HWND hListView = GetDlgItem(hWnd, IDC_NODEINDEX_LIST);

	std::vector<INode*> NodeTbl;
	std::vector<int> SelIdxTbl;
	GetSelectedListViewItemTable(hListView, NodeTbl, SelIdxTbl);

	SelectabilityStruct selStr;
	HoverabilityStruct hovStr;
	VisibilityStruct visStr;

	for (auto pNode : NodeTbl) {

		glTFImporter_Core app;
		if (IsDlgButtonChecked(hWnd, IDC_SELECT_CHK))
			app.CreateSelectabilityAttr(pNode, selStr, FALSE);
		else
			RemoveNodeAttributes(pNode, _T("Selectability"));

		if (IsDlgButtonChecked(hWnd, IDC_HOVER_CHK))
			app.CreateHoverabilityAttr(pNode, hovStr, FALSE);
		else
			RemoveNodeAttributes(pNode, _T("Hoverability"));

		if (IsDlgButtonChecked(hWnd, IDC_VISIBLE_CHK))
			app.CreateVisibilityAttr(pNode, visStr, FALSE);
		else
			RemoveNodeAttributes(pNode, _T("Visibility"));
	}

	ListView_DeleteAllItems(hListView);
	s_NodeIndexMap.clear();
	SetNodeIndexListRec(GetCOREInterface()->GetRootNode());
	SetList(hListView);

	UINT state = LVIS_SELECTED | LVIS_FOCUSED;
	UINT mask = LVIS_SELECTED | LVIS_FOCUSED;
	for (auto idx : SelIdxTbl) {
		ListView_SetItemState(hListView, idx, state, mask);
	}
}

//=============================================================================
//=============================================================================
void HSglTFTool::SetNodeExtentionValue(HWND hWnd)
{
	HWND hListView = GetDlgItem(hWnd, IDC_NODEINDEX_LIST);

	std::vector<INode*> NodeTbl;
	std::vector<int> SelIdxTbl;
	GetSelectedListViewItemTable(hListView, NodeTbl, SelIdxTbl);

	BOOL SelVal = IsDlgButtonChecked(hWnd, IDC_SEL_CHECK);
	BOOL HovVal = IsDlgButtonChecked(hWnd, IDC_HOV_CHECK);
	BOOL VisVal = IsDlgButtonChecked(hWnd, IDC_VIS_CHECK);

	for (auto pNode : NodeTbl) {
		glTFImporter_Core app;
		IParamBlock2* pBlock = NULL;
		app.GetCustAttrPBlock(pNode->GetObjectRef(), _T("Selectability"), pBlock);
		if (pBlock) {
			pBlock->SetValueByName(_T("Selectable"), SelVal, 0, 0);
		}
		app.GetCustAttrPBlock(pNode->GetObjectRef(), _T("Hoverability"), pBlock);
		if (pBlock) {
			pBlock->SetValueByName(_T("Hoverable"), HovVal, 0, 0);
		}
		app.GetCustAttrPBlock(pNode->GetObjectRef(), _T("Visibility"), pBlock);
		if (pBlock) {
			pBlock->SetValueByName(_T("Visible"), VisVal, 0, 0);
		}
	}

	ListView_DeleteAllItems(hListView);
	s_NodeIndexMap.clear();
	SetNodeIndexListRec(GetCOREInterface()->GetRootNode());
	SetList(hListView);

	UINT state = LVIS_SELECTED | LVIS_FOCUSED;
	UINT mask = LVIS_SELECTED | LVIS_FOCUSED;
	for (auto idx : SelIdxTbl) {
		ListView_SetItemState(hListView, idx, state, mask);
	}
}

//=============================================================================
//=============================================================================
void SubNodeRec(INode* pNode, std::map<DWORD, std::vector<ReferenceTarget*>>& tbl)
{
	if (pNode) {
		for (int i = 0; i < pNode->NumChildren(); i++) {
			SubNodeRec(pNode->GetChildNode(i), tbl);
		}
	}
}
