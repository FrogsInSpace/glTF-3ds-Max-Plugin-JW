
#include "HSglTFExporter.h"
#include <bitmap.h>
#include <gamma.h>

static BOOL s_CalcGamma = FALSE;
static void CorrectBitmapGamma(BitmapTex*& pBmpTex, float gamma);

//=============================================================================
//=============================================================================
void SetPNGInfo(Bitmap* pBitmap)
{
	if (!pBitmap) return;
#if 1
	ClassEntry* ce = GetCOREInterface()->GetDllDirectory()->ClassDir().FindClassEntry(BMM_IO_CLASS_ID, Class_ID(0x6be260fb, 0));
	if (!ce) return;
	ClassDesc* cd = ce->FullCD();
	if (!cd) return;

	IBitmapIO_Png* pPNG_BmpIO = (IBitmapIO_Png*)cd->GetInterface(BMPIO_INTERFACE);
	pPNG_BmpIO->SetType(BMM_TRUE_24);
	pPNG_BmpIO->SetAlpha(FALSE);
#else
	ClassEntry* ce = GetCOREInterface()->GetDllDirectory()->ClassDir().FindClassEntry(BMM_IO_CLASS_ID, Class_ID(0x6be260fb, 0));
	if (!ce) return;
	ClassDesc* cd = ce->FullCD();
	if (!cd) return;

	IBitmapIO_Png* pPNG_BmpIO = (IBitmapIO_Png*)cd->GetInterface(BMPIO_INTERFACE);
	int type;
	pBitmap->GetStoragePtr(&type);
	pPNG_BmpIO->SetType(type);
	pPNG_BmpIO->SetAlpha(FALSE);
#endif
}

//======================================================================
//======================================================================
BitmapTex* MergeRGBChannelTexture(BitmapTex* pTexR, BitmapTex* pTexG, BitmapTex* pTexB)
{
	Bitmap* pBmpR = NULL;
	Bitmap* pBmpG = NULL;
	Bitmap* pBmpB = NULL;

	tstring texFilePath;
	//tstring jpgFilePath;
	if (pTexR) {
		pBmpR = pTexR->GetBitmap(0);
		std::filesystem::path fname = pTexR->GetMapName();
		//texFilePath = fname;
		texFilePath = tstring(fname.parent_path()) + tstring(fname.stem()) + tstring(_T("_ORM")) + tstring(fname.extension());
		//jpgFilePath = tstring(fname.parent_path()) + tstring(fname.stem()) + tstring(_T("_ORM")) + tstring(_T(".jpg"));
		CopyFile(fname.c_str(), texFilePath.c_str(), FALSE);
	}
	if (pTexG) {
		pBmpG = pTexG->GetBitmap(0);
		if (texFilePath.size() == 0) {
			std::filesystem::path fname = pTexG->GetMapName();
			//texFilePath = fname;
			texFilePath = tstring(fname.parent_path()) + tstring(fname.stem()) + tstring(_T("_ORM")) + tstring(fname.extension());
			//jpgFilePath = tstring(fname.parent_path()) + tstring(fname.stem()) + tstring(_T("_ORM")) + tstring(_T(".jpg"));
			CopyFile(fname.c_str(), texFilePath.c_str(), FALSE);
		}
	}
	if (pTexB) {
		pBmpB = pTexB->GetBitmap(0);
		if (texFilePath.size() == 0) {
			std::filesystem::path fname = pTexB->GetMapName();
			//texFilePath = fname;
			texFilePath = tstring(fname.parent_path()) + tstring(fname.stem()) + tstring(_T("_ORM")) + tstring(fname.extension());
			//jpgFilePath = tstring(fname.parent_path()) + tstring(fname.stem()) + tstring(_T("_ORM")) + tstring(_T(".jpg"));
			CopyFile(fname.c_str(), texFilePath.c_str(), FALSE);
		}
	}
	//
	if (!PathFileExists(texFilePath.c_str())) {
		return NULL;
	}
/*
	if (s_CalcGamma)
	{
		gammaMgr.SetFileOutGamma(1.0f);
		gammaMgr.Enable(FALSE);
	}
	else {
		gammaMgr.SetFileOutGamma(2.2f);
	}
*/
	BitmapTex* pBmpTex = NewDefaultBitmapTex();
	pBmpTex->SetMapName(texFilePath.c_str());
	Bitmap* pOriginalBmp = pBmpTex->GetBitmap(0);
	if(!pOriginalBmp) return NULL;
	SetPNGInfo(pOriginalBmp);

	BitmapInfo bi = pOriginalBmp->GetBitmapInfo();
	bi.SetType(BMM_TRUE_24);
	bi.SetName(texFilePath.c_str());

	/*
	BitmapTex *pJPGBmpTex = NewDefaultBitmapTex();
	pJPGBmpTex->SetMapName(jpgFilePath.c_str());
	BitmapInfo JPGbi = pOriginalBmp->GetBitmapInfo();
	JPGbi.SetName(jpgFilePath.c_str());
	*/

	for (int w = 0; w < bi.Width(); w++) {
		for (int h = 0; h < bi.Height(); h++) {
			BMM_Color_64 buf1, buf2;
			buf2.r = (UINT)0x0;
			buf2.g = (UINT)0xffff;
			buf2.b = (UINT)0xffff;
			if (pBmpR) {
				pBmpR->GetPixels(w, h, 1, &buf1);
				if (s_CalcGamma)
					buf2.r = static_cast<uint16_t>(powf((float)buf1.r / 0xffff, 2.2f) * 0xffff);
				else
					buf2.r = buf1.r;
			}
			if (pBmpG) {
				pBmpG->GetPixels(w, h, 1, &buf1);
				if (s_CalcGamma)
					buf2.g = static_cast<uint16_t>(powf((float)buf1.g / 0xffff, 2.2f) * 0xffff);
				else
					buf2.g = buf1.g;
			}
			if (pBmpB) {
				pBmpB->GetPixels(w, h, 1, &buf1);
				if (s_CalcGamma)
					buf2.b = static_cast<uint16_t>(powf((float)buf1.b / 0xffff, 2.2f) * 0xffff);
				else
					buf2.b = buf1.b;
			}
			pOriginalBmp->PutPixels(w, h, 1, &buf2);
		}
	}
	pOriginalBmp->OpenOutput(&bi);
	pOriginalBmp->Write(&bi);
	pOriginalBmp->Close(&bi);
	pOriginalBmp->DeleteThis();

	//pOriginalBmp->OpenOutput(&JPGbi);
	//pOriginalBmp->Write(&JPGbi);
	//pOriginalBmp->Close(&JPGbi);
	//pOriginalBmp->DeleteThis();
	//gammaMgr.SetFileOutGamma(2.2f);

	if (s_CalcGamma)
		CorrectBitmapGamma(pBmpTex, 1.0f);

	return pBmpTex;
}


