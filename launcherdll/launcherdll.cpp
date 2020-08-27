// launcherdll.cpp : Définit les fonctions exportées de la DLL.
//

#define _SILENCE_CXX17_ITERATOR_BASE_CLASS_DEPRECIATION_WARNING

#include "pch.h"
#include "framework.h"
#include "launcherdll.h"

#include <string>
#include <thread>
#include <sstream>

#include "Auth.h"
#include "GameServer.h"
#include "GameFile.h"

#include "Console.h"
#include "gCs.h";


// Il s'agit d'un exemple de variable exportée
/*
LAUNCHERDLL_API int nlauncherdll=0;

// Il s'agit d'un exemple de fonction exportée.
LAUNCHERDLL_API int fnlauncherdll(void)
{
    return 0;
}

// Il s'agit du constructeur d'une classe qui a été exportée.
Clauncherdll::Clauncherdll()
{
    return;
}
*/

#define API LAUNCHERDLL_API __declspec(cdecl)

std::thread *gThread;
bool thread_started = false;

std::string playerkey;
unsigned selected_server;
unsigned selected_server_port;

char error_code = 0;

std::string gCs::gMessage = "";
size_t gCs::gProgress = 0;
size_t gCs::gTotalProgress = 0;

class test
{
public:
	static void ThreadContent()
	{
		std::stringstream ss;

		thread_started = true;

		gCs::gMessage = "Retrieving manifest";
		std::cout << "Before entering GetManifestForServer" << std::endl;
		auto manifest_server = GameServer::GetManifestForServer(selected_server, Auth::AuthKey());

		std::cout << "Server files:" << std::endl;
		//for (const auto &e : manifest_server.manifestElements)
			//std::cout << "\t" << e.name << std::endl;

		Manifest manifest_local;

		// If there is no existing manifest
		if (!std::fstream(MANIFEST_DEFAULTNAME).is_open())
		{
#ifdef _DEBUG
			std::cout << "[Main] Creating manifest file from current dir" << std::endl;
#endif
			gCs::gMessage = "Building own manifest";
			manifest_local = Manifest::Manifest(".");
		}
		else
		{
#ifdef _DEBUG
			std::cout << "[Main] Using existing manifest file" << std::endl;
#endif
			gCs::gMessage = "Loading manifest";
			manifest_local.LoadManifest(MANIFEST_DEFAULTNAME);
		}

		gCs::gMessage = "Diffing manifests";
		auto diff = manifest_server.CompareManifestWith(manifest_local);

#ifdef _DEBUG
		std::cout << "Diffs:" << std::get<0>(diff).size() << "," << std::get<1>(diff).size() << std::endl;
#endif
		gCs::gTotalProgress = std::get<1>(diff).size(); // Total number of files

		gCs::gMessage = "Applying differences.";
		Manifest::MakeSubfolders(".", std::get<0>(diff));

		gCs::gMessage = "Downloading files";
		if (!GameFile::DownloadFiles(std::get<1>(diff), manifest_server.relativePaths, ".", selected_server, false))
		{
#ifdef _DEBUG
			std::cout << "[Main] Failed to download at least one file !" << std::endl;
#endif
			thread_started = false;
			gCs::gMessage = "Download failed !";
			error_code = 1;
			return;
		}

#ifdef _DEBUG
		std::cout << "[Main] Game finished updating !" << std::endl;
#endif

		gCs::gMessage = "Game is up to date.";
		manifest_server.SaveManifest(MANIFEST_DEFAULTNAME);

		thread_started = false;
	}
};

