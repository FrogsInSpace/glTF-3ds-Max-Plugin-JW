#include "HSglTFImporter.h"



static void NodeIndexDlgInit(HWND hWnd);
static void DeleteNodeIndex(HWND hWnd);


//=============================================================================
// 
//=============================================================================
static LRESULT CALLBACK NodeIndexDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	int typeIdx;
	tstring valStr;

	switch (msg) {
	case WM_INITDIALOG:
		NodeIndexDlgInit(hWnd);
		return (INT_PTR)TRUE;

	case WM_DESTROY:
		break;

	case WM_COMMAND:
		switch (wParam) {
		case IDC_DEL_BTN:
			DeleteNodeIndex(hWnd);
			return TRUE;

		case IDOK:
			EndDialog(hWnd, 1);
			return TRUE;
		case IDCANCEL:
			EndDialog(hWnd, 0);
			return TRUE;
		}
		break;
	case WM_NOTIFY:
		if (LOWORD(wParam) == IDC_VARIABLR_LIST) {
			switch ((UINT)((LPNMHDR)lParam)->code) {
			case NM_CLICK:
				SelectVariableItem(hWnd);
				break;
			}
		}
	}

	return FALSE;
}

//=============================================================================
// Variableテーブルダイアログ
//=============================================================================
void HSglTFTool::NodeIndexDlg(void)
{
	s_variableTable = GetVariableTable();

	int ret = ::DialogBoxParam(hInstance, MAKEINTRESOURCE(IDD_VARIABLE_DLG), m_hWnd, VariableTableProc, 0);
	if (ret == 0) return;

	SetVariableTable(s_variableTable);
}

//=============================================================================
//=============================================================================
void NodeIndexDlgInit(HWND hWnd)
{
	HWND hListViewWnd = GetDlgItem(hWnd, IDC_VARIABLR_LIST);
	ListView_DeleteAllItems(hListViewWnd);

	ListView_SetExtendedListViewStyleEx(hListViewWnd, 0, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);

	LV_COLUMN lColumn;
	lColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_WIDTH | LVCF_TEXT; //| LVCF_DEFAULTWIDTH;
	lColumn.fmt = LVCFMT_RIGHT;
	lColumn.pszText = _T("id");
	lColumn.cx = 80;
	lColumn.iSubItem = 0;
	ListView_InsertColumn(hListViewWnd, 0, &lColumn);
	lColumn.pszText = _T("type");
	lColumn.cx = 40;
	lColumn.iSubItem = 1;
	ListView_InsertColumn(hListViewWnd, 1, &lColumn);
	lColumn.pszText = _T("Value");
	lColumn.cx = 100;
	lColumn.iSubItem = 2;
	ListView_InsertColumn(hListViewWnd, 2, &lColumn);
	lColumn.pszText = _T("Description");
	lColumn.cx = 200;
	lColumn.iSubItem = 3;
	ListView_InsertColumn(hListViewWnd, 3, &lColumn);

	int index = 0;
	for (auto p : s_variableTable) {
		LVITEM item;
		item.iItem = index++;
		item.mask = LVIF_TEXT;
		item.cchTextMax = MAX_PATH;

		item.iSubItem = 0;
		item.pszText = (LPWSTR)p.id.c_str();
		ListView_InsertItem(hListViewWnd, &item);

		item.iSubItem = 1;
		item.pszText = (LPWSTR)TypeArray[p.type].c_str();;
		ListView_SetItem(hListViewWnd, &item);

		item.iSubItem = 2;
		item.pszText = (LPWSTR)p.value.c_str();
		ListView_SetItem(hListViewWnd, &item);

		item.iSubItem = 3;
		item.pszText = (LPWSTR)p.description.c_str();
		ListView_SetItem(hListViewWnd, &item);
	}

}


//=============================================================================
//=============================================================================
void SelectVariableItem(HWND hWnd)
{
	HWND hListView = GetDlgItem(hWnd, IDC_VARIABLR_LIST);
	if (ListView_GetSelectedCount(hListView) != 1) return;
	int idx = ListView_GetSelectionMark(hListView);

	TCHAR buf[MAX_PATH];
	ListView_GetItemText(hListView, idx, 0, buf, sizeof(buf));
	SetWindowText(GetDlgItem(hWnd, IDC_ID_EDIT), buf);

	ListView_GetItemText(hListView, idx, 2, buf, sizeof(buf));
	SetWindowText(GetDlgItem(hWnd, IDC_VAL_EDIT), buf);

	ListView_GetItemText(hListView, idx, 1, buf, sizeof(buf));
	int index = SendMessage(GetDlgItem(hWnd, IDC_TYPE_COMBO), CB_FINDSTRINGEXACT, -1, (LPARAM)buf);
	SendMessage(GetDlgItem(hWnd, IDC_TYPE_COMBO), CB_SETCURSEL, (WPARAM)index, 0);

	ListView_GetItemText(hListView, idx, 3, buf, sizeof(buf));
	SetWindowText(GetDlgItem(hWnd, IDC_DESC_EDIT), buf);
}


//=============================================================================
//=============================================================================
void DeleteNodeIndex(HWND hWnd)
{
	HWND hListView = GetDlgItem(hWnd, IDC_VARIABLR_LIST);
	if (ListView_GetSelectedCount(hListView) != 1) return;
	int idx = ListView_GetSelectionMark(hListView);
	int num = ListView_GetItemCount(hListView);

	for (int i = idx + 1; i < num; i++) {
		TCHAR buf[MAX_PATH];
		ListView_GetItemText(hListView, i, 0, buf, sizeof(buf));
		ListView_SetItemText(hListView, i - 1, 0, buf);
		ListView_GetItemText(hListView, i, 1, buf, sizeof(buf));
		ListView_SetItemText(hListView, i - 1, 1, buf);
		ListView_GetItemText(hListView, i, 2, buf, sizeof(buf));
		ListView_SetItemText(hListView, i - 1, 2, buf);
		ListView_GetItemText(hListView, i, 3, buf, sizeof(buf));
		ListView_SetItemText(hListView, i - 1, 3, buf);
	}
	ListView_DeleteItem(hListView, num - 1);
}


