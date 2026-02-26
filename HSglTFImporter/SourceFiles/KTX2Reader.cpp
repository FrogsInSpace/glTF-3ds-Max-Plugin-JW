#include "HSglTFImporter.h"
//#undef max
//#undef min
#include <vector>
#include <iostream>

// libktx をスタティックライブラリとして扱うための宣言
#ifndef KHRONOS_STATIC
#define KHRONOS_STATIC
#endif
#ifndef KTX_API_STATIC
#define KTX_API_STATIC
#endif

#include <KTX2/ktx.h>

#if _MSC_VER >= 1930    // Visual Studio 2022 (v143)
#ifdef NDEBUG
#pragma comment(lib, "KTX2/lib/vs2022/Release/ktx.lib")
#else
#pragma comment(lib, "KTX2/lib/vs2022/Debug/ktx.lib")
#endif
#elif _MSC_VER >= 1920    // Visual Studio 2019 (v142)
#ifdef NDEBUG
#pragma comment(lib, "KTX2/lib/vs2019/Release/ktx.lib")
#else
#pragma comment(lib, "KTX2/lib/vs2019/Debug/ktx.lib")
#endif
#else    //
#ifdef NDEBUG
#pragma comment(lib, "KTX2/lib/vs2017/Release/ktx.lib")
#else
#pragma comment(lib, "KTX2/lib/vs2017/Debug/ktx.lib")
#endif
#endif

//===================================================================
// libktxで展開した生データ(RGBA)を3ds MaxのBitmapに変換する
//===================================================================
Bitmap* CreateMaxBitmapFromRawData(unsigned char* pRawData, int width, int height)
{
	BitmapInfo bi;
	bi.SetWidth(width);
	bi.SetHeight(height);
	bi.SetType(BMM_TRUE_64);
	bi.SetFlags(MAP_HAS_ALPHA);

	Bitmap* pBitmap = TheManager->Create(&bi);
	if (!pBitmap) return nullptr;

	for (int y = 0; y < height; ++y) {
		std::vector<BMM_Color_64> row(width);

		for (int x = 0; x < width; ++x) {
			int offset = (y * width + x) * 4;

			// 8bit(0-255) を 16bit(0-65535) に変換して格納
			row[x].r = (unsigned short)pRawData[offset] << 8;
			row[x].g = (unsigned short)pRawData[offset + 1] << 8;
			row[x].b = (unsigned short)pRawData[offset + 2] << 8;
			row[x].a = (unsigned short)pRawData[offset + 3] << 8;
		}

		pBitmap->PutPixels(0, y, width, row.data());
	}

	return pBitmap;
}

//===================================================================
//===================================================================
ktxBasisParams getParamsFromTexture(ktxTexture2* texture) {
	ktxBasisParams params = { 0 };
	params.structSize = sizeof(params);

	// 1. モードの判別
	//params.uastc = texture->isUastc;

	// 2. 法線マップ設定の推測
	//void* pValue;
	//ktx_uint32_t valueLen;
	//if (ktxHashList_FindValue(&texture->kvDataHead, KTX_NORMAL_MAP_KEY, &valueLen, &pValue) == KTX_SUCCESS) {
	//	params.normalMap = KTX_TRUE;
	//}

	// 3. 不明な項目は推奨値をセット
	params.compressionLevel = 2;
	params.qualityLevel = 128;

	return params;
}

//===================================================================
// KTX2ファイルを読み込み、RGBA8888形式のバッファを返す関数
//===================================================================
bool LoadKTX2ToRawRGBA(const tstring& ktxfilename, const tstring &filename, IBitmapIO_Png* pPNG_BmpIO)
{
	ktxTexture2* kTexture = nullptr;
	KTX_error_code result;

	std::string str = WStringToString(ktxfilename);
	result = ktxTexture2_CreateFromNamedFile(str.c_str(), KTX_TEXTURE_CREATE_LOAD_IMAGE_DATA_BIT, &kTexture);
	if (result != KTX_SUCCESS) return false;

	// 2. Basis Universal形式（圧縮）かどうかをチェックし、必要ならトランスコード
	if (ktxTexture2_NeedsTranscoding(kTexture)) {
		result = ktxTexture2_TranscodeBasis(kTexture, KTX_TTF_RGBA32, 0);
		if (result != KTX_SUCCESS) {
			ktxTexture_Destroy(ktxTexture(kTexture));
			return false;
		}
	}

	// 1. ミップマップレベル0（最大サイズ）のデータサイズのみを取得
	std::vector<unsigned char> outData;
	ktx_size_t levelSize = ktxTexture_GetImageSize(ktxTexture(kTexture), 0);
	outData.resize(levelSize);

	// 2. レベル0のデータがメモリ上のどこにあるか（オフセット）を取得
	ktx_size_t offset = 0;
	result = ktxTexture_GetImageOffset(ktxTexture(kTexture), 0, 0, 0, &offset);

	if (result == KTX_SUCCESS) {
		// 3. 全体データの先頭ポインタを取得し、オフセット分進めた位置からコピー
		ktx_uint8_t* pAllData = ktxTexture_GetData(ktxTexture(kTexture));
		memcpy(outData.data(), pAllData + offset, levelSize);
	}
	else {
		ktxTexture_Destroy(ktxTexture(kTexture));
		return false;
	}

	ktxBasisParams params = getParamsFromTexture(kTexture);

	int width = kTexture->baseWidth;
	int height = kTexture->baseHeight;
	Bitmap* pBmp = CreateMaxBitmapFromRawData(outData.data(), width, height);

	BitmapInfo bi = pBmp->GetBitmapInfo();
/*
#if MAX_RELEASE >= 26000
	if (kTexture->vkFormat == 157){
		MaxSDK::ColorManagement::IColorPipelineMgr* cpm = (MaxSDK::ColorManagement::IColorPipelineMgr*)GetCOREInterface(COLORPIPELINEMGR_INTERFACE);

		auto settings = cpm->Settings();
		if (settings->IsOCIOBased()) {
			auto ret = bi.SetRequestedColorSpace(settings->GetDataColorSpaceName(), MaxSDK::ColorManagement::ColSpaceSource::User);
			bi.SetName(filename.c_str());
			bi.ResetCustomFlag(BMM_CUSTOM_FILEGAMMA);
			bi.SetCustomFlag(BMM_CUSTOM_GAMMA);
			bi.SetCustomGamma(1.0f);
		}
	}
#endif
*/
	SetPNGInfo(pPNG_BmpIO, pBmp);

	bi.SetName(filename.c_str());
	pBmp->OpenOutput(&bi);
	pBmp->Write(&bi);
	pBmp->Close(&bi);
	pBmp->DeleteThis();

	// 5. 後片付け
	ktxTexture_Destroy(ktxTexture(kTexture));

	return true;
}

//===================================================================
//===================================================================
BOOL glTFImporter_Core::KTX2ImageCreater(const tstring& ktxname, tstring& retname)
{
	std::filesystem::path destname(ktxname);
	retname = m_WorkImageFolder + tstring(destname.stem()) + _T(".png");

	BOOL ret = LoadKTX2ToRawRGBA(ktxname, retname, m_pPNG_BmpIO);

	return ret;
}





