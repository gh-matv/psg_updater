#include <windows.h>
#include <tlhelp32.h>
#include <stdio.h>
#include <psapi.h>
#include <iostream>
#include <algorithm>

#include <thread>

#pragma comment(lib, "ntdll.lib")

class CL
{
public:
	~CL()
	{
#ifdef _DEBUG
		system("pause");
#endif
	}
};

extern "C" NTSTATUS NTAPI NtQueryInformationProcess(HANDLE ProcessHandle, ULONG ProcessInformationClass, PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);
ULONG_PTR getppid()
{
	ULONG_PTR pbi[6];
	ULONG ulSize = 0;
	LONG(WINAPI *NtQueryInformationProcess)(HANDLE ProcessHandle, ULONG ProcessInformationClass,
		PVOID ProcessInformation, ULONG ProcessInformationLength, PULONG ReturnLength);
	*(FARPROC *)&NtQueryInformationProcess =
		GetProcAddress(LoadLibraryA("NTDLL.DLL"), "NtQueryInformationProcess");
	if (NtQueryInformationProcess) {
		if (NtQueryInformationProcess(GetCurrentProcess(), 0,
			&pbi, sizeof(pbi), &ulSize) >= 0 && ulSize == sizeof(pbi))
			return pbi[5];
	}
	return (ULONG_PTR)-1;
}

int main(int argc, char* argv[])
{
	CL cl;

	if (argc != 2)
	{
		std::cerr << "Wrong number of args." << std::endl;
		return EXIT_FAILURE;
	}

	std::string exe_path;
	exe_path.resize(MAX_PATH);

	std::shared_ptr<void> h(OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, getppid()), CloseHandle);
	if (0 == h)
	{
		std::cerr << "OpenProcess() failed: " << GetLastError() << "\n";
		return EXIT_FAILURE;
	}
	else
	{
		if (!GetModuleFileNameExA(h.get(), 0, (LPSTR)exe_path.data(), sizeof(exe_path.size()) - 1))
		{
			std::cerr << "GetModuleFileNameEx() failed: " <<
				GetLastError() << "\n";
			return EXIT_FAILURE;
		}
	}

	// Wait for the process to exit
	DWORD ExitCodeResult;
	do
	{
		GetExitCodeProcess(h.get(), &ExitCodeResult);
		Sleep(100);
	} while (ExitCodeResult == STILL_ACTIVE);

	CloseHandle(h.get());

	auto last_slash = exe_path.find_last_of('/', 0);
	if (!DeleteFileA((LPCSTR)(exe_path.data() + last_slash + 1 )));
	{
		std::cerr << "Unable to delete the file" << std::endl;
		return EXIT_FAILURE;
	}
	if (!MoveFileA(argv[1], (LPCSTR)old_launcher.filename().c_str()))
	{
		std::cerr << "Unable to move the file" << std::endl;
		return EXIT_FAILURE;
	}

	system("pause");
	return 0;

}
