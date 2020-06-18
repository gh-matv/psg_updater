#pragma once

#define BOOST_ASIO_DISABLE_CONCEPTS
#define WIN32_LEAN_AND_MEAN

#include "pch.h"
#include <WinSock2.h>

#include <string>
#include <boost/filesystem.hpp>

class Globals
{
public:

	static boost::filesystem::path gamepath;
	static std::string userkey;
};

