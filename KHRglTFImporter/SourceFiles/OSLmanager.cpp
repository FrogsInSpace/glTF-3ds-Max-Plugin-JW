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
#include <MaxOSLInterface.h>
#include <maxscript\maxscript.h>

//----------------------------------------------------------------------
//----------------------------------------------------------------------
static TSTR CutOffCode = _T("shader HSFloatCutOff\n\
[[string help = \"CutOff a float number\",\n\
string label = \"Cut Off\",\n\
string category = \"Math Float\"]]\n\
(\n\
	float CutOff = 0.0[[string label = \"CutOff (0.0-1.0)\"]],\n\
	float Input = 0.0,\n\
	float Scale = 1.0,\n\
	output float Out = 0.0,\n\
	)\n\
{\n\
	if (Input*Scale > CutOff) Out = 1.0;\n\
	else  Out = 0.0;\n\
}");

//----------------------------------------------------------------------
//----------------------------------------------------------------------
static TSTR FlipNormalCode = _T("shader NormalFlip\n\
	[[ string help = \"Flip normal\",\n\
	string label = \"Normal Flip\",\n\
	string category = \"Scene Attributes\" ]]\n\
(\n\
	int FlipGreen = 0 [[\n\
		string widget = \"checkbox\",\n\
		int connectable = 0,\n\
		string label = \"Flip Green Channel\" ]],\n\
	int FlipRed = 0 [[\n\
		string widget = \"checkbox\",\n\
		int connectable = 0,\n\
		string label = \"Flip Red Channel\" ]],\n\
		color Input = color(0.0, 0.0, 0.0) [[\n\
			string widget = \"null\"]],\n\
		output color OutColor = color(0.0, 0.0, 0.0)\n\
)\n\
{\n\
	OutColor = Input;\n\
	if (FlipGreen == 1) OutColor = color(Input.r, 1.0 - Input.g, Input.b);\n\
	if (FlipRed == 1) OutColor = color(1.0 - OutColor.r, OutColor.g, OutColor.b);\n\
}");
#if 0
//----------------------------------------------------------------------
//----------------------------------------------------------------------
static TSTR AlphaFromTexCode = _T("shader HSglTFAlphaChannel\n\
[[string help = \"Alpha Channel\",\n\
string label = \"Alpha Channel\",\n\
string version = \"1.0\"]]\n\
(\n\
	color Input = 0[[string widget = \"null\"]],\n\
	output float Alpha = 0, \n\
	)\n\
{\n\
	Alpha = Input[4]; \n\
}");
#endif
//----------------------------------------------------------------------
//----------------------------------------------------------------------
static TSTR AlphaFromColor4 = _T("shader HSglTFAlphaChannel\n\
[[string help = \"Alpha Channel\",\n\
string label = \"Alpha Channel\",\n\
string version = \"1.0\"]]\n\
(\n\
	color Input = color(1.0, 1.0, 1.0, 1.0) [[string widget = \"null\"]],\n\
	output float Alpha = 1.0, \n\
	)\n\
{\n\
	Alpha = Input[3]; \n\
}");

//----------------------------------------------------------------------
//----------------------------------------------------------------------
static TSTR MetalRoughOccCode = _T("shader HSglTFMtlRghOccDivide\n\
[[string help = \"Divide RBG Channel\",\n\
string label = \"Divide RGB Channel\",\n\
string version = \"1.0\"]]\n\
(\n\
	color Input = 0[[string widget = \"null\"]],\n\
	output color Roughness = 0, \n\
	output color Metalness = 0, \n\
	output color Occlusion = 0, \n\
	)\n\
{\n\
	Occlusion = color(Input[0], Input[0], Input[0]); \n\
	Roughness = color(Input[1], Input[1], Input[1]); \n\
	Metalness = color(Input[2], Input[2], Input[2]); \n\
}");

//----------------------------------------------------------------------
//----------------------------------------------------------------------
static TSTR SpecGlossFilterCode = _T("shader HSglTFSpecGlossFilter\n\
[[ string help = \"Marge diffuse and Specular map\",\n\
string label = \"SpecGloss Filter\",\n\
string category = \"Math Float\" ]]\n\
(\n\
	color diffuse1 = 0.5 [[ string label = \"diffuse\"]],\n\
	color specular = 0.5 [[ string label = \"specular\"]],\n\
	output color Out = 0.0\n\
	)\n\
{\n\
	if (specular == color(56.0 / 255, 56.0 / 255, 56.0 / 255)) {\n\
		Out = diffuse1;\n\
	}\n\
	else {\n\
		Out = specular;\n\
	}\n\
}\n\
");

