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

//#define BUILDING_LIBCURL
#include "HSglTFImporter.h"
#include <maxscript\maxscript.h>
#include <gamma.h>
#include <Materials\\TextureOutput.h>

#include "define.h"
#include "MimeTypes.h"
//#include "define.h"

#if MAX_RELEASE >= 26000
#include "ColorManagement\IColorPipelineMgr.h"
#endif


//=============================================================================
//=============================================================================
tstring glTFImporter_Core::CreateTextureFileName(cgltf_texture* tex, tstring &originalFname)
{
	tstring fname;

	originalFname.clear();

	cgltf_image* image = NULL;
	if (tex->has_basisu) {
		image = tex->basisu_image;
	}
	else {
		image = tex->image;
	}
	if (tex->extensions_count > 0) {
		for (int i = 0; i < tex->extensions_count;i++) {
			char* name = tex->extensions[i].name;
			if (!strcmp(name, "EXT_texture_webp")) {
				char* data = tex->extensions[i].data;
				char* ptr = strchr(data, ':');
				ptr++;
				int idx = atoi(ptr);
				image = &m_glTF_data->images[idx];
				//webp_flag = TRUE;
				break;
			}else if (!strcmp(name, "KHR_texture_basisu")) {
				char* data = tex->extensions[i].data;
				char* ptr = strchr(data, ':');
				//ktx2_flag = TRUE;
			}
		}
	}

	const char* uri = image->uri;
	//int size = image->buffer_view->size;
	if (uri) {
		if (strncmp(uri, "data:", 5) == 0) {
			const char* p = strchr(uri, ';');
			char buf[MAX_PATH];
			strncpy(buf, uri + 5, p - (uri + 5));
			buf[p - (uri + 5)] = '\0';
			const char* type = MimeTypes::getExtension(buf);
			int outlength;
			p = strchr(p, ',') + 1;
			const char* endp = strchr(p, '=');
			int len1 = endp - p;
			int len2 = strlen(p);
			if (len1 <= 0) len1 = len2;
			int CharSize = (len1 < len2) ? len1 : len2; //endp - p;
			int ByteSize = (CharSize * 3) / 4;
			//if (CharSize % 4 != 0) ByteSize += 1;
			cgltf_options options = { 0 };

			void* out_data = new char[ByteSize];
			cgltf_result ret = cgltf_load_buffer_base64(&options, ByteSize, p, &out_data);

			char base_name[MAX_PATH];
			if (image->name) 	strcpy(base_name, image->name);
			else if (tex->name)	strcpy(base_name, tex->name);
			else 				sprintf(base_name, "texture_%d.%s", (UINT)m_TextureMap.size(), type);
			unsigned char* pp = (unsigned char*)base_name;
			for (int i = 0; i < strlen(base_name); i++) {
				if (!isalnum(*pp)) *pp = '_';
				pp++;
			}
			if (!strchr(base_name, '.')) {
				sprintf(buf, "%s_%d.%s", base_name, (UINT)m_TextureMap.size(), type);
				strcpy(base_name, buf);
			}
			char name[MAX_PATH];
			sprintf(name, "%s\\%s", WStringToString(m_WorkImageFolder).c_str(), base_name);

			FILE* fp = fopen(name, "wb");
			fwrite(out_data, ByteSize, 1, fp);
			fclose(fp);
			delete[] out_data;
			fname = StringToWString(name);
			m_EmbedFormat = TRUE;
		}
		else {
			//std::filesystem::path file(uri);
			fname = urlDecode(StringToWString(uri));
			fname = tstring(m_fullpath.parent_path()) + tstring(_T("\\")) + fname;
		}
	}
	else if (strlen(image->mime_type) > 1) {
		const char* type = MimeTypes::getExtension(image->mime_type);
		cgltf_buffer_view* bufferview = image->buffer_view;
		cgltf_buffer* buffer = bufferview->buffer;
		int offset = bufferview->offset;
		int size = bufferview->size;
		void* ptr = (char*)(buffer->data) + offset;

		char base_name[MAX_PATH];
		//char buf[MAX_PATH];
		if (image->name) {
			strncpy(base_name, image->name, MAX_PATH);
			char* ptr = strchr(base_name, '.');
			if (ptr) *(ptr + 1) = 0;
			strcat(base_name, type);
		}
		else if (tex->name)	strncpy(base_name, tex->name, MAX_PATH);
		else 				sprintf(base_name, "texture_%d.%s", (UINT)m_TextureMap.size(), type);

		/*
		unsigned char* pp = (unsigned char*)base_name;
		for (int i = 0; i < strlen(base_name); i++) {
			if (!isalnum(*pp)) *pp = '_';
			pp++;
		}
		*/

		tstring str = StringToWString(base_name);
		auto pos = str.rfind('\\');
		if (pos != tstring::npos) {
			str = str.substr(pos + 1, str.size());
		}

		if (str.rfind(_T(".")) == std::string::npos) {
			TCHAR buf[1000];
			_stprintf(buf, _T("%s_%d.%s"), str.c_str(), (UINT)m_TextureMap.size(), StringToWString(type).c_str());
			str = tstring(buf);
		}
		//char name[MAX_PATH];
		//sprintf(name, "%s\\%s", WStringToString(m_WorkImageFolder).c_str(), base_name);
		fname = m_WorkImageFolder + tstring(_T("\\")) + str;

		FILE* fp = _tfopen(fname.c_str(), _T("wb"));
		if (fp) {
			fwrite(ptr, size, 1, fp);
			fclose(fp);
		}
		//fname = StringToWString(name);

		m_EmbedFormat = TRUE;
	}

	if (fname.find(_T(".webp"))!=std::string::npos) {
		originalFname = fname;
		WebpDecode(originalFname, fname);
	}
	else if (fname.find(_T(".ktx2")) != std::string::npos) {
		originalFname = fname;
		KTX2ImageCreater(originalFname, fname);
	}

	return fname;
}