//======================================================================
//======================================================================
Bitmap* SetBitmapFromTexmap(Texmap* pTex, BitmapInfo& bi)
{
	if (!pTex) return NULL;

	Bitmap* p = TheManager->Create(&bi);
	pTex->RenderBitmap(0, p);
	return p;
/*
	if (pTex->ClassID() == bmptexClassID) {
		return ((BitmapTex*)pTex)->GetBitmap(0);
	}
	else {
	}
	if (pTex->ClassID() == ColorCorrectTexID) {
		Texmap* pt = pTex->GetParamBlock(0)->GetTexmap(1);
		BitmapInfo bi;
		if (pt->ClassID() == bmptexClassID) {
			bi = ((BitmapTex*)pt)->GetBitmap(0)->GetBitmapInfo();
		}
		else return NULL;
		Bitmap* p = TheManager->Create(&bi);
		pTex->RenderBitmap(0, p);
		return p;
	}
	int type = GetOSLMapType(pTex);
	if (type == OSL_BitmapLookUp) {
		TSTR name = pTex->GetParamBlock(1)->GetStr(2, 0);
		BitmapTex *pBmpTex = GetBitmapTexFromName(name);
		if (pBmpTex) {
			Bitmap* p = pBmpTex->GetBitmap(0);
			pTex->RenderBitmap(0, p);
			return p;
		}
	}
	else if (type == OSL_UberBitmap) {
		TSTR name = pTex->GetParamBlock(1)->GetStr(10, 0);
		BitmapTex* pBmpTex = GetBitmapTexFromName(name);
		if (pBmpTex) {
			Bitmap* p = pBmpTex->GetBitmap(0);
			pTex->RenderBitmap(0, p);
			return p;
		}
	}

	return NULL;
*/
}

