
#pragma once

#include <string>
#include <vector>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <tuple>
#include <Windows.h>

#include <boost/filesystem.hpp>
#include <map>
#include <set>

#include "ByteStream.h"
#include "Encrypt.h"

#include "Globals.h"
#include "gCs.h"

#include <Chrono.h>

#include <iostream>

#define MANIFEST_TEMPNAME "mnfst.tmp"
#define MANIFEST_DEFAULTNAME "mnfst.sg"

struct ManifestElement
{
	uint32_t path;
	uint32_t size;
	std::string hash = "";

	bool isLocal = false;

	ManifestElement() {}
	ManifestElement(uint32_t p, size_t s, std::string h) : path(p), size(s), hash(h) {}
};

class Manifest
{

public:
	std::vector<std::string> relativePaths;
	std::map<std::string /*path*/, std::shared_ptr<ManifestElement>> manifestElements;

	Manifest()
	{
	}

	// Create a manifest from a directory
	// std::string path: directory. Doesnt need "/" at the end.
	// bool getHashes: force the manifest to compute all hashes immediatly ? (v. slow!). Default: false
	Manifest(std::string path, bool getHashes = false);

	// Exports the manifest to a binary file.
	void SaveManifest(std::string path);

	// Import the manifest from a binary file.
	void LoadManifest(std::string path);

	// Compares two manifests.
	// "this" must refer to the correct manifest.
	// the parameter "m" should refer to the local version
	// Returns a tuple of :
	//  -0: the list of missing directories
	//  -1: a list of incorrect (missing/modified) ManifestElement (files)
	// Take note that files that only exists in "m" will be ignored.
	std::tuple < std::vector<std::string> /* missing subdirs */, std::vector<std::pair<std::string, std::shared_ptr<ManifestElement>>> /* missing/modified files */> CompareManifestWith(Manifest& m);

	static bool MakeSubfolders(std::string root, std::vector<std::string>& subdirs);

};
