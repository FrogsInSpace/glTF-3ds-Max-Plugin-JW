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

#ifdef _DEBUG_

#include <iostream>
#include <string>
#include <curl/curl.h>

// Callback function to save response data
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* output) {
    size_t totalSize = size * nmemb;
    output->append((char*)contents, totalSize);
    return totalSize;
}

int proc() {
    // 認証キー（YOUR_AUTH_KEY を適切なキーに置き換えてください）
    std::string authKey = "YOUR_AUTH_KEY";

    // リクエストURL
    std::string url = "https://api.rapidpipeline.com/api/v2/user/tokens";

    // cURL の初期化
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize curl" << std::endl;
        return 1;
    }

    // レスポンスデータを格納する変数
    std::string response;

    // HTTP ヘッダーを設定
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + authKey).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Accept: application/json");

    // cURL 設定
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());                 // URL 設定
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);              // ヘッダー設定
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);     // レスポンスのコールバック設定
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);             // レスポンスデータ格納先

    // GET リクエスト送信
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "cURL request failed: " << curl_easy_strerror(res) << std::endl;
    }
    else {
        std::cout << "Response: " << response << std::endl;  // レスポンス表示
    }

    // 後処理
    curl_slist_free_all(headers);  // ヘッダー解放
    curl_easy_cleanup(curl);       // cURL クリーンアップ

    return 0;
}

void glTFExporter_Core::PostProcess(const tstring& fullpath)
{
    proc();
}
#else
void glTFExporter_Core::PostProcess(const tstring& fullpath)
{
}
#endif