//=============================================================================
//=============================================================================
int GetOSLMapType(Texmap* pTex)
{
	if(!pTex) return OSL_UnSupport;
	if (pTex->ClassID() != OSLTex_CLASS_ID) return OSL_UnSupport;

	TSTR name = pTex->GetParamBlock(0)->GetStr(6, 0);
	if (name == _T("OSLBitmap2"))		return OSL_BitmapLookUp;
	if (name == _T("UberBitmap2"))		return OSL_UberBitmap;
	if (name == _T("UberBitmap2b"))		return OSL_UberBitmap;
	if (name == _T("HSFloatCutOff"))	return OSL_CutOff;
	if (name == _T("HSglTFAlphaChannel"))	return OSL_AlphaCh;
	if (name == _T("ColorScale"))		return OSL_ColorScale;
	if (name == _T("ColorMul"))		return OSL_ColorMultiply;

	return OSL_UnSupport;
}

//======================================================================
//======================================================================
Texmap *CreateSubtractOSLNode(Texmap *pTex)
{
	TSTR ComStr;
#if MAX_RELEASE > 26000
	TSTR fname = GetCOREInterface()->GetDir(APP_MAX_SYS_ROOT_DIR) + TSTR(_T("OSL\\FloatSub.osl"));
	ComStr.printf(_T("temp = OSLMap OSLPath:\"%s\""), fname.data());
#else
	tstring fname = GetCOREInterface()->GetDir(APP_MAX_SYS_ROOT_DIR) + tstring(_T("OSL\\FloatSub.osl"));
	ComStr.printf(_T("temp = OSLMap OSLPath:\"%s\""), fname.c_str());
#endif
	FPValue fpv;
#if MAX_RELEASE >= 24000
	ExecuteMAXScriptScript(ComStr, MAXScript::ScriptSource::NonEmbedded, FALSE, &fpv);
#else
	ExecuteMAXScriptScript(ComStr, FALSE, &fpv);
#endif
	Texmap *pOSLMap = fpv.tex;
	if (!pOSLMap) return NULL;
	auto pMapInterface = (MaxSDK::OSL::IOSLMapInterface *)pOSLMap->GetInterface(MAXOSL_OSLMAP_INTERFACE);
	IParamBlock2 *pPBlock = pMapInterface->GetParameters();
	pPBlock->SetValueByName(_T("A"), 1.0f, 0);
	pPBlock->SetValueByName(_T("B_map"), pTex, 0);

	return pOSLMap;
}

//======================================================================
//======================================================================
Texmap* CreateColorScaleOSLNode(Texmap* pTex, float scale)
{
	TSTR ComStr;
#if MAX_RELEASE > 26000
	TSTR fname = GetCOREInterface()->GetDir(APP_MAX_SYS_ROOT_DIR) + TSTR(_T("OSL\\ColorScale.osl"));
	ComStr.printf(_T("temp = OSLMap OSLPath:\"%s\""), fname.data());
#else
	tstring fname = GetCOREInterface()->GetDir(APP_MAX_SYS_ROOT_DIR) + tstring(_T("OSL\\ColorScale.osl"));
	ComStr.printf(_T("temp = OSLMap OSLPath:\"%s\""), fname.c_str());
#endif
	FPValue fpv;
#if MAX_RELEASE >= 24000
	ExecuteMAXScriptScript(ComStr, MAXScript::ScriptSource::NonEmbedded, FALSE, &fpv);
#else
	ExecuteMAXScriptScript(ComStr, FALSE, &fpv);
#endif
	Texmap* pOSLMap = fpv.tex;
	if (!pOSLMap) return NULL;
	auto pMapInterface = (MaxSDK::OSL::IOSLMapInterface*)pOSLMap->GetInterface(MAXOSL_OSLMAP_INTERFACE);
	IParamBlock2* pPBlock = pMapInterface->GetParameters();
	pPBlock->SetValueByName(_T("Scale"), scale, 0);
	pPBlock->SetValueByName(_T("In_map"), pTex, 0);

	return pOSLMap;
}

