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

#include "KHRglTFImporter.h"
#include "jsmn.h"

#include <maxscript/util/listener.h>

#define USE_REFACTORED_CREATEPARAMTABLE

bool isNumber(const char* str)
{
	for (const char* c = str;*c ; c++) {
		if (*c == '.') continue;
		if (*c == '-') continue;
		if (isdigit(*c) == 0) return false;
	}
	return true;
}

//==========================================================
//==========================================================
int glTFImporter_Core::GetCustAttrPBlock(ReferenceTarget* pRef, const tstring& AttName, IParamBlock2* &pBlock)
{
	pBlock = NULL;

	ICustAttribContainer* pContainer = pRef->GetCustAttribContainer();
	if (!pContainer) return -1;

	for (int i = 0; i < pContainer->GetNumCustAttribs(); i++) {
		CustAttrib* pAttr = pContainer->GetCustAttrib(i);
		if (GetCustomAttrName(pAttr) != TSTR(_T("Custom_Attributes"))) continue;

		IParamBlock2* pParamBlk = pAttr->GetParamBlockByID(0);
		if (pParamBlk == NULL) continue;
		if (pParamBlk->GetParameterType(0) != TYPE_STRING) continue;
		TCHAR* s = (TCHAR*)pParamBlk->GetStr(0, m_time);
		if (tstring(s) != AttName) continue;

		pBlock = pParamBlk;
		return i;
	}

	return -1;
}


#ifdef USE_REFACTORED_CREATEPARAMTABLE
//==========================================================================
// Gemini REFACTORED CreateParamTableFromExtras() with general safeguarding 
//=========================================================================
void glTFImporter_Core::CreateParamTableFromExtras(cgltf_extras &extras, cgltf_size size, std::vector<custAttrParam> &attrTbl, BOOL FileAttFlae)
{
	attrTbl.clear();

	if (size == 0) {
		return;
	}

	// FIX 1: Use std::vector to guarantee automatic memory cleanup, eliminates memory leaks if exception is thrown later.
	std::vector<char> extras_buffer(size);
	cgltf_copy_extras_json(m_glTF_data, &extras, extras_buffer.data(), &size);

	jsmn_parser p;
	jsmn_init(&p);

	// FIX 2: Capture as a standard signed 'int' to match signature and properly catch possible negative error codes.
	int parsedTokens = jsmn_parse(&p, extras_buffer.data(), size, NULL, 0);
	if (parsedTokens <= 0) 
		return; 

	size_t tokenNum = static_cast<size_t>(parsedTokens);
	std::vector<jsmntok_t> tokens(tokenNum);

	jsmn_init(&p);
	jsmn_parse(&p, extras_buffer.data(), size, tokens.data(), static_cast<unsigned int>(tokenNum));

	BOOL TitleFlag = TRUE;
	BOOL ParamFlag = FALSE;
	const int BUFFER_SIZE = 10000;
	custAttrParam param;

	for (size_t i = 0; i < tokenNum; i++) {
		char buf[BUFFER_SIZE];
		jsmntok_t tok = tokens[i];

		if (tok.type == JSMN_STRING) {
			memset(buf, 0, BUFFER_SIZE);
			strncpy_s(buf, BUFFER_SIZE, extras_buffer.data() + tok.start, std::clamp(tok.end - tok.start, 0, BUFFER_SIZE - 1));
			if (TitleFlag) {
				param.name = buf;
				TitleFlag = FALSE;
				ParamFlag = TRUE;
			}
			else if (ParamFlag) {
				if (isNumber(buf)) {
					param.type = TYPE_FLOAT;
					param.fParam = (float)atof(buf);
					param.fminParam = -100000000.0f;
					param.fmaxParam = 100000000.0f;
				}
				else {
					param.type = TYPE_STRING;
					param.sParam = buf;
				}
				TitleFlag = TRUE;
				ParamFlag = FALSE;
				attrTbl.push_back(param);
			}
		}
		else if (tok.type == JSMN_PRIMITIVE) {
			memset(buf, 0, BUFFER_SIZE);
			strncpy_s(buf, BUFFER_SIZE, extras_buffer.data() + tok.start, std::clamp(tok.end - tok.start, 0, BUFFER_SIZE - 1));
			if (strchr(buf, '.')) {
				param.type = TYPE_FLOAT;
				param.fParam = (float)atof(buf);
				param.fminParam = -100000000.0f;
				param.fmaxParam = 100000000.0f;
			}
			else {
				param.type = TYPE_INT;
				param.iParam = atoi(buf);
				param.iminParam = -100000000;
				param.imaxParam = 100000000;
			}
			TitleFlag = TRUE;
			ParamFlag = FALSE;
			attrTbl.push_back(param);
		}

		if (tok.type == JSMN_ARRAY) {
			// FIX 3: Ensure there is at least one token following the array container
			if (i + 1 >= tokenNum)
				break;

			i++; // Safely step past the JSMN_ARRAY token itself to reach the children

			if (FileAttFlae) {
				for (int xx = 0; xx < tok.size; xx++) {
					// FIX 4: Strict heap out-of-bounds tracking inside the token consumer loop
					if (i >= tokenNum) 
						break;

					jsmntok_t tk = tokens[i++];
					memset(buf, 0, BUFFER_SIZE);
					strncpy_s(buf, BUFFER_SIZE, extras_buffer.data() + tk.start, std::clamp(tk.end - tk.start, 0, BUFFER_SIZE - 1));

					param.sParam = std::string(buf);
					param.type = TYPE_STRING;
					TitleFlag = TRUE;
					ParamFlag = FALSE;
					attrTbl.push_back(param);
				}
			}
			else {
				float val[4] = {0.0f};
				int ss = tok.size > 4 ? 4 : tok.size;
				for (int xx = 0; xx < ss; xx++) {
					// FIX 4: Strict heap out-of-bounds tracking inside the token consumer loop
					if (i >= tokenNum) 
						break;

					jsmntok_t tk = tokens[i++];
					memset(buf, 0, BUFFER_SIZE);
					strncpy_s(buf, BUFFER_SIZE, extras_buffer.data() + tk.start, std::clamp(tk.end - tk.start, 0, BUFFER_SIZE - 1));
					val[xx] = (float)atof(buf);
				}

				// FIX 5: If a malicious or malformed file provides an RGBA array with > 4 items, 
				// safely consume the remaining elements so they don't break the outer loop parser alignment.
				if (tok.size > 4) {
					int remaining = tok.size - 4;
					for (int xx = 0; xx < remaining; xx++) {
						if (i >= tokenNum) break;
						i++;
					}
				}

				param.type = TYPE_RGBA;
				param.cParam = Color(val);
				TitleFlag = TRUE;
				ParamFlag = FALSE;
				attrTbl.push_back(param);
			}

			// FIX 6: Offset the outer loop's mandatory 'i++'. Otherwise the token  immediately following the array is completely skipped.
			i--; 
		}
	}
}