//=============================================================================
//=============================================================================
void glTFImporter_Core::CreateTextureTable(void)
{
	m_EmbedFormat = FALSE;
	tstring fname;
	m_TextureMap.clear();
	m_TextureViewMap.clear();

	SetTexImportStatus(0);

	int index = 0;
	
	for (int i = 0; i < m_glTF_data->textures_count; i++) {
		cgltf_texture* tex = &m_glTF_data->textures[i];
		if (!tex) continue;

		tstring originalFname;
		fname = CreateTextureFileName(tex, originalFname);
		/*
		const char *uri = image->uri;
		//int size = image->buffer_view->size;
		if (uri) {
			if (strncmp(uri, "data:", 5) == 0) {
				const char *p = strchr(uri, ';');
				char buf[MAX_PATH];
				strncpy(buf, uri+5, p-(uri + 5));
				buf[p - (uri + 5)] = '\0';
				const char *type = MimeTypes::getExtension(buf);
				int outlength;
				p = strchr(p, ',')+1;
				const char *endp = strchr(p, '=');
				int len1 = endp - p;
				int len2 = strlen(p);
				if (len1 <= 0) len1 = len2;
				int CharSize = (len1 < len2) ? len1 : len2; //endp - p;
				int ByteSize = (CharSize * 3)/4;
				//if (CharSize % 4 != 0) ByteSize += 1;
				cgltf_options options = { 0 };

				void *out_data = new char [ByteSize];
				cgltf_result ret = cgltf_load_buffer_base64(&options, ByteSize, p, &out_data);

				char base_name[MAX_PATH];
				if (image->name) 	strcpy(base_name, image->name);
				else if (tex->name)	strcpy(base_name, tex->name);
				else 				sprintf(base_name, "texture_%d.%s", (UINT)m_TextureMap.size(), type);
				unsigned char* pp = (unsigned char*)base_name;
				for (int i = 0; i < strlen(base_name); i++) {
					if (!isalnum(*pp)) *pp = '_';
					pp++;
				}
				if (!strchr(base_name, '.')) {
					sprintf(buf, "%s_%d.%s", base_name, (UINT)m_TextureMap.size(), type);
					strcpy(base_name, buf);
				}
				char name[MAX_PATH];
				sprintf(name, "%s\\%s", WStringToString(m_WorkImageFolder).c_str(), base_name);

				FILE* fp = fopen(name, "wb");
				fwrite(out_data, ByteSize, 1, fp);
				fclose(fp);
				delete[] out_data;
				fname = StringToWString(name);
				m_EmbedFormat = TRUE;
			}
			else {
				//std::filesystem::path file(uri);
				fname = urlDecode(StringToWString(uri));
				fname = tstring(m_fullpath.parent_path()) + tstring(_T("\\")) + fname;
			}
		}
		else if (strlen(image->mime_type)>1) {
			const char* type = MimeTypes::getExtension(image->mime_type);
			cgltf_buffer_view* bufferview = image->buffer_view;
			cgltf_buffer* buffer = bufferview->buffer;
			int offset = bufferview->offset;
			int size = bufferview->size;
			void* ptr = (char*)(buffer->data) + offset;

			char base_name[MAX_PATH];
			//char buf[MAX_PATH];
			if (image->name) 	strncpy(base_name, image->name, MAX_PATH);
			else if (tex->name)	strncpy(base_name, tex->name, MAX_PATH);
			else 				sprintf(base_name, "texture_%d.%s", (UINT)m_TextureMap.size(), type);


			tstring str = StringToWString(base_name);
			auto pos = str.rfind('\\');
			if (pos != tstring::npos) {
				str = str.substr(pos+1, str.size());
			}

			if (str.rfind(_T("."))== std::string::npos) {
				TCHAR buf[1000];
				_stprintf(buf, _T("%s_%d.%s"), str.c_str(), (UINT)m_TextureMap.size(), StringToWString(type).c_str());
				str = tstring(buf);
			}
			//char name[MAX_PATH];
			//sprintf(name, "%s\\%s", WStringToString(m_WorkImageFolder).c_str(), base_name);
			fname = m_WorkImageFolder + tstring(_T("\\")) + str;

			FILE* fp = _tfopen(fname.c_str(), _T("wb"));
			if (fp) {
				fwrite(ptr, size, 1, fp);
				fclose(fp);
			}
			//fname = StringToWString(name);

			m_EmbedFormat = TRUE;
		}
		*/
		/*
		if (tex->has_basisu) {
			tstring retname;
			KTX2ImageCreater(fname, retname);
			fname = retname;
		}
		*/

		BitmapTex* pBmpTex = NewDefaultBitmapTex();
		pBmpTex->GetUVGen()->SetCoordMapping(UVMAP_SCREEN_ENV);
		pBmpTex->GetUVGen()->SetTextureTiling(U_WRAP | V_WRAP);
		pBmpTex->GetUVGen()->InitSlotType(MAPSLOT_TEXTURE);
		pBmpTex->SetMapName(fname.c_str());
		pBmpTex->SetMtlFlag(MTL_TEX_DISPLAY_ENABLED, TRUE);
		pBmpTex->ActivateTexDisplay(TRUE);

#if MAX_RELEASE > 27000
		/* {
			MaxSDK::ColorManagement::IColorPipelineMgr* pColMgr = (MaxSDK::ColorManagement::IColorPipelineMgr*)GetCOREInterface(COLORPIPELINEMGR_INTERFACE);
			MaxSDK::ColorManagement::ColorPipelineMode pp = pColMgr->GetColorPipelineMode();
			BitmapInfo bi = pBmpTex->GetBitmap(0)->GetBitmapInfo();
			MaxSDK::ColorManagement::ColSpaceStatus sss = bi.SetRequestedColorSpace(_T("Raw"), MaxSDK::ColorManagement::ColSpaceSource::User);
			pBmpTex->SetBitmapInfo(bi);
		}*/
#endif
		if (originalFname.find(_T(".webp")) != std::string::npos) {
			CreateWebpEncodingAttr(pBmpTex, originalFname, originalFname.size() > 0);
		}
		else if (originalFname.find(_T(".ktx2")) != std::string::npos) {
			CreateKTX2EncodingAttr(pBmpTex, originalFname, originalFname.size() > 0);
		}

		m_TextureMap.insert(std::make_pair(tex, pBmpTex));
		SetTexImportStatus(i+1);
	}

	SetTexImportStatus(-1);
}

