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
    // TODO: The code below increases the risk of accidentally committing a real authentication key to the GitHub repository.
    // Authentication key (replace YOUR_AUTH_KEY with your appropriate key)
    std::string authKey = "YOUR_AUTH_KEY";

    // Request URL
    std::string url = "https://api.rapidpipeline.com/api/v2/user/tokens";

    // cURL initialization
    CURL* curl = curl_easy_init();
    if (!curl) {
        std::cerr << "Failed to initialize curl" << std::endl;
        return 1;
    }

    // Variable to store response data
    std::string response;

    // Set HTTP headers
    struct curl_slist* headers = nullptr;
    headers = curl_slist_append(headers, ("Authorization: Bearer " + authKey).c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    headers = curl_slist_append(headers, "Accept: application/json");

    // cURL configuration
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());                 // URL settings
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);              // Header settings
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);     // Response callback settings
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);             // Response data storage location

    // Sending a GET request
    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        std::cerr << "cURL request failed: " << curl_easy_strerror(res) << std::endl;
    }
    else {
        std::cout << "Response: " << response << std::endl;  // Display response
    }

    // Post-processing
    curl_slist_free_all(headers);  // Release headers
    curl_easy_cleanup(curl);       // cURL cleanup

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