#else
//=========================================================================
// ORIGINAL CreateParamTableFromExtras() with added buffer bounds safeguarding 
//=========================================================================
void glTFImporter_Core::CreateParamTableFromExtras(cgltf_extras &extras, cgltf_size size, std::vector<custAttrParam> &attrTbl, BOOL FileAttFlae)
{
	attrTbl.clear();

	char* extras_buffer = new char[size];
	cgltf_copy_extras_json(m_glTF_data, &extras, extras_buffer, &size);

	jsmn_parser p;
	//const char* js = "{\"foo\": \"bar\", \"baz\": [1,true]}";
	jsmn_init(&p);
	size_t tokenNum = jsmn_parse(&p, extras_buffer, size, NULL, (size_t)0);
	//size_t tokenNum = 10;
	if (tokenNum <= 0) { delete[] extras_buffer; return; }

	//jsmntok_t tokens[10] = { (jsmntype_t)0 };
	jsmntok_t* tokens = new jsmntok_t[tokenNum];// { (jsmntype_t)0 };
	jsmn_init(&p);
	jsmn_parse(&p, extras_buffer, size, tokens, static_cast<unsigned int>(tokenNum));

	BOOL TitleFlag = TRUE;
	BOOL ParamFlag = FALSE;
	const int BUFFER_SIZE = 10000;
	custAttrParam param;
	for (int i = 0; i < tokenNum; i++) {
		char buf[BUFFER_SIZE];
		jsmntok_t tok = tokens[i];
		if (tok.type == JSMN_STRING) {
			memset(buf, 0, BUFFER_SIZE);
			strncpy_s(buf, BUFFER_SIZE, extras_buffer + tok.start,  std::clamp(tok.end - tok.start, 0, BUFFER_SIZE - 1));
			if (TitleFlag) {
				param.name = buf;
				TitleFlag = FALSE;
				ParamFlag = TRUE;
			}
			else if (ParamFlag) {
				if (isNumber(buf)) {
					param.type = TYPE_FLOAT;
					param.fParam = (float)atof(buf);
					param.fminParam = -100000000.0f;
					param.fmaxParam = 100000000.0f;
				}
				else {
					param.type = TYPE_STRING;
					param.sParam = buf;
				}
				TitleFlag = TRUE;
				ParamFlag = FALSE;
				attrTbl.push_back(param);
			}
		}
		else if (tok.type == JSMN_PRIMITIVE) {
			memset(buf, 0, BUFFER_SIZE);
			strncpy_s(buf, BUFFER_SIZE, extras_buffer + tok.start, std::clamp(tok.end - tok.start, 0, BUFFER_SIZE - 1));
			if (strchr(buf, '.')) {
				param.type = TYPE_FLOAT;
				param.fParam = (float)atof(buf);
				param.fminParam = -100000000.0f;
				param.fmaxParam = 100000000.0f;
			}
			else {
				param.type = TYPE_INT;
				param.iParam = atoi(buf);
				param.iminParam = -100000000;
				param.imaxParam = 100000000;
			}
			TitleFlag = TRUE;
			ParamFlag = FALSE;
			attrTbl.push_back(param);
		}
		if (FileAttFlae) {
			if (tok.type == JSMN_ARRAY) {
				i++;
				for (int xx = 0; xx < tok.size; xx++) {
					jsmntok_t tk = tokens[i++];
					memset(buf, 0, BUFFER_SIZE);
					strncpy_s(buf, BUFFER_SIZE, extras_buffer + tk.start, std::clamp(tk.end - tk.start, 0 , BUFFER_SIZE - 1) );
					param.sParam = std::string(buf);
					param.type = TYPE_STRING;
					TitleFlag = TRUE;
					ParamFlag = FALSE;
					attrTbl.push_back(param);
				}
			}
		}
		else{
			if (tok.type == JSMN_ARRAY) {
				i++;
				float val[4];
				int ss = tok.size > 4 ? 4 : tok.size;
				for (int xx = 0; xx < ss; xx++) {
					jsmntok_t tk = tokens[i++];
					memset(buf, 0, BUFFER_SIZE);
					strncpy_s(buf,BUFFER_SIZE, extras_buffer + tk.start, std::clamp(tk.end - tk.start, 0 , BUFFER_SIZE - 1) );
					val[xx] = (float)atof(buf);
				}
				param.type = TYPE_RGBA;
				param.cParam = Color(val);
				TitleFlag = TRUE;
				ParamFlag = FALSE;
				attrTbl.push_back(param);
			}
		}
	}

	delete[] tokens;
	delete[] extras_buffer;
}
#endif


//======================================================================
//======================================================================
void glTFImporter_Core::CreateTargetListFromExtras(cgltf_extras& extras, cgltf_size size, std::vector<tstring>& tbl)
{
	tbl.clear();

	char* extras_buffer = new char[size];
	cgltf_copy_extras_json(m_glTF_data, &extras, extras_buffer, &size);

	jsmn_parser p;
	jsmn_init(&p);
	size_t tokenNum = jsmn_parse(&p, extras_buffer, size, NULL, (size_t)0);
	if (tokenNum <= 0) { delete[] extras_buffer; return; }

	jsmntok_t* tokens = new jsmntok_t[tokenNum];// { (jsmntype_t)0 };
	jsmn_init(&p);
	jsmn_parse(&p, extras_buffer, size, tokens, static_cast<unsigned int>(tokenNum));

	int index = -1;
	for (int i = 0; i < tokenNum; i++) {
		char buf[1000];
		jsmntok_t tok = tokens[i];
		if (tok.type == JSMN_STRING) {
			memset(buf, 0, sizeof(buf));
			strncpy_s(buf, sizeof(buf), extras_buffer + tok.start, tok.end - tok.start);
			if (!strcmp(buf, "targetNames")) {
				index = i;
				break;
			}
		}
	}
	if (index != -1) {
		jsmntok_t tok = tokens[++index];
		if (tok.type == JSMN_ARRAY) {
			index++;
			for (int xx = 0; xx < tok.size; xx++) {
				char buf[1000];
				jsmntok_t tk = tokens[index++];
				memset(buf, 0, sizeof(buf));
				strncpy_s(buf, sizeof(buf), extras_buffer + tk.start, tk.end - tk.start);
				tbl.push_back(StringToWString(buf));
			}
		}
	}

	delete[] tokens;
	delete[] extras_buffer;
}

//======================================================================
//======================================================================
void glTFImporter_Core::SetUserPropParam(INode *pNode, std::vector<custAttrParam>& attrTbl)
{
	for (auto param: attrTbl) {
		TSTR name(StringToWString(param.name.c_str()).c_str());
		switch(param.type) {
		case TYPE_INT:
			pNode->SetUserPropInt(name, param.iParam);
			break;
		case TYPE_FLOAT:
			pNode->SetUserPropFloat(name, param.fParam);
			break;
		default:
			pNode->SetUserPropString(name, StringToWString(param.sParam.c_str()).c_str());
		}
	}
}

