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
#include <iostream>
#include <fstream>
#include <algorithm>

#include <webp/encode.h>

#if _MSC_VER >= 1930    // Visual Studio 2022 (v143)
#ifdef NDEBUG
#pragma comment(lib, "webp/lib/vs2022/Release/libwebp.lib")
#pragma comment(lib, "webp/lib/vs2022/Release/libsharpyuv.lib")
#else
#pragma comment(lib, "webp/lib/vs2022/Debug/libwebp.lib")
#pragma comment(lib, "webp/lib/vs2022/Debug/libsharpyuv.lib")
#endif
#elif _MSC_VER >= 1920    // Visual Studio 2019 (v142)
#ifdef NDEBUG
#pragma comment(lib, "webp/lib/vs2019/Release/libwebp.lib")
#pragma comment(lib, "webp/lib/vs2019/Release/libsharpyuv.lib")
#else
#pragma comment(lib, "webp/lib/vs2019/Debug/libwebp.lib")
#pragma comment(lib, "webp/lib/vs2019/Debug/libsharpyuv.lib")
#endif
#else    //
#ifdef NDEBUG
#pragma comment(lib, "webp/lib/vs2017/Release/libwebp.lib")
#pragma comment(lib, "webp/lib/vs2017/Release/libsharpyuv.lib")
#else
#pragma comment(lib, "webp/lib/vs2017/Debug/libwebp.lib")
#pragma comment(lib, "webp/lib/vs2017/Debug/libsharpyuv.lib")
#endif
#endif


BOOL glTFExporter_Core::WebpEncode(Texmap* pTex, WebpTextureStruct &str)
{

	Bitmap* pBmp = NULL;
	BitmapTex* pWorkBitmapTex = NULL;

	if (pTex->ClassID() == bmptexClassID) {
		pBmp = ((BitmapTex*)pTex)->GetBitmap(0);
	}
	else if (pTex->ClassID() == VRayBitmapID) {
		TSTR name = pTex->GetParamBlock(0)->GetStr(0);
		pWorkBitmapTex = NewDefaultBitmapTex();
		pWorkBitmapTex->SetMapName(name);
		pWorkBitmapTex->ReloadBitmapAndUpdate();
		pBmp = pWorkBitmapTex->GetBitmap(0);
	}
	else if (pTex->ClassID() == CoronaBitmapID) {
		TSTR name = pTex->GetParamBlock(0)->GetStr(0);
		pWorkBitmapTex = NewDefaultBitmapTex();
		pWorkBitmapTex->SetMapName(name);
		pWorkBitmapTex->ReloadBitmapAndUpdate();
		pBmp = pWorkBitmapTex->GetBitmap(0);
	}
	else if (GetOSLMapType(pTex) == OSL_UberBitmap) {
		TSTR name = pTex->GetParamBlock(1)->GetStr(10);
		pWorkBitmapTex = NewDefaultBitmapTex();
		pWorkBitmapTex->SetMapName(name);
		pWorkBitmapTex->ReloadBitmapAndUpdate();
		pBmp = pWorkBitmapTex->GetBitmap(0);
	}
	else {
		return FALSE;
	}

	BitmapInfo bi = pBmp->GetBitmapInfo();

	tstring filename = bi.Filename();
	std::filesystem::path destname(filename);
#if MAX_RELEASE > 26000
	tstring fl = GetCOREInterface()->GetDir(APP_SYSTEM_IMAGE_DIR).data();
#else
	tstring fl = GetCOREInterface()->GetDir(APP_SYSTEM_IMAGE_DIR);
#endif
	tstring retname = fl + _T("\\") + tstring(destname.stem()) + _T(".webp");

	int width = bi.Width();
	int height = bi.Height();
	int elements = 4;
	std::vector< std::uint8_t > in(width * height * elements);

	for (int y = 0; y < height; ++y){
		for (int x = 0; x < width; ++x)	{
			BMM_Color_fl pix;
			pBmp->GetPixels(x,y,1, &pix);
			in[y * width * elements + x * elements + 0] = static_cast<std::uint8_t>(std::clamp(pix.r * 255.0f, 0.0f, 255.0f));
			in[y * width * elements + x * elements + 1] = static_cast<std::uint8_t>(std::clamp(pix.g * 255.0f, 0.0f, 255.0f));
			in[y * width * elements + x * elements + 2] = static_cast<std::uint8_t>(std::clamp(pix.b * 255.0f, 0.0f, 255.0f));
			in[y * width * elements + x * elements + 3] = static_cast<std::uint8_t>(std::clamp(pix.a * 255.0f, 0.0f, 255.0f));
		}
	}

	std::uint8_t* data = nullptr;
	auto size = 0ull;
	int stride = width * elements;

	{
		if (str.LossLess) {
			size = WebPEncodeLosslessRGBA(in.data(), width, height, stride, &data);
		}
		else {
			size = WebPEncodeRGBA(in.data(), width, height, stride, str.QualityFactor, &data);
		}

		//MessageBox(NULL, (std::to_wstring(size)).c_str(), _T(""), MB_OK);
		
		//write_file(retname);
		std::ofstream o(retname, std::ios::binary);
		o.write(reinterpret_cast<const char*>(data), size);
		o.close();

		WebPFree(data);
	}

	if (pWorkBitmapTex) pWorkBitmapTex->DeleteThis();


	str.originalPathStr = retname;

	return TRUE;
}
