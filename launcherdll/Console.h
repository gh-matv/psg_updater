#ifndef CONSOLE_H
#define CONSOLE_H

#include <cstdint>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <string>
#include <Windows.h>

class Console
{

public:
	static BOOL PushWindow(const std::string &ConsoleTitle);
	static void PopWindow();

	static void ShowConsoleWindow();
	static void HideConsoleWindow();

	static void DisplayByteBuffer(uint8_t *Buffer, std::size_t Size);

private:
	static HANDLE m_Handle;

};

#endif