//=============================================================================
//=============================================================================
BitmapTex *glTFImporter_Core::GetBitmapTexFromglTexture(cgltf_texture *tex)
{
	if (!tex) return NULL;

	if (tex->has_basisu) {
		cgltf_image *image = tex->basisu_image;
	}

	if (m_UniqueTexture) {
		BitmapTex *pTex = m_TextureMap[tex];
		RemapDir *pRemap = NewRemapDir();
		BitmapTex *pNewTex = (BitmapTex*)pTex->Clone(*pRemap);
		pRemap->DeleteThis();
		return pNewTex;
	}
	else {
		return m_TextureMap[tex];
	}
}

static float s_gamma = 1.0f;

//=============================================================================
//=============================================================================
BitmapTex* glTFImporter_Core::SplitOcclusionTexture(BitmapTex *pOrgTexBmp)
{
	gammaMgr.SetFileOutGamma(1.0f);

	Bitmap *pOriginalBmp = pOrgTexBmp->GetBitmap(0);
	if (!pOriginalBmp) return NULL;

	SetPNGInfo(m_pPNG_BmpIO, pOriginalBmp);

	BitmapInfo orgbi = pOriginalBmp->GetBitmapInfo();
	//std::filesystem::path fname = orgbi.Filename();
	std::filesystem::path fname = pOrgTexBmp->GetMapName();
	if (!fname.has_parent_path()) {
		fname = m_SourceImageFolder + orgbi.Filename();
	}

	tstring texFilePath = tstring(m_WorkImageFolder) + tstring(fname.stem()) + tstring(_T("_R")) + tstring(fname.extension());
	CopyFile(fname.c_str(), texFilePath.c_str(), FALSE);

	BitmapTex *pBmpTex = NewDefaultBitmapTex();
	pBmpTex->SetMapName(texFilePath.c_str());

	BitmapInfo bi(orgbi);
	bi.SetPath(texFilePath.c_str());
	bi.ResetCustomFlag(BMM_CUSTOM_GAMMA);

	//bi.SetGamma(s_gamma);
	BMMRES status;
	Bitmap *pBitmap = TheManager->Load(&bi, &status);
	for (int w = 0; w < bi.Width(); w++) {
		for (int h = 0; h < bi.Height(); h++) {
			BMM_Color_64 buff;
			//pBitmap->GetLinearPixels(w, h, 1, &buff);
			pBitmap->GetPixels(w, h, 1, &buff);
			buff.g = buff.r;
			buff.b = buff.r;
			buff.a = 0;
			pBitmap->PutPixels(w, h, 1, &buff);
		}
	}

	pBitmap->OpenOutput(&bi);
	pBitmap->Write(&bi);
	pBitmap->Close(&bi);
	pBitmap->DeleteThis();

	pBmpTex->GetUVGen()->SetCoordMapping(UVMAP_SCREEN_ENV);
	pBmpTex->GetUVGen()->SetTextureTiling(0);
	pBmpTex->GetUVGen()->InitSlotType(MAPSLOT_TEXTURE);

	int mapCh = pOrgTexBmp->GetTheUVGen()->GetMapChannel();
	pBmpTex->GetUVGen()->SetMapChannel(mapCh);

	pBmpTex->ReloadBitmapAndUpdate();

	gammaMgr.SetFileOutGamma(2.2f);

	return pBmpTex;
}

//=============================================================================
//=============================================================================
BitmapTex* glTFImporter_Core::SplitRoughnessTexture(BitmapTex *pOrgTexBmp)
{
	gammaMgr.SetFileOutGamma(1.0f);

	Bitmap *pOriginalBmp = pOrgTexBmp->GetBitmap(0);
	if (!pOriginalBmp) return NULL;

	SetPNGInfo(m_pPNG_BmpIO, pOriginalBmp);

	BitmapInfo orgbi = pOriginalBmp->GetBitmapInfo();
	//std::filesystem::path fname = orgbi.Filename();
	std::filesystem::path fname = pOrgTexBmp->GetMapName();
	if (!fname.has_parent_path()) {
		fname = m_SourceImageFolder + orgbi.Filename();
	}

	tstring texFilePath = tstring(m_WorkImageFolder) + tstring(fname.stem()) + tstring(_T("_G")) + tstring(fname.extension());
	CopyFile(fname.c_str(), texFilePath.c_str(), FALSE);

	BitmapTex *pBmpTex = NewDefaultBitmapTex();
	pBmpTex->SetMapName(texFilePath.c_str());

	BitmapInfo bi(orgbi);
	bi.SetPath(texFilePath.c_str());
	//bi.SetGamma(s_gamma);
	BMMRES status;
	Bitmap *pBitmap = TheManager->Load(&bi, &status);
	//pBmpTex->ReloadBitmapAndUpdate();
	for (int w = 0; w < bi.Width(); w++) {
		for (int h = 0; h < bi.Height(); h++) {
			BMM_Color_64 buff;
			pBitmap->GetLinearPixels(w, h, 1, &buff);
			//pBitmap->GetPixels(w, h, 1, &buff);
	/*
			{
				pBitmap->GetPixels(w, h, 1, &buff);
				COLORREF col = RGB(buff.r, buff.g, buff.b);
				COLORREF c = gammaMgr.DisplayGammaCorrect(col);
				buff.g = GetGValue(c);
			}
	*/
			buff.r = buff.g;
			buff.b = buff.g;
			buff.a = 0;
			//buff.r = buff.b = buff.g;
			pBitmap->PutPixels(w, h, 1, &buff);
		}
	}
	pBitmap->OpenOutput(&bi);
	pBitmap->Write(&bi);
	pBitmap->Close(&bi);
	pBitmap->DeleteThis();

	pBmpTex->GetUVGen()->SetCoordMapping(UVMAP_SCREEN_ENV);
	pBmpTex->GetUVGen()->SetTextureTiling(U_WRAP | V_WRAP);
	pBmpTex->GetUVGen()->InitSlotType(MAPSLOT_TEXTURE);

	int mapCh = pOrgTexBmp->GetTheUVGen()->GetMapChannel();
	pBmpTex->GetUVGen()->SetMapChannel(mapCh);

	pBmpTex->ReloadBitmapAndUpdate();

	if (m_CorrectGamma)
		CorrectBitmapGamma(pBmpTex, m_GammaValue);
//	else
//		CorrectBitmapGamma(pBmpTex, 2.2f, FALSE);

	gammaMgr.SetFileOutGamma(2.2f);

	return pBmpTex;
}