//======================================================================
//======================================================================
Class_ID glTFImporter_Core::AttachCustAttr(Animatable* pAnim, std::vector<custAttrParam>& attrTbl, tstring AttrName)
{
	Class_ID ret(0,0);
	if(attrTbl.empty()) return ret;

	const bool hasName = AttrName.size() > 1;
	const TSTR nn = hasName ? TSTR(AttrName.c_str()) : _T("Custom_Attributes");

	TSTR ComStr;
	ComStr = _T("CAT_DEF = attributes Custom_Attributes\nversion:0\n(\nparameters main rollout:params\n(\n");

	TSTR paramBlock, rolloutBlock;

	for(const auto& param : attrTbl)
	{
		const TSTR name(StringToWString(param.name.c_str()).c_str());

		switch(param.type)
		{
			case TYPE_BOOL:
				paramBlock.printf(_T("'%s' Type:#boolean UI:'%s' Default:%s\n"),
					name, name, param.iParam ? _T("true") : _T("false"));

				rolloutBlock.printf(_T("checkbox '%s' \"%s\" Width:160 Height:16 Align:#center\n"),
					name, name);
				break;

			case TYPE_INT:
				paramBlock.printf(_T("'%s' Type:#integer UI:'%s' Default:%d\n"),
					name, name, param.iParam);

				rolloutBlock.printf(_T("spinner '%s' \"%s\" Width:160 Height:16 Align:#center Type:#integer Range:[%d,%d,%d]\n"),
					name, name, param.iminParam, param.imaxParam, param.iParam);
				break;

			case TYPE_DWORD:
				paramBlock.printf(_T("'%s' Type:#integer UI:'%s' Default:%dL\n"),
					name, name, param.iParam);

				rolloutBlock.printf(_T("edittext '%s' \"%s\" Width:160 Height:16 Align:#center Type:#integer\n"),
					name, name);
				break;

			case TYPE_FLOAT:
				paramBlock.printf(_T("'%s' Type:#float UI:'%s' Default:%f\n"),
					name, name, param.fParam);

				rolloutBlock.printf(_T("spinner '%s' \"%s\" Width:160 Height:16 Align:#center Type:#float Range:[%f,%f,%f]\n"),
					name, name, param.fminParam, param.fmaxParam, param.fParam);
				break;

			case TYPE_RGBA:
				paramBlock.printf(_T("'%s' Type:#rgb UI:'%s' Default:(color %d %d %d)\n"),
					name, name,
					(int)(255.f*param.cParam.r),
					(int)(255.f*param.cParam.g),
					(int)(255.f*param.cParam.b));

				rolloutBlock.printf(_T("colorPicker '%s' \"%s\" Width:160 Height:25 Align:#center Color:(color %d %d %d)\n"),
					name, name,
					(int)(255.f*param.cParam.r),
					(int)(255.f*param.cParam.g),
					(int)(255.f*param.cParam.b));
				break;

			case TYPE_STRING:
			{
				const TSTR val(StringToWString(param.sParam.c_str()).c_str());

				if(param.inVisible)
					paramBlock.printf(_T("'%s' Type:#string Default:@\"%s\"\n"),
						name, val);
				else
					paramBlock.printf(_T("'%s' Type:#string UI:'%s' Default:@\"%s\"\n"),
						name, name, val);

				if(!param.inVisible)
					rolloutBlock.printf(_T("edittext '%s' \"%s\" Width:300 Height:17 Align:#center labelOnTop:false\n"),
						name, name);
				break;
			}

			case TYPE_TEXMAP:
				paramBlock.printf(_T("'%s' Type:#texturemap UI:'%s'\n"),
					name, name);

				rolloutBlock.printf(_T(
					"label 'lb_%s' \"%s\" Width:140 Height:17 Align:#center across:2\n"
					"mapbutton '%s' \"%s\" Width:140 Height:17 Align:#center\n"),
					name, name, name, name);
				break;
		}
	}

	ComStr += paramBlock;
	ComStr += _T(")\nRollout Params \"");
	ComStr += nn;
	ComStr += _T("\"\n(\n");
	ComStr += rolloutBlock;
	ComStr += _T(")\n)\n");

	FPValue obj(TYPE_VALUE,&undefined);

	switch(pAnim->SuperClassID())
	{
		case BASENODE_CLASS_ID: obj = FPValue(TYPE_INODE, pAnim); break;
		case MATERIAL_CLASS_ID: obj = FPValue(TYPE_MTL,pAnim); break;
		case TEXMAP_CLASS_ID:   obj = FPValue(TYPE_TEXMAP,pAnim); break;
	}

	SetCurrentAttrObj(obj);

	ComStr += _T("CustAttributes.add (KHRglTFImporter.GetCurrentAnim()) CAT_DEF\n");

	static unsigned int caDefCounter = 0;
	caDefCounter++;

#ifdef _DEBUG
	the_listener->edit_stream->printf(_T("%d\n"), caDefCounter);
	//the_listener->edit_stream->printf(_T("%d\n"), MSTR(s.c_str()).data());
#endif

	FPValue fpv;
#if MAX_RELEASE >= 24000
	if((++caDefCounter % 500u) == 0)
		ExecuteMAXScriptScript(_T("gc light:true"),MAXScript::ScriptSource::NonEmbedded, TRUE);

	ExecuteMAXScriptScript(ComStr, MAXScript::ScriptSource::NonEmbedded,  TRUE, &fpv);
#else
	if((++caDefCounter % 500u) == 0)
		ExecuteMAXScriptScript(_T("gc light:true"), TRUE);

	ExecuteMAXScriptScript(ComStr,TRUE, &fpv);
#endif

	if(fpv.type == TYPE_INT64_TAB)
	{
		ret.SetPartA((ulong)(*fpv.i64_tab)[0]);
		ret.SetPartB((ulong)(*fpv.i64_tab)[1]);
	}

	m_CustAttrMap[ret] = AttrName;

	return ret;
}

//======================================================================
//======================================================================
void glTFImporter_Core::CreateUnlitAttr(Mtl* pMtl, BOOL unlit)
{
	IParamBlock2 *pBlock = NULL;
	if(GetCustAttrPBlock(pMtl, tstring(_T("Unlit")), pBlock)!=-1) return;

	std::vector<custAttrParam> attrTbl;
	custAttrParam param;

	param.name = std::string("Unlit");
	param.type = TYPE_BOOL;
	param.iParam = unlit;
	attrTbl.push_back(param);

	Class_ID retID = AttachCustAttr(pMtl, attrTbl, _T("Unlit"));


}


