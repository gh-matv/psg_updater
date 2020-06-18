
#include "pch.h"
#include "GameFile.h"
#include <boost/filesystem.hpp>

std::string GameFile::toString(const boost::uuids::detail::sha1::digest_type & digest)
{
	const auto charDigest = reinterpret_cast<const char *>(&digest);
	std::string result;
	boost::algorithm::hex(charDigest, charDigest + sizeof(boost::uuids::detail::sha1::digest_type), std::back_inserter(result));
	return result;
}

std::string GameFile::_cachedname(uint8_t serverid, std::string filename)
{
	unsigned int dig[5];
	boost::uuids::detail::sha1 sha;
	sha.process_bytes(filename.data(), filename.size());
	sha.get_digest(dig);

	std::string hash = toString(dig);

	std::stringstream ss;
	ss << (int)serverid
		<< hash;

	std::cout << "Filename: " << filename << "\tHash: " << hash << std::endl;

	return ss.str();
}

std::string GameFile::_cachedpath(uint8_t serverid, std::string filename)
{
	auto a = boost::filesystem::absolute(Globals::gamepath == "." ? "" : Globals::gamepath);
	a /= "__cache\\";
	auto b = a.string();
	return b + _cachedname(serverid, filename);
}

bool GameFile::_iscached(uint8_t serverid, std::string filename)
{
	std::ifstream ifs(_cachedpath(serverid, filename));
	return ifs.is_open();
}

bool GameFile::_iscurrent(std::string fullpath)
{
	std::ifstream ifs(fullpath);
	return ifs.is_open();
}

bool GameFile::download(int serverid, std::string & filename, std::string & fullpath)
{
#ifdef _DEBUG
	std::cout << "[D]" << filename << std::endl;
#endif

	auto url = std::string(URL_DOWNLOAD_FILE) + "&p=" + Globals::userkey + "&s=" + std::to_string(serverid) + "&f=" + filename;
	std::cout << "[RQ]" << url << std::endl;
	auto tmpfile = Internet::Download(url);
	std::cout << "[RQ]" << tmpfile << std::endl;
	if (tmpfile == "")
	{
#ifdef _DEBUG
		std::cout << "\\!/ Download failed !" << std::endl;
#endif
		return false;
	}


	DeleteFile(fullpath.c_str());
	return MoveFile(tmpfile.c_str(), fullpath.c_str());
}

bool GameFile::backupfile(int serverid, std::string & filename, std::string & fullpath)
{
	std::cout << "[BP]" << filename << std::endl;
	auto from = fullpath.c_str();
	auto to = _cachedpath(serverid, filename);
	auto to2 = to.c_str();
	DeleteFile(to.c_str());
	auto ret = MoveFile(fullpath.c_str(), to.c_str());
	return ret;
}

// Set backup_in_cache to true in case the user changed the selected server

bool GameFile::getfile(int serverid, std::string fullpath, std::string compare_hash, bool backup_in_cache, int oldServer)
{
	auto filename = boost::filesystem::path(fullpath).filename().string();

	if (backup_in_cache && _iscurrent(fullpath))
		if (!backupfile(oldServer, filename, fullpath))
		{
			auto x = GetLastError();
			return false;
		}

	if (_iscached(serverid, filename))
	{
		if (Encrypt::HashFile(_cachedpath(serverid, filename)) == compare_hash)
		{
#ifdef _DEBUG
			std::cout << "[C] " << filename << std::endl;
#endif

			DeleteFile(fullpath.c_str());
			return MoveFile(_cachedpath(serverid, filename).c_str(), fullpath.c_str());
		}
		else
		{
			// File exists, but wrong hash. Either the file was corrupted, or there is a new version.
			DeleteFile(_cachedpath(serverid, filename).c_str());
		}
	}

	return download(serverid, filename, fullpath);
}

// NOTE: THERE *MUST* ALWAYS HAVE AT LEAST ONE FILE ON "." DIRECTORY IN THE SERVER MANIFEST FOR THIS TO WORK.

bool GameFile::DownloadFiles(const std::vector<std::pair<std::string, std::shared_ptr<ManifestElement>>>& mes, const std::vector<std::string>& paths, const std::string & basepath, const uint8_t serverid, const bool changeServer, const int oldServer, std::function<void(const std::string&fname)> cb)
{

	std::cout << "Downloading " << mes.size() << " files." << std::endl;

	for (const auto& me : mes)
	{
		gCs::gMessage = "Downloading " + me.first;
		//if (!getfile(serverid, (basepath + (me.second->path != 0 ? "\\" + paths[me.second->path] : "") + "\\" + me.first), me.second->hash, changeServer, oldServer))
		if (cb) cb(me.first);
		if (!getfile(serverid, paths[me.second->path] + "\\" + me.first, me.second->hash, changeServer, oldServer))
		{
			int x = 2;
			return false;
		}
		gCs::gProgress++;
	}

	return true;
}