//======================================================================
//======================================================================
BitmapTex* MergeRGBChannelTexture(Texmap* pTexR, Texmap* pTexG, Texmap* pTexB, const tstring &Basename, const tstring& fileType)
{
	std::filesystem::path texFilePath;
	texFilePath = ExportFolder() + tstring(_T("\\")) + Basename + TextureTableCountStr() + fileType;
	DeleteFile(texFilePath.c_str());

	IPoint2 mapSize = GetBitmapSize();
	if (mapSize == IPoint2(0, 0)) {
		IPoint2 temp(0,0);
		if (pTexG) {
			if (pTexG->ClassID() == bmptexClassID) {
				temp.x = ((BitmapTex*)pTexG)->GetBitmap(0)->Width();
				temp.y = ((BitmapTex*)pTexG)->GetBitmap(0)->Height();
			}
		}
		if (pTexB) {
			if (pTexB->ClassID() == bmptexClassID) {
				mapSize.x = ((BitmapTex*)pTexB)->GetBitmap(0)->Width();
				mapSize.y = ((BitmapTex*)pTexB)->GetBitmap(0)->Height();
			}
			if (mapSize.x > temp.x)temp.x = mapSize.x;
			if (mapSize.y > temp.y)temp.y = mapSize.y;
		}
		if (pTexR) {
			if (pTexR->ClassID() == bmptexClassID) {
				mapSize.x = ((BitmapTex*)pTexR)->GetBitmap(0)->Width();
				mapSize.y = ((BitmapTex*)pTexR)->GetBitmap(0)->Height();
			}
			if (mapSize.x > temp.x)temp.x = mapSize.x;
			if (mapSize.y > temp.y)temp.y = mapSize.y;
		}
		mapSize.x = temp.x;
		mapSize.y = temp.y;
	}
	if (mapSize.x == 0 || mapSize.y == 0) {
		mapSize.x = 1024;
		mapSize.y = 1024;
	}
	BitmapInfo bi;
	bi.SetHeight(mapSize.y);
	bi.SetWidth(mapSize.x);
	bi.SetName(texFilePath.c_str());
	bi.SetType(BMM_TRUE_64);
	bi.SetFlags(0);

	Bitmap* pBmpR = SetBitmapFromTexmap(pTexR, bi);
	Bitmap* pBmpG = SetBitmapFromTexmap(pTexG, bi);
	Bitmap* pBmpB = SetBitmapFromTexmap(pTexB, bi);
/*
	Bitmap* pSrcBitmap = NULL;
	if (pBmpG) {
		pSrcBitmap = pBmpG;
		Bitmap* p = TheManager->Create(&bi);
		p->CopyImage(pBmpG, COPY_IMAGE_RESIZE_HI_QUALITY, 0);
		pSrcBitmap = p;
	}
	else if (pBmpB) {
		//pSrcBitmap = pBmpB;
		Bitmap* p = TheManager->Create(&bi);
		p->CopyImage(pBmpB, COPY_IMAGE_RESIZE_HI_QUALITY, 0);
		pSrcBitmap = p;
	}
	else if (pBmpR) {
		//pSrcBitmap = pBmpR;
		Bitmap* p = TheManager->Create(&bi);
		p->CopyImage(pBmpR, COPY_IMAGE_RESIZE_HI_QUALITY, 0);
		pSrcBitmap = p;
	}
*/
	//CopyFile(bi.Name(), texFilePath.c_str(), FALSE);
	BitmapTex* pBmpTex = CreateBitmapTex(texFilePath.c_str(), NULL, mapSize);
	Bitmap* pDestBmp = pBmpTex->GetBitmap(0);
	/*
	gammaMgr.SetFileOutGamma(2.2f);
	if (s_CalcGamma)
	{
		gammaMgr.SetFileOutGamma(1.0f);
		gammaMgr.Enable(FALSE);
	}
	else {
		gammaMgr.SetFileOutGamma(2.2f);
	}
	*/
	int ww = bi.Width();
	int hh = bi.Height();

	for (int w = 0; w < bi.Width(); w++) {
		for (int h = 0; h < bi.Height(); h++) {
			BMM_Color_64 buf1, buf2;
			buf2.r = (UINT)0xffff;
			buf2.g = (UINT)0xffff;
			buf2.b = (UINT)0xffff;
			if (pBmpR) {
				pBmpR->GetPixels(w, h, 1, &buf1);
				if (s_CalcGamma)
					buf2.r = static_cast<uint16_t>(powf((float)buf1.r / 0xffff, 2.2f) * 0xffff);
				else
					buf2.r = buf1.r;
			}
			if (pBmpG) {
				pBmpG->GetPixels(w, h, 1, &buf1);
				if (s_CalcGamma)
					buf2.g = static_cast<uint16_t>(powf((float)buf1.g / 0xffff, 2.2f) * 0xffff);
				else
					buf2.g = buf1.g;
			}
			if (pBmpB) {
				pBmpB->GetPixels(w, h, 1, &buf1);
				if (s_CalcGamma)
					buf2.b = static_cast<uint16_t>(powf((float)buf1.b / 0xffff, 2.2f) * 0xffff);
				else
					buf2.b = buf1.b;
			}
			pDestBmp->PutPixels(w, h, 1, &buf2);
		}
	}
	pDestBmp->OpenOutput(&bi);
	pDestBmp->Write(&bi);
	pDestBmp->Close(&bi);
	pDestBmp->DeleteThis();

	if (pBmpR) pBmpR->DeleteThis();
	if (pBmpG) pBmpG->DeleteThis();
	if (pBmpB) pBmpB->DeleteThis();

	//gammaMgr.SetFileOutGamma(2.2f);
	if (s_CalcGamma)
		CorrectBitmapGamma(pBmpTex, 1.0f);

	return pBmpTex;
}

//=============================================================================
//=============================================================================
void CorrectBitmapGamma(BitmapTex*& pBmpTex, float gamma)
{
	if (!pBmpTex) return;

	IParamBlock2* pb2 = pBmpTex->GetParamBlock(0);
	// get the bitmap parameter
	int n = pb2->GetDesc()->NameToIndex(_T("bitmap"));
	ParamID id = pb2->GetDesc()->IndextoID(n);
	PBBitmap* pbBitmap = pb2->GetBitmap(id);

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

	// now reload the bitmap from the disk.
	pBmpTex->ReloadBitmapAndUpdate();
}