//======================================================================
//======================================================================
void glTFImporter_Core::CreateIridescenceAttr(Mtl *pMtl, cgltf_iridescence* iridescence, BOOL enabled)
{
	IParamBlock2 *pBlock = NULL;
	if (GetCustAttrPBlock(pMtl, tstring(_T("Iridescence")), pBlock)!=-1) return;

	std::vector<custAttrParam> attrTbl;
	custAttrParam param;

	param.name = std::string("Enabled");
	param.type = TYPE_BOOL;
	param.iParam = enabled;
	attrTbl.push_back(param);

	param.name = std::string("iridescenceFactor");
	param.type = TYPE_FLOAT;
	param.fParam = iridescence->iridescence_factor;
	param.fminParam = 0.0f;
	param.fmaxParam = 1.0;
	attrTbl.push_back(param);

	param.name = std::string("iridescenceIor");
	param.type = TYPE_FLOAT;
	param.fParam = iridescence->iridescence_ior;
	param.fminParam = 1.0f;
	param.fmaxParam = 10.0;
	attrTbl.push_back(param);

	param.name = std::string("iridescenceThicknessMinimum");
	param.type = TYPE_FLOAT;
	param.fParam = iridescence->iridescence_thickness_min;
	param.fminParam = 0.0f;
	param.fmaxParam = 100000.0;
	attrTbl.push_back(param);

	param.name = std::string("iridescenceThicknessMaximum");
	param.type = TYPE_FLOAT;
	param.fParam = iridescence->iridescence_thickness_max;
	param.fminParam = 0.0f;
	param.fmaxParam = 100000.0;
	attrTbl.push_back(param);

	BitmapTex* pBmpTex1 = NULL;
	cgltf_texture_view *texInfo1 = &iridescence->iridescence_texture;
	if (texInfo1->texture) {
		pBmpTex1 = GetBitmapTexFromglTexture(texInfo1->texture);
		SetTextureUVoffset(pBmpTex1, texInfo1);
		param.name = std::string("iridescenceTexture");
		param.type = TYPE_TEXMAP;
		param.pParam = pBmpTex1;
		attrTbl.push_back(param);
	}
	BitmapTex* pBmpTex2 = NULL;
	cgltf_texture_view* texInfo2 = &iridescence->iridescence_thickness_texture;
	if (texInfo2->texture) {
		pBmpTex2 = GetBitmapTexFromglTexture(texInfo2->texture);
		SetTextureUVoffset(pBmpTex2, texInfo2);
		param.name = std::string("iridescenceThicknessTexture");
		param.type = TYPE_TEXMAP;
		param.pParam = pBmpTex2;
		attrTbl.push_back(param);
	}


	Class_ID retID = AttachCustAttr(pMtl, attrTbl, _T("Iridescence"));

	ICustAttribContainer *pContainer = pMtl->GetCustAttribContainer();
	if (!pContainer) return;

	CustAttrib* pAttr = NULL;
	for (int i = 0; i < pContainer->GetNumCustAttribs(); i++) {
		pAttr = pContainer->GetCustAttrib(i);
		//TSTR nn = pAttr->GetName();
		//if (pAttr->GetName() != TSTR(_T("Iridescence"))) continue;
		if (pAttr->ClassID() != retID) continue;

		//pAttr->GetName();
		IParamBlock2* pParamBlk = pAttr->GetParamBlockByID(0);
		if (pBmpTex1) {
			pParamBlk->SetValueByName(_T("iridescenceTexture"), pBmpTex1, 0);
		}
		if (pBmpTex2) {
			pParamBlk->SetValueByName(_T("iridescenceThicknessTexture"), pBmpTex2, 0);
		}
	}
}

//======================================================================
//======================================================================
void glTFImporter_Core::CreateIORAttr(Mtl* pMtl, cgltf_ior* ior, BOOL enabled)
{
	IParamBlock2 *pBlock = NULL;
	if (GetCustAttrPBlock(pMtl, tstring(_T("IOR")), pBlock)!=-1) return;

	std::vector<custAttrParam> attrTbl;
	custAttrParam param;

	param.name = std::string("Enabled");
	param.type = TYPE_BOOL;
	param.iParam = enabled;
	attrTbl.push_back(param);

	param.name = std::string("IOR");
	param.type = TYPE_FLOAT;
	param.fParam = ior->ior;
	param.fminParam = 1.0f;
	param.fmaxParam = 50.0f;
	attrTbl.push_back(param);

	Class_ID retID = AttachCustAttr(pMtl, attrTbl, _T("IOR"));
/*
	ICustAttribContainer* pContainer = pMtl->GetCustAttribContainer();

	if( pContainer ) {
		for (int i = 0; i < pContainer->GetNumCustAttribs(); i++) {
			CustAttrib* pAttr = pContainer->GetCustAttrib(i);
			//TSTR nn = pAttr->GetName();
			//if (pAttr->GetName() != TSTR(_T("Iridescence"))) continue;
			if (pAttr->ClassID() != retID) continue;
		}
	}
*/
}

//======================================================================
//======================================================================
void glTFImporter_Core::CreateEmissiveStrengthAttr(Mtl* pMtl, cgltf_emissive_strength* strength, BOOL enabled)
{
	IParamBlock2 *pBlock = NULL;
	if (GetCustAttrPBlock(pMtl, tstring(_T("EmissiveStrength")),pBlock)!=-1) return;

	std::vector<custAttrParam> attrTbl;
	custAttrParam param;

	param.name = std::string("Enabled");
	param.type = TYPE_BOOL;
	param.iParam = enabled;
	attrTbl.push_back(param);

	param.name = std::string("emissiveStrength");
	param.type = TYPE_FLOAT;
	param.fParam = strength->emissive_strength;
	param.fminParam = 0.0f;
	param.fmaxParam = 100.0f;
	attrTbl.push_back(param);

	Class_ID retID = AttachCustAttr(pMtl, attrTbl, _T("EmissiveStrength"));
}

//======================================================================
//======================================================================
void glTFImporter_Core::CreateVolumeAttr(Mtl* pMtl, cgltf_volume* volume, BOOL enabled)
{
	IParamBlock2 *pBlock = NULL;
	if (GetCustAttrPBlock(pMtl, tstring(_T("Volume")), pBlock) != -1) return;

	std::vector<custAttrParam> attrTbl;
	custAttrParam param;

	param.name = std::string("Enabled");
	param.type = TYPE_BOOL;
	param.iParam = enabled;
	attrTbl.push_back(param);

	param.name = std::string("thicknessFactor");
	param.type = TYPE_FLOAT;
	param.fParam = volume->thickness_factor;
	param.fminParam = 0.0f;
	param.fmaxParam = 1000000000.0f;
	attrTbl.push_back(param);

	param.name = std::string("attenuationDistance");
	param.type = TYPE_FLOAT;
	param.fParam = volume->attenuation_distance;
	param.fminParam = 0.0f;
	param.fmaxParam = 1000000000.0;
	attrTbl.push_back(param);

	param.name = std::string("attenuationColor");
	param.type = TYPE_RGBA;
	param.cParam = Color(volume->attenuation_color);
	attrTbl.push_back(param);

	BitmapTex* pBmpTex1 = NULL;
	cgltf_texture_view* texInfo1 = &volume->thickness_texture;
	if (texInfo1->texture) {
		pBmpTex1 = GetBitmapTexFromglTexture(texInfo1->texture);
		SetTextureUVoffset(pBmpTex1, texInfo1);
		param.name = std::string("thicknessTexture");
		param.type = TYPE_TEXMAP;
		param.pParam = pBmpTex1;
		attrTbl.push_back(param);
	}

	Class_ID retID = AttachCustAttr(pMtl, attrTbl, _T("Volume"));
	if (retID == Class_ID(0, 0)) return;

	ICustAttribContainer* pContainer = pMtl->GetCustAttribContainer();
	if(pContainer) {

		for(int i = 0; i < pContainer->GetNumCustAttribs(); i++) {
			CustAttrib* pAttr = pContainer->GetCustAttrib(i);
			if(pAttr->ClassID() != retID) continue;

			IParamBlock2* pParamBlk = pAttr->GetParamBlockByID(0);
			if(pBmpTex1) {
				pParamBlk->SetValueByName(_T("thicknessTexture"), pBmpTex1, 0);
			}
		}
	}
}

