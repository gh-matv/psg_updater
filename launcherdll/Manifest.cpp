
#include "pch.h"
#include "Manifest.h"

// Create a manifest from a directory
// std::string path: directory. Doesnt need "/" at the end.
// bool getHashes: force the manifest to compute all hashes immediatly ? (v. slow!). Default: false

Manifest::Manifest(std::string path, bool getHashes)
{

	// By default, getHashes is set to false, because most of the time, there is faster ways
	//  to compare two files (size, path, etc...). The hash is computed in the comparison function,
	//  in case the previous comparison methods didn't find any difference.

	gCs::gMessage = "Scanning directory...";


#ifdef _DEBUG
	std::cout << "[MNFST]" << std::endl;
	Chrono c;
	c.Start();
#endif

	boost::filesystem::path p(path);
	if (is_directory(p))
	{
		for (const auto& x : boost::filesystem::recursive_directory_iterator(p))
		{

			if (boost::filesystem::is_directory(x)) continue;
			if (x.path().string().find("__cache") != std::string::npos)
			{
#ifdef _DEBUG
				//std::cout << "Ignoring __cache file " << x.path().string() << std::endl;
#endif
				continue;
			}

			auto filepath = boost::filesystem::path(x);
			auto relativepath = boost::filesystem::relative(filepath.parent_path(), p);

			// GET THE RELATIVE PATH INDEX		
			auto f = std::find(relativePaths.begin(), relativePaths.end(), relativepath);
			if (f == relativePaths.end())
			{
				relativePaths.push_back(relativepath.string());
				f = relativePaths.end() - 1;
			}
			auto pos = std::distance(relativePaths.begin(), f);

			// GET HASH
			std::string hash = "";
			if (getHashes)
			{
				hash = Encrypt::HashFile(x.path().string());
			}

#ifdef _DEBUG
			//auto res = 
#endif
			manifestElements.insert(
				std::pair<std::string, std::shared_ptr<ManifestElement>>(filepath.filename().string(),
					std::make_shared<ManifestElement>(
					(uint32_t)pos,
						boost::filesystem::file_size(x),
						hash
						)));

#ifdef _DEBUG
			//std::cout << "\t" << res.first->first << ": " << relativePaths[res.first->second->path] << std::endl;
#endif

		}
	}
#ifdef _DEBUG
	else
	{
		//std::cout << "Unable to open path (" << path << "): the specified path doesnt lead to a directory." << std::endl;
	}
#endif

#ifdef _DEBUG
	c.End();
	std::cout << "[MNFST] Taken " << c.Count() << "ms to generate manifest !" << std::endl;
#endif

}

// Exports the manifest to a binary file.

void Manifest::SaveManifest(std::string path)
{

	palace::ByteStream<uint16_t> bs(10);

	bs << (uint32_t)relativePaths.size();
	bs << (uint32_t)manifestElements.size();

	for (const auto& rp : relativePaths)
		bs << Encrypt::Xor(rp);

	for (const auto& me : manifestElements)
	{
		bs << Encrypt::Xor(me.first);
		bs << me.second->path;
		bs << me.second->size;
		bs << Encrypt::Xor(me.second->hash);
	}

	std::ofstream ofs(path, std::ios::trunc | std::ios::binary);
	bs.ostream(ofs);

}

// Import the manifest from a binary file.

void Manifest::LoadManifest(std::string path)
{
	gCs::gMessage = "Loading manifest file";

	std::ifstream ifs(path, std::ios::beg | std::ios::binary);
	palace::ByteStream<uint16_t> bs(ifs);

	auto rps = bs.read<uint32_t>();
	auto mes = bs.read<uint32_t>();
	relativePaths.resize(rps);

	for (auto i = 0; i != relativePaths.size(); ++i)
	{
		relativePaths[i] = Encrypt::Xor(bs.read());
#ifdef _DEBUG
		std::cout << "\t+Path: " << relativePaths[i] << std::endl;
#endif
	}

	for (auto i = 0; i != mes; ++i)
	{
		auto elt = manifestElements.insert(
			std::make_pair<std::string, std::shared_ptr<ManifestElement>>(
				Encrypt::Xor(bs.read()),
				std::make_shared<ManifestElement>()
				));

		elt.first->second->path = bs.read<uint32_t>();
		elt.first->second->size = bs.read<uint32_t>();
		elt.first->second->hash = Encrypt::Xor(bs.read());
#ifdef _DEBUG
		std::cout << "\t+Elt: " << elt.first->first << "\t" << elt.first->second->path << "\t" << elt.first->second->size << std::endl;
#endif
	}
}

