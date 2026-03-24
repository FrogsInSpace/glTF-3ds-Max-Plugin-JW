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

static HWND hProgressWnd;

static int Total_Node = 0;
static int Total_Mtl = 0;
static int Total_Texture = 0;
static int Total_Animation = 0;
static int Total_Skin = 0;
static int Total_Morph = 0;
static int Physic_Morph = 0;

//======================================================================
//======================================================================
INT_PTR CALLBACK HSglTFProgressDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	//TCHAR buf[MAX_PATH];

	switch (message) {
	case WM_INITDIALOG:
		CenterWindow(hWnd, GetParent(hWnd));
		return TRUE;

	case WM_CLOSE:
		return 1;
/*
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		}
		break;
*/
	}
	return 0;
}

//======================================================================
//======================================================================
void OpenProgreessDlg(cgltf_data* m_glTF_data)
{
	Total_Node = m_glTF_data->nodes_count;
	Total_Mtl = m_glTF_data->materials_count;
	Total_Texture = m_glTF_data->textures_count;
	Total_Animation = m_glTF_data->animations_count;
	Total_Skin = m_glTF_data->skins_count;
	Total_Morph = 0;

	hProgressWnd = ::CreateDialogParam(hInstance, MAKEINTRESOURCE(IDD_PROGRESS_DLG), GetActiveWindow(), HSglTFProgressDlgProc, (LPARAM)NULL);
}

//======================================================================
//======================================================================
TCHAR *GetProgressStr(void)
{
	static TSTR progSTr[] = {
		_T("."),
		_T(". . ."),
		_T(". . . . ."),
		_T(". . . . . . ."),
		_T(". . . . . . . . .")
	};
	static int ProgressCount = 0;
	static TCHAR ProgressBuf[MAX_PATH];
	int num = sizeof(progSTr) / sizeof(progSTr[0]);

	if (ProgressCount < 0)
	{
		_stprintf_s(ProgressBuf, sizeof(ProgressBuf), _T(" Finish."));
	}
	else {
		_stprintf_s(ProgressBuf, sizeof(ProgressBuf), _T(" %s"), progSTr[ProgressCount++ % num].data());
	}

	return ProgressBuf;
}

//======================================================================
//======================================================================
void SetNodeImportStatus(int Count)
{
	switch (Count) {
	case 0:	SendMessage(GetDlgItem(hProgressWnd, IDC_NODE_STATE), WM_SETTEXT, 0, (LPARAM)_T("InProgress")); break;
	case -1: {
		SendMessage(GetDlgItem(hProgressWnd, IDC_NODE_STATE), WM_SETTEXT, 0, (LPARAM)_T("Finish"));
		SendMessage(GetDlgItem(hProgressWnd, IDC_NODE_COUNT), WM_SETTEXT, 0, (LPARAM)_T(""));
		break;
	}
	default:SendMessage(GetDlgItem(hProgressWnd, IDC_NODE_COUNT), WM_SETTEXT, 0, (LPARAM)GetProgressStr()); break;
	}
}

//======================================================================
//======================================================================
void SetMtlImportStatus(int Count)
{
	switch (Count) {
	case 0:	SendMessage(GetDlgItem(hProgressWnd, IDC_MTL_STATE), WM_SETTEXT, 0, (LPARAM)_T("InProgress")); break;
	case -1: {
		SendMessage(GetDlgItem(hProgressWnd, IDC_MTL_STATE), WM_SETTEXT, 0, (LPARAM)_T("Finish"));
		SendMessage(GetDlgItem(hProgressWnd, IDC_MTL_COUNT), WM_SETTEXT, 0, (LPARAM)_T(""));
		break;
	}
	default:SendMessage(GetDlgItem(hProgressWnd, IDC_MTL_COUNT), WM_SETTEXT, 0, (LPARAM)GetProgressStr()); break;
	}
}
void SetTexImportStatus(int Count) {
	switch (Count) {
	case 0:	SendMessage(GetDlgItem(hProgressWnd, IDC_TEX_STATE), WM_SETTEXT, 0, (LPARAM)_T("InProgress")); break;
	case -1: {
		SendMessage(GetDlgItem(hProgressWnd, IDC_TEX_STATE), WM_SETTEXT, 0, (LPARAM)_T("Finish"));
		SendMessage(GetDlgItem(hProgressWnd, IDC_TEX_COUNT), WM_SETTEXT, 0, (LPARAM)_T(""));
		break;
	}
	default:SendMessage(GetDlgItem(hProgressWnd, IDC_TEX_COUNT), WM_SETTEXT, 0, (LPARAM)GetProgressStr()); break;
	}
}

//======================================================================
//======================================================================
void SetAnimImportStatus(int Count)
{
	switch (Count) {
	case 0:	SendMessage(GetDlgItem(hProgressWnd, IDC_ANIM_STATE), WM_SETTEXT, 0, (LPARAM)_T("InProgress")); break;
	case -1: {
		SendMessage(GetDlgItem(hProgressWnd, IDC_ANIM_STATE), WM_SETTEXT, 0, (LPARAM)_T("Finish"));
		SendMessage(GetDlgItem(hProgressWnd, IDC_ANIM_COUNT), WM_SETTEXT, 0, (LPARAM)_T(""));
		break;
	}
	default:SendMessage(GetDlgItem(hProgressWnd, IDC_ANIM_COUNT), WM_SETTEXT, 0, (LPARAM)GetProgressStr()); break;
	}
}
void SetSkinImportStatus(int Count) {
	switch (Count) {
	case 0:	SendMessage(GetDlgItem(hProgressWnd, IDC_SKIN_STATE), WM_SETTEXT, 0, (LPARAM)_T("InProgress")); break;
	case -1: {
		SendMessage(GetDlgItem(hProgressWnd, IDC_SKIN_STATE), WM_SETTEXT, 0, (LPARAM)_T("Finish"));
		SendMessage(GetDlgItem(hProgressWnd, IDC_SKIN_COUNT), WM_SETTEXT, 0, (LPARAM)_T(""));
		break;
	}
	default:SendMessage(GetDlgItem(hProgressWnd, IDC_SKIN_COUNT), WM_SETTEXT, 0, (LPARAM)GetProgressStr()); break;
	}
}

//======================================================================
//======================================================================
void SetPhysicImportStatus(int Count)
{
	switch (Count) {
	case 0:	SendMessage(GetDlgItem(hProgressWnd, IDC_PHYSIC_STATE), WM_SETTEXT, 0, (LPARAM)_T("InProgress")); break;
	case -1: {
		SendMessage(GetDlgItem(hProgressWnd, IDC_PHYSIC_STATE), WM_SETTEXT, 0, (LPARAM)_T("Finish"));
		SendMessage(GetDlgItem(hProgressWnd, IDC_PHYSIC_COUNT), WM_SETTEXT, 0, (LPARAM)_T(""));
		break;
	}
	default:SendMessage(GetDlgItem(hProgressWnd, IDC_PHYSIC_COUNT), WM_SETTEXT, 0, (LPARAM)GetProgressStr()); break;
	}
}
void CloseProgreessDlg(void)
{
	DestroyWindow(hProgressWnd);
}