//=============================================================================
//=============================================================================
BitmapTex* glTFImporter_Core::SplitMetalnessTexture(BitmapTex *pOrgTexBmp)
{
	gammaMgr.SetFileOutGamma(1.0f);

	Bitmap *pOriginalBmp = pOrgTexBmp->GetBitmap(0);
	if (!pOriginalBmp) return NULL;

	SetPNGInfo(m_pPNG_BmpIO, pOriginalBmp);

	BitmapInfo orgbi = pOriginalBmp->GetBitmapInfo();
	orgbi.GetDeviceFlags();
	//std::filesystem::path fname = orgbi.Filename();
	std::filesystem::path fname = pOrgTexBmp->GetMapName();
	if (!fname.has_parent_path()) {
		fname = m_SourceImageFolder + orgbi.Filename();
	}

	tstring texFilePath = tstring(m_WorkImageFolder) + tstring(fname.stem()) + tstring(_T("_B")) + tstring(fname.extension());
	CopyFile(fname.c_str(), texFilePath.c_str(), FALSE);

	BitmapTex *pBmpTex = NewDefaultBitmapTex();
	pBmpTex->SetMapName(texFilePath.c_str());

	BitmapInfo bi(orgbi);
	bi.SetPath(texFilePath.c_str());
	bi.SetGamma(s_gamma);
	BMMRES status;
	Bitmap *pBitmap = TheManager->Load(&bi, &status);
	pBmpTex->ReloadBitmapAndUpdate();
	for (int w = 0; w < bi.Width(); w++) {
		for (int h = 0; h < bi.Height(); h++) {
			BMM_Color_64 buff;
			pBitmap->GetLinearPixels(w, h, 1, &buff);
			//pBitmap->GetPixels(w, h, 1, &buff);
			buff.r = buff.b;
			buff.g = buff.b;
			pBitmap->PutPixels(w, h, 1, &buff);
		}
	}
	pBitmap->OpenOutput(&bi);
	pBitmap->Write(&bi);
	pBitmap->Close(&bi);
	pBitmap->DeleteThis();

	pBmpTex->GetUVGen()->SetCoordMapping(UVMAP_SCREEN_ENV);
	pBmpTex->GetUVGen()->SetTextureTiling(U_WRAP | V_WRAP);
	pBmpTex->GetUVGen()->InitSlotType(MAPSLOT_TEXTURE);

	int mapCh = pOrgTexBmp->GetTheUVGen()->GetMapChannel();
	pBmpTex->GetUVGen()->SetMapChannel(mapCh);

	pBmpTex->ReloadBitmapAndUpdate();

	if (m_CorrectGamma)
		CorrectBitmapGamma(pBmpTex, m_GammaValue);
//	else
//		CorrectBitmapGamma(pBmpTex, 2.2f, FALSE);

	gammaMgr.SetFileOutGamma(2.2f);

	return pBmpTex;
}

//=============================================================================
//=============================================================================
Texmap* glTFImporter_Core::SetMetalRoughOccClrCorrectMap(Texmap* pOrgTexBmp, Texmap** pMetalTex, Texmap** pRoughTex, Texmap** pOccTex, BOOL Metallic, BOOL Roughness, BOOL Occlusion)
{
#define	clrCrctMap	1
#define	rewireMode	2
#define	rewireR		3
#define	rewireG		4
#define	rewireB		5
#define	rewireA		6

//	if (m_CorrectGamma)
//		CorrectBitmapGamma(pOrgTexBmp, m_GammaValue);
//	else
//		CorrectBitmapGamma(pOrgTexBmp, 2.2f, FALSE);

	if (Metallic) *pMetalTex = NULL;
	if (Roughness) *pRoughTex = NULL;
	if (Occlusion) *pOccTex = NULL;

//	TSTR name = pOrgTexBmp->GetMapName();
	//Texmap* pTex = CreateMetalRoughOccOSLNode(pOrgTexBmp, 0);
	//IParamBlock2* pBlock = pTex->GetParamBlock(1);
	//pBlock->SetValue(0, m_time, name);

	if (Metallic)
	{
		*pMetalTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, ColorCorrectTexID);
		IParamBlock2* pBlock = (*pMetalTex)->GetParamBlock(0);
		pBlock->SetValue(clrCrctMap, m_time, pOrgTexBmp);
		pBlock->SetValue(rewireMode, m_time, 3);
		pBlock->SetValue(rewireR, m_time, 2);
		pBlock->SetValue(rewireG, m_time, 2);
		pBlock->SetValue(rewireB, m_time, 2);
	}
	if (Roughness)
	{
		*pRoughTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, ColorCorrectTexID);
		IParamBlock2* pBlock = (*pRoughTex)->GetParamBlock(0);
		pBlock->SetValue(clrCrctMap, m_time, pOrgTexBmp);
		pBlock->SetValue(rewireMode, m_time, 3);
		pBlock->SetValue(rewireR, m_time, 1);
		pBlock->SetValue(rewireG, m_time, 1);
		pBlock->SetValue(rewireB, m_time, 1);
	}
	if (Occlusion)
	{
		*pOccTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, ColorCorrectTexID);
		IParamBlock2* pBlock = (*pOccTex)->GetParamBlock(0);
		pBlock->SetValue(clrCrctMap, m_time, pOrgTexBmp);
		pBlock->SetValue(rewireMode, m_time, 3);
		pBlock->SetValue(rewireR, m_time, 0);
		pBlock->SetValue(rewireG, m_time, 0);
		pBlock->SetValue(rewireB, m_time, 0);
	}

	return pOrgTexBmp;
}