//======================================================================
//======================================================================
Texmap* CreateColorMultiplyOSLNode(Texmap* pTex, Color col)
{
	TSTR ComStr;
#if MAX_RELEASE > 26000
	TSTR fname = GetCOREInterface()->GetDir(APP_MAX_SYS_ROOT_DIR) + TSTR(_T("OSL\\ColorMul.osl"));
	ComStr.printf(_T("temp = OSLMap OSLPath:\"%s\""), fname.data());
#else
	tstring fname = GetCOREInterface()->GetDir(APP_MAX_SYS_ROOT_DIR) + tstring(_T("OSL\\ColorMul.osl"));
	ComStr.printf(_T("temp = OSLMap OSLPath:\"%s\""), fname.c_str());
#endif
	FPValue fpv;
#if MAX_RELEASE >= 24000
	ExecuteMAXScriptScript(ComStr, MAXScript::ScriptSource::NonEmbedded, FALSE, &fpv);
#else
	ExecuteMAXScriptScript(ComStr, FALSE, &fpv);
#endif
	Texmap* pOSLMap = fpv.tex;
	if (!pOSLMap) return NULL;
	auto pMapInterface = (MaxSDK::OSL::IOSLMapInterface*)pOSLMap->GetInterface(MAXOSL_OSLMAP_INTERFACE);
	IParamBlock2* pPBlock = pMapInterface->GetParameters();
	pPBlock->SetValueByName(_T("A_map"), pTex, 0);
	pPBlock->SetValueByName(_T("B"), col, 0);

	return pOSLMap;
}

//======================================================================
//======================================================================
Texmap *CreateCutOffOSLNode(Texmap *pTex, float value, Texmap* pAlphaTex)
{
	Texmap *pOSLMap = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, OSLTex_CLASS_ID);
	IParamBlock2 *pPBlock0 = pOSLMap->GetParamBlock(0);
	pPBlock0->SetValue(3, 0, CutOffCode);
	pPBlock0->SetValue(6, 0, _T("Float CutOff"));

#if MAX_RELEASE >= 24000
	pOSLMap->Update(0);
#else
	pOSLMap->Update(0, FOREVER);
#endif

	IParamBlock2 *pPBlock1 = pOSLMap->GetParamBlock(1);
	pPBlock1->SetValue(0, 0, value);
	pPBlock1->SetValue(5, 0, pTex);
	if (pAlphaTex) {
		pPBlock1->SetValue(6, 0, pAlphaTex);
	}

	return pOSLMap;
}
#if 0
//======================================================================
//======================================================================
Texmap* CreateAlphaChOSLNode(Texmap* pTex)
{
	Texmap* pOSLMap = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, OSLTex_CLASS_ID);
	IParamBlock2* pPBlock0 = pOSLMap->GetParamBlock(0);
	pPBlock0->SetValue(3, 0, AlphaFromTexCode);
	pPBlock0->SetValue(6, 0, _T("Alpha Channel"));

#if MAX_RELEASE >= 24000
	pOSLMap->Update(0);
#else
	pOSLMap->Update(0, FOREVER);
#endif

	IParamBlock2* pPBlock1 = pOSLMap->GetParamBlock(1);
	pPBlock1->SetValue(0, 0, pTex);

	return pOSLMap;
}
#endif
//======================================================================
//======================================================================
Texmap* CreateAlphaChOSLNode(AColor col)
{
	Texmap* pOSLMap = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, OSLTex_CLASS_ID);
	IParamBlock2* pPBlock0 = pOSLMap->GetParamBlock(0);
	pPBlock0->SetValue(3, 0, AlphaFromColor4);
	pPBlock0->SetValue(6, 0, _T("Alpha Channel"));

#if MAX_RELEASE >= 24000
	pOSLMap->Update(0);
#else
	pOSLMap->Update(0, FOREVER);
#endif

	IParamBlock2* pPBlock1 = pOSLMap->GetParamBlock(1);
	pPBlock1->SetValue(0, 0, col);

	return pOSLMap;
}

//======================================================================
//======================================================================
Texmap* CreateFlipNormalOSLNode(Texmap* pTex, BOOL FlipGreen, BOOL FlipRed)
{
	Texmap* pOSLMap = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, OSLTex_CLASS_ID);
	IParamBlock2* pPBlock0 = pOSLMap->GetParamBlock(0);
	pPBlock0->SetValue(3, 0, FlipNormalCode);
	pPBlock0->SetValue(6, 0, _T("Flip Normal"));

#if MAX_RELEASE >= 24000
	pOSLMap->Update(0);
#else
	pOSLMap->Update(0,FOREVER);
