#include <iostream>
#include <vector>
#include "cxxopts.hpp"

int main(int argc, char* argv[]) {
	cxxopts::Options options("vocaloid-cli", "Command line version of vocaloid");
	options.add_options()
		("lib, library", "library path", cxxopts::value<std::string>())
		("i, input", "input file list", cxxopts::value<std::vector<std::string>>())
		("o, output", "output file list", cxxopts::value<std::vector<std::string>>())
		("a, arguments", "arguments", cxxopts::value<std::vector<std::string>>())
		;

	options.parse(argc, argv);
	return 0;
}