//=============================================================================
//=============================================================================
Texmap* glTFImporter_Core::SetAlphaClrCorrectMap(Texmap* pOrgTexBmp, Texmap** pAlphaTex)
{
#define	clrCrctMap	1
#define	rewireMode	2
#define	rewireR		3
#define	rewireG		4
#define	rewireB		5
#define	rewireA		6

	*pAlphaTex = NULL;

	*pAlphaTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, ColorCorrectTexID);
	IParamBlock2* pBlock = (*pAlphaTex)->GetParamBlock(0);
	pBlock->SetValue(clrCrctMap, m_time, pOrgTexBmp);
	pBlock->SetValue(rewireMode, m_time, 3);
	pBlock->SetValue(rewireR, m_time, 3);
	pBlock->SetValue(rewireG, m_time, 3);
	pBlock->SetValue(rewireB, m_time, 3);

	return pOrgTexBmp;
}

//=============================================================================
//=============================================================================
#if 1
Texmap* glTFImporter_Core::SetMetalRoughOccOSLMap(BitmapTex* pOrgTexBmp, Texmap** pMetalTex, Texmap** pRoughTex, Texmap** pOccTex, BOOL Metallic, BOOL Roughness, BOOL Occlusion, cgltf_texture_view *texview)
{
#define	sourceMap			0
#define	outputChannelIndex	1

	* pMetalTex = NULL;
	*pRoughTex = NULL;
	*pOccTex = NULL;

	int mapCh = 1;
	cgltf_texture_transform *transform = NULL;
	if (texview) {
		if (texview->has_transform) transform = &texview->transform;
		mapCh = texview->texcoord + 1;
	}

	TSTR name = pOrgTexBmp->GetMapName();
	Texmap* pTex = NULL;
	if (TRUE) {	// transform -> TRUE
		pTex = CreateUberBitmapOSLNode(name);
		IParamBlock2 *pBlock1 = pTex->GetParamBlock(1);
		Point3 offset(0.0f, 0.0f, 0.0f);
		Point3 tiling(1.0f, 1.0f, 0.0f);
		float rot = 0.0f;
		if (transform) {
			offset.x = transform->offset[0];
			offset.y = transform->offset[1];
			tiling.x = transform->scale[0];
			tiling.y = transform->scale[1];
			rot = transform->rotation;
		}
		pBlock1->SetValue(UberBmp_Offset, m_time, offset);
		pBlock1->SetValue(UberBmp_Rotate, m_time, rot);
		pBlock1->SetValue(UberBmp_Tiling, m_time, tiling);
		pBlock1->SetValue(UberBmp_UVSet, m_time, mapCh);

		if (transform->has_texcoord) {
		}

		if (Metallic)
		{
			*pMetalTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, MULTIOUTPUTTOTEXMAP_CLASS_ID);
			IParamBlock2* pBlock = (*pMetalTex)->GetParamBlock(0);
			pBlock->SetValue(sourceMap, m_time, pTex);
			pBlock->SetValue(outputChannelIndex, m_time, 3);
		}
		if (Roughness)
		{
			*pRoughTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, MULTIOUTPUTTOTEXMAP_CLASS_ID);
			IParamBlock2* pBlock = (*pRoughTex)->GetParamBlock(0);
			pBlock->SetValue(sourceMap, m_time, pTex);
			pBlock->SetValue(outputChannelIndex, m_time, 2);
		}
		if (Occlusion)
		{
			*pOccTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, MULTIOUTPUTTOTEXMAP_CLASS_ID);
			IParamBlock2* pBlock = (*pOccTex)->GetParamBlock(0);
			pBlock->SetValue(sourceMap, m_time, pTex);
			pBlock->SetValue(outputChannelIndex, m_time, 1);
		}
	}
	else {
		pTex = CreateBitmapLookupOSLNode(name);
		//IParamBlock2* pBlock = pTex->GetParamBlock(1);
		//pBlock->SetValue(0, m_time, name);

		if (Metallic)
		{
			*pMetalTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, MULTIOUTPUTTOTEXMAP_CLASS_ID);
			IParamBlock2* pBlock = (*pMetalTex)->GetParamBlock(0);
			pBlock->SetValue(sourceMap, m_time, pTex);
			pBlock->SetValue(outputChannelIndex, m_time, 3);
		}
		if (Roughness)
		{
			*pRoughTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, MULTIOUTPUTTOTEXMAP_CLASS_ID);
			IParamBlock2* pBlock = (*pRoughTex)->GetParamBlock(0);
			pBlock->SetValue(sourceMap, m_time, pTex);
			pBlock->SetValue(outputChannelIndex, m_time, 2);
		}
		if (Occlusion)
		{
			*pOccTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, MULTIOUTPUTTOTEXMAP_CLASS_ID);
			IParamBlock2* pBlock = (*pOccTex)->GetParamBlock(0);
			pBlock->SetValue(sourceMap, m_time, pTex);
			pBlock->SetValue(outputChannelIndex, m_time, 1);
		}
	}

	IParamBlock2* pBlock = NULL;
	GetCustAttrPBlock(pOrgTexBmp, tstring(_T("Webp Encode")), pBlock);
	if (pBlock) {
		tstring str = pBlock->GetStr(2, m_time);
		CreateWebpEncodingAttr(pTex, str, str.size() > 0);
	}

	return pTex;
}