//======================================================================
//======================================================================
void glTFImporter_Core::CreateSheenAttr(Mtl* pMtl, cgltf_sheen* sheen, BOOL enabled)
{
	IParamBlock2 *pBlock = NULL;
	if (GetCustAttrPBlock(pMtl, tstring(_T("Sheen")),pBlock)!=-1) return;

	std::vector<custAttrParam> attrTbl;
	custAttrParam param;

	param.name = std::string("Enabled");
	param.type = TYPE_BOOL;
	param.iParam = enabled;
	attrTbl.push_back(param);

	param.name = std::string("sheenColorFactor");
	param.type = TYPE_RGBA;
	param.cParam = Color(sheen->sheen_color_factor);
	attrTbl.push_back(param);

	BitmapTex* pBmpTex1 = NULL;
	cgltf_texture_view* texInfo1 = &sheen->sheen_color_texture;
	if (texInfo1->texture) {
		pBmpTex1 = GetBitmapTexFromglTexture(texInfo1->texture);
		SetTextureUVoffset(pBmpTex1, texInfo1);
		param.name = std::string("sheenColorTexture");
		param.type = TYPE_TEXMAP;
		param.pParam = pBmpTex1;
		attrTbl.push_back(param);
	}

	param.name = std::string("sheenRoughnessFactor");
	param.type = TYPE_FLOAT;
	param.fParam = sheen->sheen_roughness_factor;
	param.fminParam = 0.0f;
	param.fmaxParam = 1.0;
	attrTbl.push_back(param);

	BitmapTex* pBmpTex2 = NULL;
	cgltf_texture_view* texInfo2 = &sheen->sheen_roughness_texture;
	if (texInfo2->texture) {
		pBmpTex2 = GetBitmapTexFromglTexture(texInfo2->texture);
		SetTextureUVoffset(pBmpTex2, texInfo2);
		param.name = std::string("sheenRoughnessTexture");
		param.type = TYPE_TEXMAP;
		param.pParam = pBmpTex2;
		attrTbl.push_back(param);
	}

	Class_ID retID = AttachCustAttr(pMtl, attrTbl, _T("Sheen"));

	ICustAttribContainer* pContainer = pMtl->GetCustAttribContainer();
	if(pContainer) {
		for(int i = 0; i < pContainer->GetNumCustAttribs(); i++) {
			CustAttrib* pAttr = pContainer->GetCustAttrib(i);
			if(pAttr->ClassID() != retID) continue;

			//		pAttr->GetName();
			IParamBlock2* pParamBlk = pAttr->GetParamBlockByID(0);
			if(pBmpTex1) {
				pParamBlk->SetValueByName(_T("sheenColorTexture"), pBmpTex1, 0);
			}
			if(pBmpTex2) {
				pParamBlk->SetValueByName(_T("sheenRoughnessTexture"), pBmpTex2, 0);
			}
		}
	}
}

//======================================================================
//======================================================================
void glTFImporter_Core::CreateClearcoatAttr(Mtl* pMtl, cgltf_clearcoat* clearcoat, BOOL enabled)
{
	IParamBlock2 *pBlock = NULL;
	if (GetCustAttrPBlock(pMtl, tstring(_T("Clearcoat")),pBlock)!=-1) return;

	std::vector<custAttrParam> attrTbl;
	custAttrParam param;

	param.name = std::string("Enabled");
	param.type = TYPE_BOOL;
	param.iParam = enabled;
	attrTbl.push_back(param);

	param.name = std::string("clearcoatFactor");
	param.type = TYPE_FLOAT;
	param.fParam = clearcoat->clearcoat_factor;
	param.fminParam = 0.0f;
	param.fmaxParam = 1.0;
	attrTbl.push_back(param);

	BitmapTex* pBmpTex1 = NULL;
	cgltf_texture_view* texInfo1 = &clearcoat->clearcoat_texture;
	if (texInfo1->texture) {
		pBmpTex1 = GetBitmapTexFromglTexture(texInfo1->texture);
		SetTextureUVoffset(pBmpTex1, texInfo1);
		param.name = std::string("clearcoatTexture");
		param.type = TYPE_TEXMAP;
		param.pParam = pBmpTex1;
		attrTbl.push_back(param);
	}

	param.name = std::string("clearcoatRoughnessFactor");
	param.type = TYPE_FLOAT;
	param.fParam = clearcoat->clearcoat_roughness_factor;
	param.fminParam = 0.0f;
	param.fmaxParam = 1.0;
	attrTbl.push_back(param);

	BitmapTex* pBmpTex2 = NULL;
	cgltf_texture_view* texInfo2 = &clearcoat->clearcoat_roughness_texture;
	if (texInfo2->texture) {
		pBmpTex2 = GetBitmapTexFromglTexture(texInfo2->texture);
		SetTextureUVoffset(pBmpTex2, texInfo2);
		param.name = std::string("clearcoatRoughnessTexture");
		param.type = TYPE_TEXMAP;
		param.pParam = pBmpTex2;
		attrTbl.push_back(param);
	}

	BitmapTex* pBmpTex3 = NULL;
	cgltf_texture_view* texInfo3 = &clearcoat->clearcoat_normal_texture;
	if (texInfo3->texture) {
		pBmpTex3 = GetBitmapTexFromglTexture(texInfo3->texture);
		SetTextureUVoffset(pBmpTex3, texInfo3);
		param.name = std::string("clearcoatNormalTexture");
		param.type = TYPE_TEXMAP;
		param.pParam = pBmpTex3;
		attrTbl.push_back(param);
	}

	Class_ID retID = AttachCustAttr(pMtl, attrTbl, _T("Clearcoat"));

	ICustAttribContainer* pContainer = pMtl->GetCustAttribContainer();
	if(pContainer) {
		for(int i = 0; i < pContainer->GetNumCustAttribs(); i++) {
			CustAttrib* pAttr = pContainer->GetCustAttrib(i);
			if(pAttr->ClassID() != retID) continue;

			//		pAttr->GetName();
			IParamBlock2* pParamBlk = pAttr->GetParamBlockByID(0);
			if(pBmpTex1) {
				pParamBlk->SetValueByName(_T("clearcoatTexture"), pBmpTex1, 0);
			}
			if(pBmpTex2) {
				pParamBlk->SetValueByName(_T("clearcoatRoughnessTexture"), pBmpTex2, 0);
			}
			if(pBmpTex3) {
				pParamBlk->SetValueByName(_T("clearcoatNormalTexture"), pBmpTex3, 0);
			}
		}
	}
}

//======================================================================
//======================================================================
void glTFImporter_Core::CreateTransmissionAttr(Mtl* pMtl, cgltf_transmission* transmission, BOOL enabled)
{
	IParamBlock2 *pBlock = NULL;
	if (GetCustAttrPBlock(pMtl, tstring(_T("Transmission")),pBlock)!=-1) return;

	std::vector<custAttrParam> attrTbl;
	custAttrParam param;

	param.name = std::string("Enabled");
	param.type = TYPE_BOOL;
	param.iParam = enabled;
	attrTbl.push_back(param);

	param.name = std::string("transmissionFactor");
	param.type = TYPE_FLOAT;
	param.fParam = transmission->transmission_factor;
	param.fminParam = 0.0f;
	param.fmaxParam = 1.0;
	attrTbl.push_back(param);

	BitmapTex* pBmpTex1 = NULL;
	cgltf_texture_view* texInfo1 = &transmission->transmission_texture;
	if (texInfo1->texture) {
		pBmpTex1 = GetBitmapTexFromglTexture(texInfo1->texture);
		SetTextureUVoffset(pBmpTex1, texInfo1);
		param.name = std::string("transmissionTexture");
		param.type = TYPE_TEXMAP;
		param.pParam = pBmpTex1;
		attrTbl.push_back(param);
	}


	Class_ID retID = AttachCustAttr(pMtl, attrTbl, _T("Transmission"));

	ICustAttribContainer* pContainer = pMtl->GetCustAttribContainer();
	if(pContainer) {
		for(int i = 0; i < pContainer->GetNumCustAttribs(); i++) {
			CustAttrib* pAttr = pContainer->GetCustAttrib(i);
			if(pAttr->ClassID() != retID) continue;

			IParamBlock2* pParamBlk = pAttr->GetParamBlockByID(0);
			if(pBmpTex1) {
				pParamBlk->SetValueByName(_T("transmissionTexture"), pBmpTex1, 0);
			}
		}
	}
}

