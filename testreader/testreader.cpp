// testreader.cpp : Ce fichier contient la fonction 'main'. L'exécution du programme commence et se termine à cet endroit.
//

#include <iostream>
#include <ByteStream.h>
#include <fstream>
#include <string>

std::string Xor(std::string inp, char xorkey = 0x42)
{
	for (auto i = 0; i != inp.size(); ++i)
		inp[i] ^= xorkey;
	return inp;
}

int main()
{
    std::cout << "Hello World!\n";
	std::ifstream ifs("C:\\Users\\geekg\\source\\repos\\ui_is_for_assholes\\WpfApp1\\bin\\Debug\\file.bin", std::ios::binary);

	palace::ByteStream<uint16_t> bs(ifs);
	bs.seekg(0, std::ios::beg);

	auto rps = bs.read<uint32_t>();
	auto mes = bs.read<uint32_t>();

	for (auto i = 0; i != rps; ++i)
	{
		std::cout << Xor(bs.read()) << std::endl;
	}

	for (auto i = 0; i != mes; ++i)
	{
		std::string xnm = Xor(bs.read());
		uint32_t path = bs.read<uint32_t>();
		uint32_t size = bs.read<uint32_t>();
		std::string hsh = Xor(bs.read());

		std::cout << xnm << '\t' << path << "\t" << size << "\t" << hsh << std::endl;
	}
	system("pause");
}