#else
Texmap* glTFImporter_Core::SetMetalRoughOccOSLMap(BitmapTex *pOrgTexBmp, Texmap **pMetalTex, Texmap **pRoughTex, Texmap **pOccTex, BOOL Metallic, BOOL Roughness, BOOL Occlusion)
{
#define	sourceMap			0
#define	outputChannelIndex	1

	*pMetalTex = NULL;
	*pRoughTex = NULL;
	*pOccTex = NULL;

	TSTR name = pOrgTexBmp->GetMapName();
	Texmap *pTex = CreateMetalRoughOccOSLNode(pOrgTexBmp,0);
	IParamBlock2 *pBlock = pTex->GetParamBlock(1);
	pBlock->SetValue(0, m_time, name);

	if (Metallic)
	{
		*pMetalTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, MULTIOUTPUTTOTEXMAP_CLASS_ID);
		IParamBlock2 *pBlock = (*pMetalTex)->GetParamBlock(0);
		pBlock->SetValue(sourceMap, m_time, pTex);
		pBlock->SetValue(outputChannelIndex, m_time, 1);
	}
	if (Roughness)
	{
		*pRoughTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, MULTIOUTPUTTOTEXMAP_CLASS_ID);
		IParamBlock2 *pBlock = (*pRoughTex)->GetParamBlock(0);
		pBlock->SetValue(sourceMap, m_time, pTex);
		pBlock->SetValue(outputChannelIndex, m_time, 0);
	}
	if (Occlusion)
	{
		*pOccTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, MULTIOUTPUTTOTEXMAP_CLASS_ID);
		IParamBlock2 *pBlock = (*pOccTex)->GetParamBlock(0);
		pBlock->SetValue(sourceMap, m_time, pTex);
		pBlock->SetValue(outputChannelIndex, m_time, 2);
	}

	return pTex;
}
#endif
//=============================================================================
//=============================================================================
int GetColorDepth(Bitmap *pBitmap)
{
	int type;
	void *pBfffer = pBitmap->GetStoragePtr(&type);
	int depth = 32;
	switch (type) {
	case BMM_BMP_4:		depth = 4; break;
	case BMM_TRUE_16:	depth = 16; break;
	case BMM_TRUE_24:	depth = 24; break;
	case BMM_TRUE_32:	depth = 32; break;
	case BMM_TRUE_64:	depth = 64; break;
	case BMM_LINE_ART:	depth = 1; break;
	case BMM_PALETTED:	depth = 8; break;
	}

	return depth;
}
//=============================================================================
//=============================================================================
void SetPNGInfo(IBitmapIO_Png *pPNG_BmpIO, Bitmap *pBitmap)
{
	if (!pPNG_BmpIO | !pBitmap) return;

	int type;
	pBitmap->GetStoragePtr(&type);
	pPNG_BmpIO->SetType(type);
	pPNG_BmpIO->SetAlpha(pBitmap->HasAlpha());
}

//=============================================================================
//=============================================================================
Texmap *CreateBaseColorMap(void)
{

	TSTR ComStr;
	ComStr.printf(_T("tex = CompositeMap(); tex.mapEnabled.count = 2; tex"));
	FPValue fpv;
#if MAX_RELEASE >= 24000
	ExecuteMAXScriptScript(ComStr, MAXScript::ScriptSource::NonEmbedded, FALSE, &fpv);
#else
	ExecuteMAXScriptScript(ComStr, FALSE, &fpv);
#endif
	Texmap *pTex = fpv.tex;

	return pTex;
}
//=============================================================================
//=============================================================================
Texmap *CreateColorMap(AColor &c)
{
#define solidcolor 0
#define mapEnabled 2

	Texmap *pTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, ColorMapTexID);
	Control* pC = (Control*)GetCOREInterface()->CreateInstance(CTRL_POINT4_CLASS_ID, Class_ID(0x2012, 0x0));
	pTex->GetParamBlock(0)->SetControllerByIndex(solidcolor, 0, pC);

	pTex->GetParamBlock(0)->SetValue(solidcolor, 0, c);
	pTex->GetParamBlock(0)->SetValue(mapEnabled, 0, 0);

	return pTex;
}

//=============================================================================
//=============================================================================
cgltf_texture *glTFImporter_Core::GetglTFTexByTexmap(Texmap* pTex)
{
	for (auto it : m_TextureMap) {
		if (it.second == pTex) return it.first;
	}

	return NULL;
}


Point2 ApplyGltfTextureTransform(Texmap* pBmpTex, const cgltf_texture_view* textview, TimeValue t)
{
	Point2 offset(0.0f, 0.0f);
	if (!pBmpTex || !textview || !textview->texture) return offset;

	StdUVGen* pUVGen = GetUVGen(pBmpTex);
	if (!pUVGen) return offset;

	// UVセット番号（Map Channel は +1）
	pUVGen->SetMapChannel(textview->texcoord + 1);

	// サンプラの tiling 設定
	UINT tiling = 0;
	cgltf_sampler* pSampler = textview->texture->sampler;
	if (pSampler) {
		if (pSampler->wrap_s == 10497) tiling |= U_WRAP;
		if (pSampler->wrap_t == 10497) tiling |= V_WRAP;
		if (pSampler->wrap_s == 33648) tiling |= U_WRAP | U_MIRROR;
		if (pSampler->wrap_t == 33648) tiling |= V_WRAP | V_MIRROR;
		pUVGen->SetTextureTiling(tiling);
	}

	// Transformが存在しない or 無効化されている場合
	if (!textview->has_transform) return offset;

	// glTF transform情報
	float sclU = textview->transform.scale[0];
	float sclV = textview->transform.scale[1];
	float rot = textview->transform.rotation;
	offset.x = textview->transform.offset[0];
	offset.y = textview->transform.offset[1];

	// glTFは(0.5, 0.5)回転、3ds Maxは(0.0, 0.0)回転のため、オフセット補正
	float pivotU = 0.5f;
	float pivotV = 0.5f;

	float cosR = cosf(rot);
	float sinR = sinf(rot);

	float dU = pivotU - (pivotU * cosR - pivotV * sinR);
	float dV = pivotV - (pivotU * sinR + pivotV * cosR);

	offset.x += dU;
	offset.y += dV;

	// スケールが1未満のときに中心がずれるので補正
	if (sclU != 0.0f) offset.x = offset.x / sclU;
	if (sclV != 0.0f) offset.y = offset.y / sclV;

	// UVGenに設定
	pUVGen->SetUOffs(offset.x, t);
	pUVGen->SetVOffs(offset.y, t);
	pUVGen->SetUScl(sclU, t);
	pUVGen->SetVScl(sclV, t);
	pUVGen->SetWAng(rot, t); // 3ds MaxのWAngはラジアン単位でOK

	if (pBmpTex->ClassID() == bmptexClassID) {
		((BitmapTex*)pBmpTex)->ReloadBitmapAndUpdate();
	}

	return offset;
}

