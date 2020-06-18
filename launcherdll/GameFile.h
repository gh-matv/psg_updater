#pragma once

#include <Windows.h>

#include <string>
#include <sstream>
#include <fstream>
#include <functional>

#include <boost/filesystem.hpp>

#include "Encrypt.h"
#include "Internet.h"
#include "ByteStream.h"
#include "Manifest.h"

#include "Defines.h"
#include "gCs.h"

#define URL_DOWNLOAD_FILE SG_URL "?c=Api_Patcher&a=File"

#include <boost/uuid/sha1.hpp>
#include <boost/algorithm/hex.hpp>

class GameFile
{
public:

private:

	static std::string toString(const boost::uuids::detail::sha1::digest_type &digest);

	static std::string _cachedname(uint8_t serverid, std::string filename);

	static std::string _cachedpath(uint8_t serverid, std::string filename);

	static bool _iscached(uint8_t serverid, std::string filename);

	static bool _iscurrent(std::string fullpath);

	static bool download(int serverid, std::string& filename, std::string& fullpath);

	static bool backupfile(int serverid, std::string& filename, std::string& fullpath);

public:

	// Set backup_in_cache to true in case the user changed the selected server
	static bool getfile(int serverid, std::string fullpath, std::string compare_hash, bool backup_in_cache = false, int oldServer = 0);

	// NOTE: THERE *MUST* ALWAYS HAVE AT LEAST ONE FILE ON "." DIRECTORY IN THE SERVER MANIFEST FOR THIS TO WORK.
	static bool DownloadFiles(
		const std::vector<std::pair<std::string, std::shared_ptr<ManifestElement>>>&mes,
		const std::vector<std::string>& paths,
		const std::string &basepath,
		const uint8_t serverid,
		const bool changeServer = false,
		const int oldServer = 0,
		std::function<void(const std::string& fname)> cb = NULL);

};

