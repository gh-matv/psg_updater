#include "pch.h"

#include "Console.h"

HANDLE Console::m_Handle;

BOOL Console::PushWindow(const std::string &strConsoleTitle)
{
	AllocConsole();

	std::freopen("conin$", "r+t", stdin);
	std::freopen("conout$", "w+t", stdout);
	std::freopen("conout$", "w+t", stderr);

	if (!strConsoleTitle.empty())
		SetConsoleTitle(strConsoleTitle.c_str());

	m_Handle = GetStdHandle(STD_OUTPUT_HANDLE);
	return SetConsoleTextAttribute(m_Handle, 2 | 1 | 4);
}

void Console::PopWindow()
{
	FreeConsole();
}

void Console::ShowConsoleWindow()
{
	auto hConsole = GetConsoleWindow();
	if (hConsole != nullptr)
		ShowWindow(hConsole, SW_SHOW);
}

void Console::HideConsoleWindow()
{
	auto hConsole = GetConsoleWindow();
	if (hConsole != nullptr)
		ShowWindow(hConsole, SW_HIDE);
}

void Console::DisplayByteBuffer(uint8_t *pData, std::size_t stSize)
{
	std::stringstream ss;
	for (std::size_t i = 0; i != stSize; ++i)
		ss << std::hex << std::setfill('0') << std::setw(2) << std::uppercase << static_cast<int32_t>(pData[i]) << " ";

	std::cout << ss.str() << std::endl;
}