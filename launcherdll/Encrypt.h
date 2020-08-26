#pragma once

#include <Windows.h>

#include <iostream>
#include <fstream>
#include <vector>
#include <thread>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/sha1.hpp>
#include <sstream>

class Encrypt
{
public:

	static std::string Xor(std::string a, std::string b)
	{
		std::stringstream ss;

		auto len = a.length();
		for (auto i = 0; i != len; ++i)
			ss << (a[i] ^= b[i%b.length()]);

		return ss.str();
	}

	static std::string Xor(std::string inp, char xorkey = 0x42)
	{
		for (auto i = 0; i != inp.size(); ++i)
			inp[i] ^= xorkey;
		return inp;
	}

	static std::string HashFile(std::string filepath)
	{
		std::ifstream ifs(filepath, std::ios::binary | std::ios::ate);
		if (!ifs.is_open())
		{
			std::cout << "Unable to open file." << std::endl;
			return "";
		}

		size_t sz = ifs.tellg();
		ifs.seekg(0, std::ios::beg);

		std::vector<char> v;
		v.resize(sz);

		ifs.read(v.data(), sz);

		boost::uuids::detail::sha1 sha1;
		sha1.process_bytes(v.data(), sz);

		unsigned hash[5];
		sha1.get_digest(hash);

		std::stringstream ss;

		for (auto v : hash)
		{
			ss << v;
		}

		return ss.str();
	}

};