// Compares two manifests.
// "this" must refer to the correct manifest.
// the parameter "m" should refer to the local version
// Returns a tuple of :
//  -0: the list of missing directories
//  -1: a list of incorrect (missing/modified) ManifestElement (files)
// Take note that files that only exists in "m" will be ignored.

std::tuple<std::vector<std::string>, std::vector<std::pair<std::string, std::shared_ptr<ManifestElement>>>> Manifest::CompareManifestWith(Manifest & m)
{

	// Searching for missing relative paths
	std::vector<std::string> missing_relativepaths;
	for (const auto& f : relativePaths)
		if (std::find(m.relativePaths.begin(), m.relativePaths.end(), f) == m.relativePaths.end())
			missing_relativepaths.push_back(f);

#ifdef _DEBUG
	std::cout << "[MNFST] Starting diffing files" << std::endl;
#endif

	std::vector<std::pair<std::string, std::shared_ptr<ManifestElement>>> diff;
	//std::set_difference(manifestElements.begin(), manifestElements.end(), m.manifestElements.begin(), m.manifestElements.end(), std::inserter(diff, diff.begin()), comparer);


	for (auto &m : manifestElements) m.second->isLocal = true;
	for (auto &m : m.manifestElements) m.second->isLocal = false;

	for (const auto &me : manifestElements)
	{
		if (!m.manifestElements.count(me.first))
		{
			std::cout << "\t[+Missing] " << me.first << std::endl;
			diff.push_back(me);
		}
		else
		{
			auto comparer = [&m, this](const std::pair<const std::string, std::shared_ptr<ManifestElement>> &server, const std::pair<const std::string, std::shared_ptr<ManifestElement>> &client)->bool {
				// Compare sizes
				if (server.second->size != client.second->size)
				{
					std::cout << "\t[Cpr] (size) " << server.second->size << " " << client.second->size << "\t" << server.first << std::endl;
					return false;
				}

				// Compare hashes
				if (client.second->hash.empty())
				{
#ifdef _DEBUG
					std::cout << "Getting hash for file (B) '" << client.first << "'" << std::endl;
#endif
					client.second->hash = Encrypt::HashFile(m.relativePaths[client.second->path] + "/" + client.first);
				}

				if (server.second->hash != client.second->hash)
				{
					std::cout << "\t[Cpr] (hash) " << server.second->hash << " (" << std::boolalpha << server.second->isLocal << ") " << client.second->hash << "\t" << server.first << std::endl;
					std::cin.get();
					return false;
				}

				return true;
			};

			auto oth = std::make_pair(me.first, m.manifestElements.at(me.first));
			if (!comparer(me, oth))
			{
				std::cout << "\t[+Modified] " << me.first << std::endl;
				diff.push_back(me);
			}
		}
	}

#ifdef _DEBUG
	std::cout << "[MNFST]DIFF SZ: " << diff.size() << std::endl;
#endif

	return { missing_relativepaths, diff };
}

bool Manifest::MakeSubfolders(std::string root, std::vector<std::string>& subdirs)
{
	CreateDirectory((root + "\\__cache").c_str(), NULL);
	SetFileAttributes((root + "\\__cache").c_str(), FILE_ATTRIBUTE_HIDDEN | FILE_ATTRIBUTE_NOT_CONTENT_INDEXED);

	for (const auto& sd : subdirs)
	{
		if (sd != ".")
			CreateDirectory((root + "\\" + sd).c_str(), NULL);
	}

	return true;
}