#if 1	// オリジナル
//=============================================================================
//=============================================================================
Point2 glTFImporter_Core::SetTextureUVoffset(Texmap *pBmpTex, cgltf_texture_view *textview)
{
	//return ApplyGltfTextureTransform(pBmpTex, textview, m_time);

	Point2 offset(0.0f, 0.0f);
	if (!pBmpTex) return offset;
	if (!textview) return offset;

	StdUVGen* pUVGen = GetUVGen(pBmpTex);
	if (pUVGen) pUVGen->SetMapChannel(textview->texcoord + 1);

	UINT Tiling = 0;
	cgltf_sampler* pSampler = textview->texture->sampler;
	if (pSampler) {
		if (pSampler->wrap_s == 10497) Tiling += U_WRAP;
		if (pSampler->wrap_t == 10497) Tiling += V_WRAP;
		if (pSampler->wrap_s == 33648) Tiling += U_WRAP + U_MIRROR;
		if (pSampler->wrap_t == 33648) Tiling += V_WRAP + V_MIRROR;
		if (pUVGen) {
			pUVGen->SetTextureTiling(Tiling);
			int mapCh = textview->texcoord + 1;
			pUVGen->SetMapChannel(mapCh);
		}
	}

	if (!textview->has_transform) return offset;
	if(m_Quantization) return offset;

	m_TextureViewMap.insert(std::make_pair(pBmpTex, textview));

	offset.x = textview->transform.offset[0];
	offset.y = textview->transform.offset[1];
	float sclU = textview->transform.scale[0];
	float sclV = textview->transform.scale[1];
	float rot = textview->transform.rotation;

	float localoffsetU = -0.5f * cos(rot) + 0.5f * sin(rot) + 0.5f;
	float localoffsetV = -0.5f * sin(rot) - 0.5f * cos(rot) + 0.5f;
	if (sclU == 0.0f) {
	}
	else if (sclU >= 1.0f) {
		localoffsetU += (1.0f - (1.0f / sclU)) / 2.0f;
		offset.x = -offset.x - localoffsetU;
	}
	else if (sclU < 1.0f) {
		offset.x = (1.0f / sclU - 1.0f) * 0.5f + (1.0f - offset.x) / sclU;
	}

	if (sclV == 0.0f) {
	}
	else if (sclV >= 1.0f) {
		localoffsetV += (1.0f - (1.0f / sclV)) / 2.0f;
		offset.y = offset.y + localoffsetV;
	}
	else if (sclV < 1.0f) {
		offset.y = (offset.y) / sclV-(1.0f / sclV - 1.0f) / 2.0f;
	}

	if (pUVGen) {
		pUVGen->SetUOffs(offset.x, m_time);
		pUVGen->SetVOffs(offset.y, m_time);
		pUVGen->SetUScl(sclU, m_time);
		pUVGen->SetVScl(sclV, m_time);
		pUVGen->SetWAng(rot, m_time);
	}
	if (pBmpTex->ClassID() == bmptexClassID) {
		((BitmapTex*)pBmpTex)->ReloadBitmapAndUpdate();
	}
	return offset;
}

#else // Geminio版
//=============================================================================
//=============================================================================
Point2 glTFImporter_Core::SetTextureUVoffset(Texmap* pBmpTex, cgltf_texture_view* textview)
{
	Point2 p2(0.0f, 0.0f);
	if (textview && textview->has_transform)
	{
		float scaleU = textview->transform.scale[0];
		float scaleV = textview->transform.scale[1];
		float rot = textview->transform.rotation;
		float offU = textview->transform.offset[0];
		float offV = textview->transform.offset[1];

		if (pBmpTex) {
			StdUVGen* uvGen = GetUVGen(pBmpTex);
			if (uvGen) {
				TimeValue t = GetCOREInterface()->GetTime();
				cgltf_sampler* pSampler = textview->texture->sampler;
				if (pSampler) {
					UINT Tiling = 0;

					if (pSampler->wrap_s == 10497) Tiling += U_WRAP;
					if (pSampler->wrap_t == 10497) Tiling += V_WRAP;
					if (pSampler->wrap_s == 33648) Tiling += U_WRAP + U_MIRROR;
					if (pSampler->wrap_t == 33648) Tiling += V_WRAP + V_MIRROR;
					uvGen->SetTextureTiling(Tiling);
					int mapCh = textview->texcoord + 1;
					uvGen->SetMapChannel(mapCh);
				}

				// 1. タイリング（スケール）の設定
				uvGen->SetUScl(scaleU, t);
				uvGen->SetVScl(scaleV, t);

				// 2. 回転の設定
				// glTFは原点(0,0)中心の反時計回り。
				// MaxのWAngも同様だが、座標系が上下反転しているため、
				// 回転方向や中心の補正が必要な場合があります。
				uvGen->SetWAng(rot, t);

				// 3. オフセットの計算
				// MaxのUVGenでは、Offset値は「タイリング後の1単位」として扱われるため
				// 回転がない場合は以下の式が標準的です。
				p2.x = offU;
				// V方向：glTFの「上端からのオフセット」を、Maxの「下端からのオフセット」に変換
				// さらにタイリング（高さ）分を考慮して、開始点を下端基準へ。
				p2.y = 1.0f - scaleV - offV;
				uvGen->SetUOffs(p2.x, t);
				uvGen->SetVOffs(p2.y, t);

				// 4. 重要：座標系の原点設定
				// glTFの仕様に合わせるため、回転・スケールの中心を(0,0)にする必要がある場合、
				// 以下のフラグを確認してください（必要に応じてコメント解除）
				// uvGen->SetFlag(U_OFFSET, 0); 
			}
		}
	}
	return p2;
}
#endif

