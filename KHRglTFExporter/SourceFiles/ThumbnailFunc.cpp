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

#include "KHRglTFExporter.h"

//======================================================================
// Get image from active View and save it as thumbNail
//======================================================================
bool SaveActiveViewportAsDIB(const tstring& filename)
{
//    Interface* ip = GetCOREInterface();
	ViewExp& vpt = GetCOREInterface()->GetActiveViewExp();

	GraphicsWindow* gw = vpt.getGW();
	if (!gw) return false;

	int dibSize = 0;
	if (!gw->getDIB(nullptr, &dibSize))  return false;
	if (dibSize <= 0) return false;

	std::vector<BYTE> pngData;
		pngData.resize(dibSize);

	BITMAPINFO* bmi = reinterpret_cast<BITMAPINFO*>(pngData.data());
	if (!gw->getDIB(bmi, &dibSize)) return false;

	BITMAPINFOHEADER* bih = &bmi->bmiHeader;
	int width = bih->biWidth;
	int height = abs(bih->biHeight);
	int bitsPerPixel = bih->biBitCount;
	if (bitsPerPixel != 32) return false;

	BYTE* pixels = pixels = pngData.data() + sizeof(BITMAPINFOHEADER);

	BitmapInfo bi;
	bi.SetHeight(height);
	bi.SetWidth(width);
	bi.SetName(filename.c_str());
	bi.SetType(BMM_TRUE_64);
	Bitmap* pSrcBmp = TheManager->Create(&bi);

	int stride = width * 4;
	for (int y = 0; y < height; y++){
	    // DIB : bottom-up
	    int srcY = (bih->biHeight > 0) ? height - 1 - y : y;
	    BYTE* row = pixels + srcY * stride;
	    for (int x = 0; x < width; x++) {
	        BYTE* pixel = row + x * 4;
	        // DIB: BGR
	        BYTE b = pixel[0];
	        BYTE g = pixel[1];
	        BYTE r = pixel[2];

	        BMM_Color_64 col;
	        col.r = (uint16_t)r * 257;
	        col.g = (uint16_t)g * 257;
	        col.b = (uint16_t)b * 257;
	        col.a = 65535;
	        pSrcBmp->PutPixels(x, y, 1, &col);
	    }
	}

	int thumbNailSize = 200;
	bi.SetHeight(thumbNailSize);
	bi.SetWidth(thumbNailSize * width/ height);
	//bi.SetType(BMM_TRUE_64);
	Bitmap* pDstBmp = TheManager->Create(&bi);
	pDstBmp->CopyImage(pSrcBmp, COPY_IMAGE_RESIZE_LO_QUALITY, 0);
	pSrcBmp->DeleteThis();

	pDstBmp->OpenOutput(&bi);
	pDstBmp->Write(&bi);
	pDstBmp->Close(&bi);
	pDstBmp->DeleteThis();

	return true;
}

//======================================================================
// Create ThumbImage and save it
//======================================================================
int glTFExporter_Core::CreateThumbNail(void)
{
	int imageIndex = -1;

	TSTR imgPath = GetCOREInterface()->GetDir(APP_IMAGE_DIR);
	tstring WorkImageFolder = tstring(imgPath.data()) + _T("\\") + tstring(m_fullpath.stem()) + tstring(_T("_Images\\"));
	std::filesystem::create_directory(WorkImageFolder);
	tstring filename = WorkImageFolder + _T("ThumbNail.png");
	if (SaveActiveViewportAsDIB(filename)) {
		imageIndex = CreateImage(filename.c_str());

		m_model.asset.thumbnail = imageIndex;
		//tinygltf::Value::Object in_0;
		//in_0.insert(std::make_pair("thumbnail", tinygltf::Value(imageIndex)));
		//m_model.asset.extras = tinygltf::Value(in_0);
	}

	return imageIndex;

#if 0
	{
	    tinygltf::BufferView bv;
	    bv.buffer = 0;
	    bv.byteOffset = m_BufferByteOffset;
	    bv.byteLength = pngData.size();

	    void* ptr = SecureMemory((int)pngData.size());
	    char* dstPtr = (char*)ptr + bv.byteOffset;
	    memcpy(dstPtr, pngData.data(), pngData.size());

	    m_model.bufferViews.push_back(bv);
	    int bufferViewIndex = (int)m_model.bufferViews.size() - 1;
	    //image.bufferView = bufferViewIndex;
	    //  m_model.images.push_back(image);
	    //  int imageIndex = (int)m_model.images.size() - 1;
	}
	return imageIndex;
#endif
}

#if 0
//======================================================================
// DIB->BMP File
//======================================================================
BOOL WriteToImageFile(std::vector<BYTE>&pngData, const tstring & filename)
{
	BITMAPINFOHEADER* bih = reinterpret_cast<BITMAPINFOHEADER*>(pngData.data());

	DWORD paletteSize = 0;

	if (bih->biBitCount <= 8)
	{
		paletteSize =
			(bih->biClrUsed ?
				bih->biClrUsed :
				(1 << bih->biBitCount)) *
			sizeof(RGBQUAD);
	}

	DWORD pixelOffset = sizeof(BITMAPINFOHEADER) + paletteSize;
	BITMAPFILEHEADER bfh = {};

	bfh.bfType = 0x4D42; // "BM"
	bfh.bfOffBits = sizeof(BITMAPFILEHEADER) + pixelOffset;

	bfh.bfSize = bfh.bfOffBits + bih->biSizeImage;
	HANDLE hFile = CreateFileW(
		filename.c_str(),
		GENERIC_WRITE,
		0,
		nullptr,
		CREATE_ALWAYS,
		FILE_ATTRIBUTE_NORMAL,
		nullptr);

	if (hFile == INVALID_HANDLE_VALUE)   return false;

	DWORD written;
	WriteFile(hFile, &bfh, sizeof(bfh), &written, nullptr);
	WriteFile(hFile, pngData.data(), pixelOffset + bih->biSizeImage, &written, nullptr);

	CloseHandle(hFile);

	return true;
}
#endif
