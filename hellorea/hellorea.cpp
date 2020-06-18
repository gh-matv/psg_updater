// hellorea.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include <Windows.h>

typedef void(*ff)();


int main(int argc, char* argv[])
{
	/*
	HMODULE hModule = LoadLibrary(TEXT("lnc.dll"));
	ff f = (ff)GetProcAddress(hModule, "openconsole");

	f();
    std::cout << "Hello Rea!\n";
	system("pause");
	*/

	for (auto i = 0; i != argc; ++i)
		std::cout << argv[i] << std::endl;
	system("pause");

}