//=============================================================================
//=============================================================================
void glTFImporter_Core::RescaleUVOffset(Mtl *pMtl, Box2D &rect)
{
	return;

#ifndef _DEBUG
#endif

	Point2 CropSize;
	CropSize.x = rect.max.x - rect.min.x;
	CropSize.y = rect.max.y- rect.min.y;

	if (CropSize.x == 1.0f && CropSize.y == 1.0f) return;
	if (CropSize.x == 0.0f && CropSize.y == 0.0f) return;

	float cropRateU = 1.0f / CropSize.x;
	float cropRateV = 1.0f / CropSize.y;

	int texnum = pMtl->NumSubTexmaps();
	for (int i = 0; i < texnum; i++) {
		Texmap* pTex = pMtl->GetSubTexmap(i);
		if (!pTex) continue;

		//cgltf_texture* tex = GetglTFTexByTexmap(pTex);
		//if (!tex) return;

		cgltf_texture_view* textview = m_TextureViewMap[pTex];
		if (!textview) return;

		Point2 offset(0.0f, 0.0f);
		offset.x = textview->transform.offset[0];
		offset.y = textview->transform.offset[1];
		float sclU = textview->transform.scale[0];
		float sclV = textview->transform.scale[1];
		float rot = textview->transform.rotation;

		float localoffsetU = -0.5f * cos(rot) + 0.5f * sin(rot) + 0.5f;
		float localoffsetV = -0.5f * sin(rot) - 0.5f * cos(rot) + 0.5f;
		localoffsetU += (1.0f / cropRateU - (1.0f / sclU)) / 2.0f;
		localoffsetV += (1.0f / cropRateV - (1.0f / sclV)) / 2.0f;
		offset.x = -offset.x - localoffsetU;
		offset.y = offset.y + localoffsetV;

		Point2 newOffset(0.0f, 0.0f);
		newOffset.x =  offset.x;
		newOffset.y =  offset.y;

		{
			float rotoffsetU = offset.x * cos(rot) + offset.y * sin(rot);
			float rotoffsetV = offset.x * sin(rot) - offset.y * cos(rot);
			Point2 newOffset2(0.0f, 0.0f);
			newOffset2.x = (rotoffsetU - CropSize.x / 2.0f) / sclU;
			newOffset2.y = (rotoffsetV - CropSize.y / 2.0f) / sclV;
		}

		StdUVGen* pUVGen = GetUVGen(pTex);
		if(pUVGen){
			pUVGen->SetUOffs(newOffset.x, m_time);
			pUVGen->SetVOffs(newOffset.y, m_time);
		}
	}
}

//=============================================================================
//=============================================================================
void glTFImporter_Core::CorrectBitmapGamma(BitmapTex*& pBmpTex, float gamma, BOOL custom)
{
	if (!pBmpTex) return;

	if (pBmpTex->ClassID() == VRayBitmapID) {
	}
	else {
		IParamBlock2* pb2 = pBmpTex->GetParamBlock(0);
		// get the bitmap parameter
		int n = pb2->GetDesc()->NameToIndex(_T("bitmap"));
		ParamID id = pb2->GetDesc()->IndextoID(n);
		PBBitmap* pbBitmap = pb2->GetBitmap(id);
		if (custom) {
#if MAX_RELEASE >= 26000
			if (gamma == 1.0f) {

				Bitmap* pBmp = pBmpTex->GetBitmap(0);
				BitmapInfo* bi = &pBmp->GetBitmapInfo();

				{
					//auto cpm = MaxSDK::ColorManagement::IColorPipelineMgr::GetInstance();
					MaxSDK::ColorManagement::IColorPipelineMgr* cpm = (MaxSDK::ColorManagement::IColorPipelineMgr*)GetCOREInterface(COLORPIPELINEMGR_INTERFACE);

					auto settings = cpm->Settings();
					if (settings->IsOCIOBased())
					{
						BitmapInfo bmi(*bi);
						auto ret = bmi.SetRequestedColorSpace(settings->GetDataColorSpaceName(), MaxSDK::ColorManagement::ColSpaceSource::User);
						bmi.SetName(pBmpTex->GetMapName());
						bmi.ResetCustomFlag(BMM_CUSTOM_FILEGAMMA);
						bmi.SetCustomFlag(BMM_CUSTOM_GAMMA);
						bmi.SetCustomGamma(gamma);
						pBmpTex->SetBitmapInfo(bmi);
					}

				}
			}
#else
			pbBitmap->bi.ResetCustomFlag(BMM_CUSTOM_FILEGAMMA);
			pbBitmap->bi.SetCustomFlag(BMM_CUSTOM_GAMMA);
			pbBitmap->bi.SetCustomGamma(gamma);
#endif
		}
		else {
			pbBitmap->bi.ResetCustomFlag(BMM_CUSTOM_GAMMA);
		}
		// now reload the bitmap from the disk.
		pBmpTex->ReloadBitmapAndUpdate();
	}
}

//=============================================================================
//=============================================================================
StdUVGen* GetUVGen(Texmap* pTex)
{
	if (!pTex) return NULL;

	StdUVGen* pUVGen = NULL;
	if (pTex->ClassID() == bmptexClassID) {
		pUVGen = ((BitmapTex*)pTex)->GetUVGen();
	}
	else if (pTex->ClassID() == VRayBitmapID) {
		for (int i = 0; i < pTex->NumSubs(); i++) {
			Animatable* pAnim = pTex->SubAnim(i);
			if (pAnim->ClassID() == Class_ID(0x100, 0)) return (StdUVGen*)pAnim;
		}
	}
	else if (pTex->ClassID() == CoronaBitmapID) {
		for (int i = 0; i < pTex->NumSubs(); i++) {
			Animatable* pAnim = pTex->SubAnim(i);
			if (pAnim->ClassID() == Class_ID(0x100, 0)) return (StdUVGen*)pAnim;
		}
	}
	return pUVGen;
}

//=============================================================================
//=============================================================================
Texmap* CreateAlphaFilterMap(Texmap *pTarget, AColor col)
{
#define	clrColor	0
#define	clrCrctMap	1
#define	rewireMode	2
#define	rewireR		3
#define	rewireG		4
#define	rewireB		5
#define	rewireA		6

	Texmap *pTex = (Texmap*)GetCOREInterface()->CreateInstance(TEXMAP_CLASS_ID, ColorCorrectTexID);
	IParamBlock2* pBlock = pTex->GetParamBlock(0);
	pBlock->SetValue(clrColor, 0, col);
	if(pTarget) pBlock->SetValue(clrCrctMap, 0, pTarget);
	pBlock->SetValue(rewireMode, 0, 3);
	pBlock->SetValue(rewireR, 0, 3);
	pBlock->SetValue(rewireG, 0, 3);
	pBlock->SetValue(rewireB, 0, 3);

	return pTex;
}
//=============================================================================
//=============================================================================
void SetTextureOutputScale(Texmap* pTex, float scale)
{
	if (!pTex) return;

	TextureOutput* texout = NULL;
	if (pTex->ClassID() == VRayBitmapID) {
		texout = (TextureOutput*)pTex->SubAnim(4);
	}
	else if (pTex->ClassID() == bmptexClassID) {
		texout = ((BitmapTex*)pTex)->GetTexout();
	}

	if (texout) {
		texout->SetOutputLevel(0, scale);
	}
}