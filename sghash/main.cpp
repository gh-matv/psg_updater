#include <iostream>
#include <fstream>


int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cout << 0 << std::endl;
		return EXIT_FAILURE;
	}

	std::ifstream ifs(argv[1], std::ios::binary);
	
}
