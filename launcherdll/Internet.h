#pragma once

#include <Windows.h>

#include <string>
#include <urlmon.h>
#include <fstream>

#include <ByteStream.h>

#pragma comment(lib, "Urlmon.lib")
using namespace palace;

class Internet
{
public:

	static inline HRESULT lastresult;

	static std::string Download(std::string url, std::string path = "")
	{
		if (path != "")
		{
			URLDownloadToFile(NULL, url.c_str(), path.c_str(), 0, NULL);
			return "";
		}
		else
		{
			std::string tmpFileName;
			tmpFileName.resize(_MAX_PATH);

			auto ret = URLDownloadToCacheFile(NULL, url.c_str(), (LPSTR)tmpFileName.data(), _MAX_PATH, NULL, NULL);
			lastresult = ret;

			if (ret != S_OK)
			{
				return "";
			}

			return tmpFileName;
		}
	}
	static ByteStream<uint16_t> DownloadBS(std::string url)
	{
		auto path = Download(url);
		if (path == "") return {};

		std::ifstream ifs(path, std::ios::binary);
		ByteStream<uint16_t> ss(ifs);

		return ss;
	}
	static std::string ReadString(std::string url)
	{
		auto fname = Download(url);
		if (fname == "") return "";

		std::ifstream ifs(fname);
		if (!ifs.is_open()) return "";

		std::string servret;
		std::getline(ifs, servret);

		ifs.close();
		DeleteFile(fname.c_str());

		return servret;
	}

};

//HRESULT Internet::lastresult = 0;
