#pragma once

#include <fstream>
#include <sstream>
#include <Windows.h>

#include <boost/filesystem.hpp>

#include "Encrypt.h"
#include "Internet.h"
#include "Defines.h"
#include "Globals.h"
#include "base64.h"

#include "Hwid.h"

class Auth
{
#define TOKEN_FILE_NAME "token"
#define URL_LOGIN SG_URL "?c=Auth&a=Login"

public:

	static bool isAuth()
	{
		auto v = !Globals::userkey.empty() || std::ifstream(TOKEN_FILE_NAME).is_open();
		return v;
	}

	static std::string AuthKey()
	{
		std::ifstream ifs(TOKEN_FILE_NAME, std::ios::binary);
		if (!ifs.is_open())
		{
#ifdef _DEBUG
			std::cout << "[AUTH] Reading file failed" << std::endl;
#endif
			return false;
		}

		std::stringstream ss;
		ss << ifs.rdbuf();

		std::cout << "[AUTH] Read authkey=" << ss.str() << std::endl;

		// Decrypt it
		sHWID hdd;
		getDiskSerial(hdd);
		auto str = Encrypt::Xor(ss.str(), hdd.MainDiskSerial);

		Globals::userkey = str;
		return str;
	}

	static bool Authentify(
		std::string login,
		std::string pass)
	{
		auto el = base64_encode((const unsigned char*)login.c_str(), login.size());
		auto ep = base64_encode((const unsigned char*)pass.c_str(), pass.size());

		std::stringstream ss;
		ss << URL_LOGIN
			<< "&u=" << el
			<< "&p=" << ep;

		auto x = ss.str();
		auto authcode = Internet::ReadString(ss.str());
		if (authcode.empty()) return false;

		std::ofstream ofs((boost::filesystem::path(".") / TOKEN_FILE_NAME).string(), std::ios::trunc);
		if (!ofs.is_open()) return false;

		// Encrypt it
		sHWID hdd;
		getDiskSerial(hdd);
		ofs << Encrypt::Xor(authcode, hdd.MainDiskSerial);

		return true;
	}

	static void LogOut()
	{
		std::cout << "Before logging out" << std::endl;
		DeleteFileW((boost::filesystem::path(".") / TOKEN_FILE_NAME).c_str());
		std::cout << "After logging out" << std::endl;
		Globals::userkey = "";
	}

};

