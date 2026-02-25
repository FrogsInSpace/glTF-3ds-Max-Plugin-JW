#if 0
#ifdef _DEBUG
#pragma comment(lib, "cpprest142_2_10d.lib")
#else
#pragma comment(lib, "cpprest142_2_10.lib")
#endif

#include <Windows.h>
#include <iostream>
#include <cpprest/http_client.h>

using namespace web;
using namespace web::http;
using namespace web::http::client;

#if 1
pplx::task<int> Post()
{
	return pplx::create_task([]
	{
		json::value postData;

		auto sex = utility::conversions::to_string_t("man");
		postData[L"user_info"][L"name"] = json::value::string(L"user001");
		postData[L"user_info"][L"sex"] = json::value::string(sex);
		postData[L"user_info"][L"age"] = json::value::number(20);

		// proxyあり
		// auto config = http_client_config();
		// config.set_proxy(web_proxy(utility::conversions::to_string_t("http://127.0.0.1:8080")));
		// http_client client(L"http://localhost/api", config);
//		http_client client(L"http://localhost/api");
//		return client.request(methods::POST, L"", postData.serialize(), L"application/json");

		http_client client(L"https://apps.autodesk.com/webservices/checkentitlement?userid=2N5FMZW9CCED&appid=2024453975166401172");
		return client.request(methods::POST, L"", L"");
	}).then([](http_response response)
	{
		if (response.status_code() == status_codes::OK)
		{
			//auto body = response.extract_string();
			//std::wcout << body.get().c_str() << std::endl;
			//std::cout << response.extract_json() << std::endl;
			return response.extract_json();
		}
	}).then([](json::value json)
	{
		// リザルトコードを返す
		return json[L"result"].as_integer();
	});
}
int LicenseTest(void)
{
	// コマンドプロンプトの文字コードをUTF-8に設定する
	//SetConsoleOutputCP(CP_UTF8);

	try
	{
		auto result = Post().wait();
		std::cout << "Result = " << result << std::endl;
	}
	catch (const std::exception &e)
	{
		std::cout << "Error " << e.what() << std::endl;
	}

	return 0;
}


#else
// #pragma comment(lib, "crypt32.lib")
// #pragma comment(lib, "bcrypt.lib")
// #pragma comment(lib, "winhttp.lib")
#include <cpprest/http_client.h>
#include <dbgprint.h>

int LicenseTest1(void)
{
	web::http::client::http_client client(_T("https://apps.autodesk.com"));

	auto response = client.request(web::http::methods::GET, _T("webservices/checkentitlement"),_T("?userid=2N5FMZW9CCED&appid=2024453975166401172")).get();
	if (response.status_code() == web::http::status_codes::OK)
	{
		// レスポンスを文字列として取得後、標準出力する
		auto body = response.extract_string().get();
		auto header = response.headers();
		std::wcout << body << std::endl;
		DebugOutputString(body.c_str());
		auto jbody = response.extract_json();
		//DebugOutputString(jbody.get().as_string().c_str());
		//std::wcout << jbody. << std::endl;
	}
	return 0;
}

int LicenseTest(void)
{
	web::http::client::http_client client(L"https://httpbin.org/get");
	//web::http::client::http_client client(_T("https://apps.autodesk.com/webservices/checkentitlement?userid=2N5FMZW9CCED&appid=2024453975166401172"));

	auto response = client.request(web::http::methods::GET).get();
	if (response.status_code() == web::http::status_codes::OK)
	{
		// レスポンスを文字列として取得後、標準出力する
		auto body = response.extract_string().get();
		auto header = response.headers();
		std::wcout << body << std::endl;                     
		DebugOutputString(body.c_str());
		auto jbody = response.extract_json();
		//DebugOutputString(jbody.get().as_string().c_str());
		//std::wcout << jbody. << std::endl;
	}
	return 0;
}

int LicenseTestBase(void)
{
	//web::http::client::http_client client(L"http://example.com/");
	web::http::client::http_client client(L"https://apps.autodesk.com/webservices/checkentitlement?userid=2N5FMZW9CCED&appid=2024453975166401172");

	auto response = client.request(web::http::methods::GET).get();
	if (response.status_code() == web::http::status_codes::OK)
	{
		// レスポンスを文字列として取得後、標準出力する
		auto body = response.extract_string().get();
		std::wcout << body << std::endl;
	}
	return 0;
}
#endif

#endif