//======================================================================
//======================================================================
void glTFImporter_Core::CreateDispersionAttr(Mtl* pMtl, cgltf_dispersion* dispersion, BOOL enabled)
{
	IParamBlock2 *pBlock = NULL;
	if (GetCustAttrPBlock(pMtl, tstring(_T("Dispersion")),pBlock)!=-1) return;

	std::vector<custAttrParam> attrTbl;
	custAttrParam param;

	param.name = std::string("Enabled");
	param.type = TYPE_BOOL;
	param.iParam = enabled;
	attrTbl.push_back(param);

	param.name = std::string("dispersion");
	param.type = TYPE_FLOAT;
	param.fParam = dispersion->dispersion;
	param.fminParam = 0.0f;
	param.fmaxParam = 100.0f;
	attrTbl.push_back(param);

	Class_ID retID = AttachCustAttr(pMtl, attrTbl, _T("Dispersion"));
}

//======================================================================
//======================================================================
void glTFImporter_Core::CreateAnisotropyAttr(Mtl* pMtl, cgltf_anisotropy* anisotropy, BOOL enabled)
{
	IParamBlock2 *pBlock = NULL;
	if (GetCustAttrPBlock(pMtl, tstring(_T("Anisotropy")),pBlock)!=-1) return;

	std::vector<custAttrParam> attrTbl;
	custAttrParam param;

	param.name = std::string("Enabled");
	param.type = TYPE_BOOL;
	param.iParam = enabled;
	attrTbl.push_back(param);

	param.name = std::string("anisotropyStrength");
	param.type = TYPE_FLOAT;
	param.fParam = anisotropy->anisotropy_strength;
	param.fminParam = 0.0f;
	param.fmaxParam = 1.0f;
	attrTbl.push_back(param);

	param.name = std::string("anisotropyRotation");
	param.type = TYPE_FLOAT;
	param.fParam = anisotropy->anisotropy_rotation;
	param.fminParam = 0.0f;
	param.fmaxParam = 1.0f;
	attrTbl.push_back(param);

	BitmapTex* pBmpTex1 = NULL;
	cgltf_texture_view* texInfo1 = &anisotropy->anisotropy_texture;
	if (texInfo1->texture) {
		pBmpTex1 = GetBitmapTexFromglTexture(texInfo1->texture);
		SetTextureUVoffset(pBmpTex1, texInfo1);
		param.name = std::string("anisotropyTexture");
		param.type = TYPE_TEXMAP;
		param.pParam = pBmpTex1;
		attrTbl.push_back(param);
	}


	Class_ID retID = AttachCustAttr(pMtl, attrTbl, _T("Anisotropy"));

	ICustAttribContainer* pContainer = pMtl->GetCustAttribContainer();
	if(pContainer) {
		for(int i = 0; i < pContainer->GetNumCustAttribs(); i++) {
			CustAttrib* pAttr = pContainer->GetCustAttrib(i);
			if(pAttr->ClassID() != retID) continue;

			IParamBlock2* pParamBlk = pAttr->GetParamBlockByID(0);
			if(pBmpTex1) {
				pParamBlk->SetValueByName(_T("anisotropyTexture"), pBmpTex1, 0);
			}
		}
	}
}

//======================================================================
//======================================================================
void glTFImporter_Core::CreateDiffuseTransmissionAttr(Mtl* pMtl, cgltf_diffuse_transmission* diffuse_transmission, BOOL enabled)
{
	IParamBlock2 *pBlock = NULL;
	if (GetCustAttrPBlock(pMtl, tstring(_T("DiffuseTransmission")),pBlock)!=-1) return;

	std::vector<custAttrParam> attrTbl;
	custAttrParam param;

	param.name = std::string("Enabled");
	param.type = TYPE_BOOL;
	param.iParam = enabled;
	attrTbl.push_back(param);

	param.name = std::string("diffuseTransmissionFactor");
	param.type = TYPE_FLOAT;
	param.fParam = diffuse_transmission->diffuseTransmissionFactor;
	param.fminParam = 0.0f;
	param.fmaxParam = 1.0f;
	attrTbl.push_back(param);

	param.name = std::string("diffuseTransmissionClr");
	param.type = TYPE_RGBA;
	param.cParam = Color(diffuse_transmission->diffuseTransmissionColorFactor);
	attrTbl.push_back(param);

	BitmapTex* pBmpTex1 = NULL;
	cgltf_texture_view* texInfo1 = &diffuse_transmission->diffuseTransmissionColorTexture;
	if (texInfo1 && texInfo1->texture) {
		pBmpTex1 = GetBitmapTexFromglTexture(texInfo1->texture);
		SetTextureUVoffset(pBmpTex1, texInfo1);
		param.name = std::string("diffuseTransmissionColorTexture");
		param.type = TYPE_TEXMAP;
		param.pParam = pBmpTex1;
		attrTbl.push_back(param);
	}

	BitmapTex* pBmpTex2 = NULL;
	cgltf_texture_view* texInfo2 = &diffuse_transmission->diffuseTransmissionTexture;
	if (texInfo2 && texInfo2->texture) {
		pBmpTex2 = GetBitmapTexFromglTexture(texInfo2->texture);
		SetTextureUVoffset(pBmpTex2, texInfo2);
		param.name = std::string("diffuseTransmissionTexture");
		param.type = TYPE_TEXMAP;
		param.pParam = pBmpTex2;
		attrTbl.push_back(param);
	}

	Class_ID retID = AttachCustAttr(pMtl, attrTbl, _T("DiffuseTransmission"));

	ICustAttribContainer* pContainer = pMtl->GetCustAttribContainer();
	if(pContainer) {
		for(int i = 0; i < pContainer->GetNumCustAttribs(); i++) {
			CustAttrib* pAttr = pContainer->GetCustAttrib(i);
			if(pAttr->ClassID() != retID) continue;

			IParamBlock2* pParamBlk = pAttr->GetParamBlockByID(0);
			if(pBmpTex1) {
				pParamBlk->SetValueByName(_T("diffuseTransmissionColorTexture"), pBmpTex1, 0);
			}
			if(pBmpTex2) {
				pParamBlk->SetValueByName(_T("diffuseTransmissionTexture"), pBmpTex2, 0);
			}
		}
	}
}

