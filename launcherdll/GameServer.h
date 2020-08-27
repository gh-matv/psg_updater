
#pragma once

#include <map>
#include <string>
#include <iostream>

#include "Defines.h"
#include "Globals.h"
#include "Manifest.h"

#include <ByteStream.h>
#include "Internet.h"

using namespace palace;

class GameServer
{
public:

	static uint8_t selectedServer;
	static uint8_t oldServer;

	enum errMessage: int
	{
		ERRM_UNAUTHORIZED = 1,
		ERRM_NOSERVERAVAILABLE,
		ERRM_USERISBANNED,
		ERRM_NOCREATESERVFILE_GENERIC,
		ERRM_NOCREATESERVFILE_OUTOFMEM,
		ERRM_NOCREATESERVFILE_DOWNLOADFAILED
	};
	
	struct serverdata_s
	{
		uint8_t id;
		uint16_t port;
	};

	static std::map<std::string, serverdata_s> pAvailableServers;

	static std::map<std::string, serverdata_s>& GetAvailableServers(std::string userkey, int &errcode)
	{
		std::cout << __FUNCTION__ << std::endl;
		auto servs_file = Internet::Download(SG_APIURL("AvailableServers", "beta=1", userkey));

		std::cout << servs_file << std::endl;

		// In case it's unable to create the file (internet error, or no directory authorization)
		if (servs_file == "")
		{
			switch (Internet::lastresult)
			{
			case E_OUTOFMEMORY:
				errcode = ERRM_NOCREATESERVFILE_OUTOFMEM;
				break;
			case INET_E_DOWNLOAD_FAILURE:
				errcode = ERRM_NOCREATESERVFILE_DOWNLOADFAILED;
				break;
			default:
				errcode = ERRM_NOCREATESERVFILE_GENERIC;
			}

			return pAvailableServers;
		}

		std::ifstream ifs(servs_file, std::ios::binary);
		ByteStream<uint16_t> bs(ifs);

		auto servCount = bs.read<int16_t>();

		// In case the server sent an error code
		if (servCount <= 0)
		{
			errcode = -1 * servCount; // returned as negative number from serv
			return pAvailableServers;
		}

		// The server sent the correct data, parse it.
		for (auto i = 0; i != servCount; ++i)
		{
			serverdata_s serverdata;

			serverdata.id = bs.read<uint8_t>();
			serverdata.port = bs.read<uint16_t>();
			std::string servName = bs.read();

			pAvailableServers[servName] = serverdata;
		}

		return pAvailableServers;
	}

	static Manifest GetManifestForServer(uint8_t serverId, std::string playerkey)
	{
#ifdef _DEBUG
		auto url = SG_APIURL("GetManifest", std::string("s=").append(std::to_string(serverId)), playerkey);
		std::cout << "Getting manifest at url=" << url << std::endl;
#endif

		auto manifestPath = Internet::Download(SG_APIURL("GetManifest", std::string("s=").append(std::to_string(serverId)), playerkey));

		std::cout << "Maybe download failed" << std::endl;

		if (manifestPath == "")
		{
			std::cout << "\\!/ UNABLE TO DOWNLOAD" << std::endl;
			return Manifest();
		}

		std::cout << "Before load manifest" << std::endl;

		Manifest m;
		m.LoadManifest(manifestPath);

		std::cout << "After load manifest" << std::endl;

		return m;
	}
};

uint8_t GameServer::selectedServer = 0;
uint8_t GameServer::oldServer = 0;
std::map<std::string, GameServer::serverdata_s> GameServer::pAvailableServers = {};
