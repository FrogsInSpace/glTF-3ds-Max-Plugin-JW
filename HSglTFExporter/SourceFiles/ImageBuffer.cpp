
#include "HSglTFExporter.h"
#include <AssetManagement/iassetmanager.h>
#include <AssetManagement/AssetUser.h>

//----------------------------------------------------------
//----------------------------------------------------------
void glTFExporter_Core::CreateImageBuffer(void)
{
	/*
	{
		int idx = 0;
		for (auto t : m_model.textures) {
			t.source += m_model.bufferViews.size();
			m_model.textures[idx++] = t;
		}
	}
	*/

	for (UINT it = 0; it < m_model.images.size();it++ ) {
		tinygltf::Image *image = &m_model.images[it];
		std::string fname = image->uri;
		tstring wfname = StringToWString(fname.c_str());
		if (!PathFileExists(wfname.c_str())) {
			MaxSDK::AssetManagement::AssetUser asset = MaxSDK::AssetManagement::IAssetManager::GetInstance()->GetAsset(TSTR(wfname.c_str()), MaxSDK::AssetManagement::AssetType::kBitmapAsset);
			MSTR str(wfname.c_str());
			asset.GetFullFilePath(str);
			fname = WStringToString(tstring(str));
		}

		struct stat stbuf;
		stat(fname.c_str(), &stbuf);

		int size = stbuf.st_size;

		char *buf = (char*)malloc(size);
		FILE *pFp = fopen(fname.c_str(), "rb");
		if (!pFp) continue;

		fread(buf, sizeof(char), size, pFp);
		fclose(pFp);

		tinygltf::BufferView bfView;// = Create_glTFBufferView();
		bfView.buffer = 0;
		bfView.byteOffset = m_BufferByteOffset;
		bfView.byteLength = size;
		//bfView.target = TINYGLTF_TARGET_ARRAY_BUFFER;

		void *ptr = SecureMemory(bfView.byteLength);
		memcpy((char*)ptr + bfView.byteOffset, buf, size);
		free(buf);

		m_model.bufferViews.push_back(bfView);
		image->bufferView = m_model.bufferViews.size() - 1;
		size_t idx = fname.find_last_of('.') + 1;
		image->mimeType = MimeTypes::getType(&fname[idx]);
		image->uri = "";
	}
}

//----------------------------------------------------------
// イメージファイルよりBase64文字列を作る
//----------------------------------------------------------
tstring GetURILFromFile(std::string &f)
{
	tstring fname(StringToWString(f.c_str()));

	if (!PathFileExists(fname.c_str())) {
		MaxSDK::AssetManagement::AssetUser asset = MaxSDK::AssetManagement::IAssetManager::GetInstance()->GetAsset(TSTR(fname.c_str()), MaxSDK::AssetManagement::AssetType::kBitmapAsset);
		MSTR str(fname.c_str());
		asset.GetFullFilePath(str);
		fname = str;
	}

	struct _stat stbuf;
	_wstat(fname.c_str(), &stbuf);

	int size = stbuf.st_size;

	unsigned char *data = (unsigned char*)malloc(size);
	FILE *pFp = _tfopen(fname.c_str(), _T("rb"));
	if (!pFp) return tstring(_T(""));
	fread(data, sizeof(unsigned char), size, pFp);
	fclose(pFp);

	std::string str;
	base64_encode(data, size, str);

	return StringToWString(str.c_str());
}

static char encoding_table[] = { 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
								'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
								'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
								'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
								'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
								'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
								'w', 'x', 'y', 'z', '0', '1', '2', '3',
								'4', '5', '6', '7', '8', '9', '+', '/' };
static char *decoding_table = NULL;
static int mod_table[] = { 0, 2, 1 };

//----------------------------------------------------------
// バイナリデータからBase64文字列をエンコードする
//----------------------------------------------------------
int base64_encode(const unsigned char *data, int size, std::string &ret)
{

	int output_length = 4 * ((size + 2) / 3);

	ret.resize(output_length);

	for (int i = 0, j = 0; i < size;) {

		uint32_t octet_a = i < size ? (unsigned char)data[i++] : 0;
		uint32_t octet_b = i < size ? (unsigned char)data[i++] : 0;
		uint32_t octet_c = i < size ? (unsigned char)data[i++] : 0;

		uint32_t triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

		ret[j++] = encoding_table[(triple >> 3 * 6) & 0x3F];
		ret[j++] = encoding_table[(triple >> 2 * 6) & 0x3F];
		ret[j++] = encoding_table[(triple >> 1 * 6) & 0x3F];
		ret[j++] = encoding_table[(triple >> 0 * 6) & 0x3F];
	}

	for (int i = 0; i < mod_table[size % 3]; i++)
		ret[output_length - 1 - i] = '=';

	return output_length;
}
