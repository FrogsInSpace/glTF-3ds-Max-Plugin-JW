
#include "KHRglTFImporter.h"
#include <iparamwire.h>

std::map<Mtl*, Texmap*> WireCtrlTable;

void InitWireConnectTable(void)
{
	WireCtrlTable.clear();
}
void SetWireConnectTable(Mtl *pMtl, Texmap *pTex)
{
	WireCtrlTable.insert(std::make_pair(pMtl, pTex));
}
void CreateWireConnect(void)
{
	IParamWireMgr* pPramWireCtrl = GetParamWireMgr();

	for (auto tbl : WireCtrlTable) {
		auto pMtl = tbl.first;
		auto pTex = tbl.second;

		IParamBlock2* pMtlBlock = pMtl->GetParamBlock(0);
		IParamBlock2* pTexBlock = pTex->GetParamBlock(0);

		Control* pClr1 = (Control*)GetCOREInterface()->CreateInstance(CTRL_POINT4_CLASS_ID, Class_ID(0x2013, 0x0));
		Control* pClr2 = (Control*)GetCOREInterface()->CreateInstance(CTRL_POINT4_CLASS_ID, Class_ID(0x2013, 0x0));

		Control* pWireClr1 = (Control*)GetCOREInterface()->CreateInstance(CTRL_POINT4_CLASS_ID, POINT4_WIRE_CONTROL_CLASS_ID);
		IBaseWireControl* pwc1 = GetWireControlInterface(pWireClr1);
		pwc1->set_driven_animation(pClr2);
		pwc1->set_expr_text(0, _T("Base_Color"));
		pwc1->doFixUp();
		pwc1->doValidate();

		Control* pWireClr2 = (Control*)GetCOREInterface()->CreateInstance(CTRL_POINT4_CLASS_ID, POINT4_WIRE_CONTROL_CLASS_ID);
		IBaseWireControl* pwc2 = GetWireControlInterface(pWireClr2);
		pwc2->set_driven_animation(pClr1);
		pwc2->set_expr_text(0, _T("Color_1"));
		pwc2->doFixUp();
		pwc2->doValidate();


		pMtlBlock->SetControllerByID(2, 0, pWireClr1);
		pTexBlock->SetControllerByID(0, 0, pWireClr2);

#if 0
		pPramWireCtrl->EditControllers(pClr1, pClr2);
		ReferenceTarget* pLeftParent = pMtlBlock;
		int  leftSubNum = 1;
		ReferenceTarget* pRightParent = pTexBlock;
		int  rightSubNum = 0;
		const MCHAR* leftExpr = _T("Base_Color");
		const MCHAR* rightExpr = _T("Color_1");

		pPramWireCtrl->Connect2Way(pLeftParent, leftSubNum, pRightParent, rightSubNum, leftExpr, rightExpr);
#endif
	}
}