//======================================================================
//======================================================================
void glTFImporter_Core::CreateSpecularAttr(Mtl* pMtl, cgltf_specular* specular, BOOL enabled)
{
	IParamBlock2 *pBlock = NULL;
	if (GetCustAttrPBlock(pMtl, tstring(_T("Specular")),pBlock)!=-1) return;

	std::vector<custAttrParam> attrTbl;
	custAttrParam param;

	param.name = std::string("Enabled");
	param.type = TYPE_BOOL;
	param.iParam = enabled;
	attrTbl.push_back(param);

	param.name = std::string("specularFactor");
	param.type = TYPE_FLOAT;
	param.fParam = specular->specular_factor;
	param.fminParam = 0.0f;
	param.fmaxParam = 1.0f;
	attrTbl.push_back(param);

	BitmapTex* pBmpTex1 = NULL;
	cgltf_texture_view* texInfo1 = &specular->specular_texture;
	if (texInfo1 && texInfo1->texture) {
		pBmpTex1 = GetBitmapTexFromglTexture(texInfo1->texture);
		SetTextureUVoffset(pBmpTex1, texInfo1);
		param.name = std::string("specularTexture");
		param.type = TYPE_TEXMAP;
		param.pParam = pBmpTex1;
		attrTbl.push_back(param);
	}

	param.name = std::string("specularColorFactor");
	param.type = TYPE_RGBA;
	param.cParam = Color(specular->specular_color_factor);
	attrTbl.push_back(param);

	BitmapTex* pBmpTex2 = NULL;
	cgltf_texture_view* texInfo2 = &specular->specular_color_texture;
	if (texInfo2 && texInfo2->texture) {
		pBmpTex2 = GetBitmapTexFromglTexture(texInfo2->texture);
		SetTextureUVoffset(pBmpTex2, texInfo2);
		param.name = std::string("specularColorTexture");
		param.type = TYPE_TEXMAP;
		param.pParam = pBmpTex2;
		attrTbl.push_back(param);
	}

	Class_ID retID = AttachCustAttr(pMtl, attrTbl, _T("Specular"));

	ICustAttribContainer* pContainer = pMtl->GetCustAttribContainer();
	if(pContainer) {
		for(int i = 0; i < pContainer->GetNumCustAttribs(); i++) {
			CustAttrib* pAttr = pContainer->GetCustAttrib(i);
			if(!pAttr || pAttr->ClassID() != retID) continue;

			IParamBlock2* pParamBlk = pAttr->GetParamBlockByID(0);
			if(pBmpTex1) {
				pParamBlk->SetValueByName(_T("specularTexture"), pBmpTex1, 0);
			}
			if(pBmpTex2) {
				pParamBlk->SetValueByName(_T("specularColorTexture"), pBmpTex2, 0);
			}
		}
	}
}

//======================================================================
//======================================================================
void glTFImporter_Core::CreateWebpEncodingAttr(Texmap* pTex, const tstring &path, BOOL enabled)
{
	IParamBlock2 *pBlock = NULL;
	if (GetCustAttrPBlock(pTex, tstring(_T("Webp Encode")),pBlock)!=-1) return;

	std::vector<custAttrParam> attrTbl;
	custAttrParam param;

	param.name = std::string("Enabled");
	param.type = TYPE_BOOL;
	param.iParam = enabled;
	attrTbl.push_back(param);

	param.name = std::string("Original Path");
	param.type = TYPE_STRING;
	param.sParam = WStringToString(path);
	param.inVisible = FALSE;
	attrTbl.push_back(param);

	param.name = std::string("QualityFactor");
	param.type = TYPE_FLOAT;
	param.fParam = 100.0;
	param.fminParam = 0.0f;
	param.fmaxParam = 100.0f;
	attrTbl.push_back(param);

	param.name = std::string("Lossless");
	param.type = TYPE_BOOL;
	param.iParam = FALSE;
	attrTbl.push_back(param);

	Class_ID retID = AttachCustAttr(pTex, attrTbl, _T("Webp Encode"));
}

//======================================================================
//======================================================================
void glTFImporter_Core::CreateKTX2EncodingAttr(Texmap* pTex, const tstring& path, BOOL enabled)
{
	IParamBlock2* pBlock = NULL;
	if (GetCustAttrPBlock(pTex, tstring(_T("KTX2 Encode")), pBlock) != -1) return;

	std::vector<custAttrParam> attrTbl;
	custAttrParam param;

	param.name = std::string("Enabled");
	param.type = TYPE_BOOL;
	param.iParam = enabled;
	attrTbl.push_back(param);

	param.name = std::string("Original Path");
	param.type = TYPE_STRING;
	param.sParam = WStringToString(path);
	param.inVisible = FALSE;
	attrTbl.push_back(param);

	param.name = std::string("compression");
	param.type = TYPE_INT;
	param.iParam = 4;
	param.iminParam = 1;
	param.imaxParam = 5;
	attrTbl.push_back(param);

	param.name = std::string("quality");
	param.type = TYPE_INT;
	param.iParam = 128;
	param.iminParam = 1;
	param.imaxParam = 255;
	attrTbl.push_back(param);

	param.name = std::string("UASTC(On)/ETC1S(Off)");
	param.type = TYPE_BOOL;
	param.iParam = TRUE;
	attrTbl.push_back(param);

	param.name = std::string("mipmap");
	param.type = TYPE_BOOL;
	param.iParam = FALSE;
	attrTbl.push_back(param);

	Class_ID retID = AttachCustAttr(pTex, attrTbl, _T("KTX2 Encode"));
}



//======================================================================
//======================================================================
void glTFImporter_Core::CreateVRayExtAttr(Mtl* pMtl, const vrayExtStruct &vray, BOOL enabled)
{
	IParamBlock2 *pBlock = NULL;
	if (GetCustAttrPBlock(pMtl, tstring(_T("VRay Extension")),pBlock)!=-1) return;

	std::vector<custAttrParam> attrTbl;
	custAttrParam param;

	param.name = std::string("PBR roughness");
	param.type = TYPE_FLOAT;
	param.fParam = vray.roughness;
	param.fminParam = 0.0f;
	param.fmaxParam = 1.0f;
	attrTbl.push_back(param);

	Class_ID retID = AttachCustAttr(pMtl, attrTbl, _T("VRay Extension"));

	//ICustAttribContainer* pContainer = pMtl->GetCustAttribContainer();
}

//======================================================================
//======================================================================
void glTFImporter_Core::CreateSelectabilityAttr(INode* pNode, const SelectabilityStruct &str, BOOL enabled)
{
	IParamBlock2 *pBlock = NULL;
	if (GetCustAttrPBlock(pNode->GetObjectRef(), tstring(_T("Selectability")),pBlock)>=0) return;

	std::vector<custAttrParam> attrTbl;
	custAttrParam param;

	param.name = std::string("selectable");
	param.type = TYPE_BOOL;
	param.iParam = str.selectable;
	attrTbl.push_back(param);

	Class_ID retID = AttachCustAttr(pNode, attrTbl, _T("Selectability"));
}

//======================================================================
//======================================================================
void glTFImporter_Core::CreateHoverabilityAttr(INode* pNode, const HoverabilityStruct &str, BOOL enabled)
{
	IParamBlock2 *pBlock = NULL;
	if (GetCustAttrPBlock(pNode->GetObjectRef(), tstring(_T("Hoverability")),pBlock)>=0) return;

	std::vector<custAttrParam> attrTbl;
	custAttrParam param;

	param.name = std::string("hoverable");
	param.type = TYPE_BOOL;
	param.iParam = str.hoverable;
	attrTbl.push_back(param);

	Class_ID retID = AttachCustAttr(pNode, attrTbl, _T("Hoverability"));
}