extern "C"
{
	API void openconsole()
	{
#ifdef _DEBUG
		std::cout << "[API]" __FUNCTION__ << std::endl;
		Console::PushWindow("PSG Launcher Debug Console");
		Console::ShowConsoleWindow();
#endif
	}

	API bool isconnected()
	{
		std::cout << "[API]" __FUNCTION__ << std::endl;
		return Auth::isAuth();
	}

	API bool trylogin(char *login, char* pass)
	{
		std::cout << "[API]" __FUNCTION__ << std::endl;
		return Auth::Authentify(std::string(login), std::string(pass));
	}

	API bool getservers(char *msg, size_t max_sz)
	{
		std::cout << "[API]" __FUNCTION__ << std::endl;
		int errCode = 0;
		auto servers = GameServer::GetAvailableServers(Auth::AuthKey(), errCode);
		if (errCode != 0)
		{
			strcpy_s(msg, max_sz, std::to_string(errCode).c_str());
			return true;
		}

		std::stringstream ss;
		for (const auto &s : servers)
		{
			ss << s.first << ",";
		}
		
		if (ss.str().size() > max_sz) return false;

		strcpy_s(msg, max_sz, ss.str().c_str());
		return true;
	}

	API bool quickuptodatecheck()
	{
		std::cout << "[API]" __FUNCTION__ << std::endl;
		return false; // false = not up2date
	}

	API bool startupdate(char *servname)
	{
		std::cout << "[API]" __FUNCTION__ "(" << GameServer::pAvailableServers[servname].id << ")" << std::endl;
		//if (thread_started) return false;
		selected_server = GameServer::pAvailableServers[servname].id;
		selected_server_port = GameServer::pAvailableServers[servname].port;
		gThread = new std::thread(test::ThreadContent);
		thread_started = true;
		return true;
	}

	API bool updatedone()
	{
		//std::cout << "[API]" __FUNCTION__ << std::endl;
		return !thread_started;
	}

	API char updateresult()
	{
		std::cout << "[API]" __FUNCTION__ << std::endl;
		return error_code;
	}

	API float getprogresspercent()
	{
		std::cout << "[API]" __FUNCTION__ << std::endl;
		return (0.0f + gCs::gProgress)/(gCs::gTotalProgress != 0 ? gCs::gTotalProgress : 1);
	}

	// Get the current progress message to show
	API void getprogressmsg(char *msg, size_t max_sz)
	{
		//std::cout << "[API]" __FUNCTION__ << std::endl;
		strcpy_s(msg, max_sz, gCs::gMessage.c_str());
	}

	// Force check all files (slow)
	API bool forcecheck()
	{
		std::cout << "[API]" __FUNCTION__ << std::endl;
		return false;
	}

	API void logout()
	{
		std::cout << "[API]" __FUNCTION__ << std::endl;
		Auth::LogOut();
	}

	API bool startgame(size_t cp=1147)
	{
		std::cout << "[API]" __FUNCTION__ << std::endl;
		std::stringstream ss;
		ss << "./StreetGears.exe"
			<< " /locale:cp" << cp
			<< " /auth_port:55000"
			<< " /auth_ip:160.20.145.163"
			//<< "/auth_ip:127.0.0.1"
			<< " /login_id:" << Auth::AuthKey();

		std::cout << "[CMD]" << ss.str() << std::endl;

		STARTUPINFO si;
		PROCESS_INFORMATION pi;
		ZeroMemory(&si, sizeof(si));
		si.cb = sizeof(si);
		ZeroMemory(&pi, sizeof(pi));

		// Start the child process. 
		
		if (!CreateProcess(
			NULL,   // No module name (use command line)
			(LPSTR)ss.str().c_str(),        // Command line
			NULL,           // Process handle not inheritable
			NULL,           // Thread handle not inheritable
			FALSE,          // Set handle inheritance to FALSE
			0,              // No creation flags
			NULL,           // Use parent's environment block
			NULL,           // Use parent's starting directory 
			&si,            // Pointer to STARTUPINFO structure
			&pi)           // Pointer to PROCESS_INFORMATION structure
			)
		{
			printf("CreateProcess failed (%d).\n", GetLastError());
			return false;
		}
		
		
		return true;
	}
}
