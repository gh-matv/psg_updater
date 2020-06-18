
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

	static std::map<std::string, uint8_t> GetAvailableServers(std::string userkey)
	{
		auto servs_file = Internet::Download(SG_APIURL("AvailableServers", "", userkey));
		if (servs_file == "") return std::map<std::string, uint8_t>();

		std::ifstream ifs(servs_file, std::ios::binary);
		ByteStream<uint16_t> bs(ifs);

		std::map<std::string, uint8_t> servers;
		auto servCount = bs.read<uint16_t>();

		for (auto i = 0; i != servCount; ++i)
		{
			uint8_t servId = bs.read<uint8_t>();
			std::string servName = bs.read();

			servers[servName] = servId;
		}

		return servers;
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