//======================================================================
//======================================================================
void glTFImporter_Core::CreateVisibilityAttr(INode* pNode, const VisibilityStruct& str, BOOL enabled)
{
	IParamBlock2* pBlock = NULL;
	if (GetCustAttrPBlock(pNode->GetObjectRef(), tstring(_T("Visibility")), pBlock) >= 0) return;

	std::vector<custAttrParam> attrTbl;
	custAttrParam param;

	param.name = std::string("visible");
	param.type = TYPE_BOOL;
	param.iParam = str.visible;
	attrTbl.push_back(param);

	Class_ID retID = AttachCustAttr(pNode, attrTbl, _T("Visibility"));
}

//======================================================================
//======================================================================
DWORD glTFImporter_Core::CreateInteractivityAttr(ReferenceTarget* pRef, const InteractivityStruct& str, BOOL enabled)
{
	IParamBlock2* pBlock = NULL;
	ReferenceTarget* p = pRef;
	if (pRef->SuperClassID() == BASENODE_CLASS_ID) p = ((INode*)pRef)->GetObjectRef();
	if (!p) return FALSE;
	if (GetCustAttrPBlock(p, tstring(_T("Interactivity")), pBlock) >= 0) {
		const MCHAR *val = pBlock->GetStr(1, m_time, FOREVER);
		if (!val) return 0;
		return std::stoul(tstring(val));
	}

	std::vector<custAttrParam> attrTbl;
	custAttrParam param;

	param.name = std::string("id");
	param.type = TYPE_STRING;
	param.sParam = std::to_string(str.id);
	param.inVisible = TRUE;
	attrTbl.push_back(param);

	Class_ID retID = AttachCustAttr(pRef, attrTbl, _T("Interactivity"));
	return str.id;
}


//======================================================================
//======================================================================
BOOL glTFImporter_Core::GetInteractivityPointerID(ReferenceTarget* pRef, DWORD &id)
{
	if (!pRef) return FALSE;

	IParamBlock2* pBlock = NULL;
	ReferenceTarget* p = pRef;
	if (pRef->SuperClassID() == BASENODE_CLASS_ID) p = ((INode*)pRef)->GetObjectRef();
	if (!p) return FALSE;
	if (p->SuperClassID() == GEN_DERIVOB_CLASS_ID) {
		Object *pObj = ((IDerivedObject*)p)->GetObjRef();
		if (pObj) p = pObj;
		else return FALSE;
	}
	if (GetCustAttrPBlock(p, tstring(_T("Interactivity")), pBlock) >= 0) {
		const MCHAR* val = pBlock->GetStr(1, m_time, FOREVER);
		if (!val) return FALSE;
		id = std::stoul(tstring(val));
		return TRUE;
	}

	return FALSE;
}

//======================================================================
//======================================================================
BOOL glTFImporter_Core::RemoveInteractivityAttr(ReferenceTarget* pRef)
{
	if (!pRef) return FALSE;

	IParamBlock2* pBlock = NULL;
	ReferenceTarget* p = pRef;
	if (pRef->SuperClassID() == BASENODE_CLASS_ID) p = ((INode*)pRef)->GetObjectRef();
	if (!p) return FALSE;
	if (p->SuperClassID() == GEN_DERIVOB_CLASS_ID) {
		Object* pObj = ((IDerivedObject*)p)->GetObjRef();
		if (pObj) p = pObj;
		else return FALSE;
	}

	ICustAttribContainer* pContainer = p->GetCustAttribContainer();
	if (!pContainer) return FALSE;

	int idx = GetCustAttrPBlock(p, tstring(_T("Interactivity")), pBlock);
	if (idx >= 0) {
		while (idx >= 0) {
			pContainer->RemoveCustAttrib(idx);
			idx = GetCustAttrPBlock(p, tstring(_T("Interactivity")), pBlock);
		}
		return TRUE;
	}

	return FALSE;
}


//======================================================================
//======================================================================
void glTFImporter_Core::AttachAlphaModeCustAttr(Mtl *pMtl, int alphamode)
{
	IParamBlock2 *pBlock = NULL;
	if (GetCustAttrPBlock(pMtl, tstring(_T("AlphaMode")),pBlock)!=-1) return;

	Class_ID ret(0, 0);

	TSTR ComStr;
	ComStr = _T("CAT_DEF = attributes Custom_Attributes\nversion:0\n(\nParameters main rollout:params \n(\n");

	TSTR SubStr = _T("");
	SubStr.printf(_T("'ExtensionName' Type:#string Default:\"AlphaMode\"\n"));
	ComStr += SubStr;

	SubStr.printf(_T("'Alpha_Mode' Type:#integer UI:'Alpha_Mode' Default:%d\n"), alphamode + 1);
	ComStr += SubStr;

	SubStr.printf(_T(")\nRollout Params \"Alpha Mode\"\n(\n"));
	ComStr += SubStr;

	SubStr.printf(_T("dropdownlist 'Alpha_Mode' \"\" items:#(\"Opaque\",\"Mask\",\"Blend\") selection:%d Width:160 Height:16 Align:#Center Offset:[0,0]\n"), alphamode+1);
	ComStr += SubStr;

	ComStr += _T(")\n)\nCustAttributes.add (KHRglTFImporter.GetCurrentAnim()) CAT_DEF\n");

	SetCurrentAttrObj(FPValue(TYPE_MTL, pMtl));

	FPValue fpv;
	//mputs(ComStr);
#if MAX_RELEASE >= 24000
	ExecuteMAXScriptScript(ComStr, MAXScript::ScriptSource::NonEmbedded, TRUE);
	ExecuteMAXScriptScript(_T("CAT_DEF.classid"), MAXScript::ScriptSource::NonEmbedded, TRUE, &fpv);
#else
	ExecuteMAXScriptScript(ComStr, TRUE);
	ExecuteMAXScriptScript(_T("CAT_DEF.classid"), TRUE, &fpv);
#endif


#if 0
	pAnim->AllocCustAttribContainer();
	ICustAttribContainer* pContainer = pAnim->GetCustAttribContainer();

	SimpleCustAttrib* ca = new SimpleCustAttrib();
	pContainer->InsertCustAttrib(0, ca);
#endif
}
//======================================================================
//======================================================================
ReferenceTarget* glTFImporter_Core::GetAnimByUniqueID(DWORD id)
{
	INode* pNode = GetNodeByUniqueIDRec(GetCOREInterface()->GetRootNode(), id);
	if (pNode) return pNode;

	MtlBaseLib* mtlLib = GetCOREInterface()->GetSceneMtls();
	for (int i = 0; i < mtlLib->Count(); i++) {
		MtlBase* pMtl = *mtlLib->Addr(i);
		DWORD dd;
		if (GetInteractivityPointerID(pMtl, dd)) {
			if (id == dd) return pMtl;
		}

		for (int j = 0; j < pMtl->NumSubTexmaps(); j++) {
			Texmap* pTex = pMtl->GetSubTexmap(j);
			if (GetInteractivityPointerID(pTex, dd)) {
				if (id == dd) return pTex;
			}
		}
	}

	return NULL;
}

//======================================================================
//======================================================================
INode* glTFImporter_Core::GetNodeByUniqueIDRec(INode* pNode, DWORD id)
{
	if (!pNode) return NULL;

	DWORD dd;
	if (GetInteractivityPointerID(pNode, dd)) {
		if(id == dd) return pNode;
	}

	for (int i = 0; i < pNode->NumChildren(); i++) {
		INode* p = GetNodeByUniqueIDRec(pNode->GetChildNode(i), id);
		if (p) return p;
	}

	return NULL;
}