#endif

	IParamBlock2* pPBlock1 = pOSLMap->GetParamBlock(1);
	pPBlock1->SetValue(0, 0, FlipGreen);
	pPBlock1->SetValue(1, 0, FlipRed);
	pPBlock1->SetValue(6, 0, pTex);

	return pOSLMap;
}

//======================================================================
//======================================================================
Texmap *CreateMetalRoughOccOSLNode(Texmap *pTex, float value)
{
	Texmap* pOSLMap = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, OSLTex_CLASS_ID);
	IParamBlock2* pPBlock0 = pOSLMap->GetParamBlock(0);
	pPBlock0->SetValue(3, 0, MetalRoughOccCode);
	pPBlock0->SetValue(6, 0, _T("MetalRoughOcc"));

#if MAX_RELEASE >= 24000
	pOSLMap->Update(0);
#else
	pOSLMap->Update(0, FOREVER);
#endif

	IParamBlock2* pPBlock1 = pOSLMap->GetParamBlock(1);
	pPBlock1->SetValue(4, 0, pTex);

	return pOSLMap;
}

//======================================================================
//======================================================================
Texmap* CreateBitmapLookupOSLNode(const TSTR &fname)
{
#if MAX_RELEASE >= 25900
	TSTR oslFname = TSTR(GetCOREInterface()->GetDir(APP_MAX_SYS_ROOT_DIR)) + _T("OSL\\OSLBitmap2.osl");
#else
	TSTR oslFname = TSTR(GetCOREInterface()->GetDir(APP_MAX_SYS_ROOT_DIR)) + _T("OSL\\OSLBitmap.osl");
#endif

	Texmap* pOSLMap = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, OSLTex_CLASS_ID);
	IParamBlock2* pPBlock0 = pOSLMap->GetParamBlock(0);
	pPBlock0->SetValue(2, 0, oslFname);
	pPBlock0->SetValue(6, 0, _T("OSLBitmap2"));

#if MAX_RELEASE >= 24000
	pOSLMap->Update(0);
#else
	pOSLMap->Update(0, FOREVER);
#endif

	IParamBlock2* pPBlock1 = pOSLMap->GetParamBlock(1);
	pPBlock1->SetValue(2, 0, fname);

	return pOSLMap;
}
Texmap* CreateUberBitmapOSLNode(const TSTR& fname)
{
#if MAX_RELEASE >= 25900
	TSTR oslFname = TSTR(GetCOREInterface()->GetDir(APP_MAX_SYS_ROOT_DIR)) + _T("OSL\\UberBitmap2.osl");
#else
	TSTR oslFname = TSTR(GetCOREInterface()->GetDir(APP_MAX_SYS_ROOT_DIR)) + _T("OSL\\UberBitmap.osl");
#endif

	Texmap* pOSLMap = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, OSLTex_CLASS_ID);
	IParamBlock2* pPBlock0 = pOSLMap->GetParamBlock(0);
	pPBlock0->SetValue(2, 0, oslFname);
#if MAX_RELEASE >= 26900
	pPBlock0->SetValue(6, 0, _T("UberBitmap2b"));
#else
	pPBlock0->SetValue(6, 0, _T("UberBitmap2"));
#endif

#if MAX_RELEASE >= 24000
	pOSLMap->Update(0);
#else
	pOSLMap->Update(0, FOREVER);
#endif

	IParamBlock2* pPBlock1 = pOSLMap->GetParamBlock(1);
	pPBlock1->SetValue(10, 0, fname);
	pPBlock1->SetValue(11, 0, _T("Raw"));

	return pOSLMap;
}



//======================================================================
//======================================================================
Texmap* CreateSpecGlossFilterOSLNode(Texmap* pTex1, Texmap* pTex2)
{
	Texmap* pOSLMap = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, OSLTex_CLASS_ID);
	IParamBlock2* pPBlock0 = pOSLMap->GetParamBlock(0);
	pPBlock0->SetValue(3, 0, SpecGlossFilterCode);
	pPBlock0->SetValue(6, 0, _T("SpecGlossFilter"));

#if MAX_RELEASE >= 24000
	pOSLMap->Update(0);
#else
	pOSLMap->Update(0, FOREVER);
#endif

	IParamBlock2* pPBlock1 = pOSLMap->GetParamBlock(1);
	pPBlock1->SetValue(3, 0, pTex1);
	pPBlock1->SetValue(4, 0, pTex2);

	return pOSLMap;
}
