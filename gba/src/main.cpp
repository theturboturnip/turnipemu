#include "gba/gamepak.h"

#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char* argv[]){
	assert(argc >= 2);
	
	const char* bios_path = argv[1];
	std::vector<GBA::byte> bios;
	std::ifstream bios_file(bios_path, std::ios::binary);
	bios_file >> std::noskipws;
	bios_file.seekg(0, std::ios::end);
	bios.reserve(bios_file.tellg());
	bios_file.seekg(0, std::ios::beg);
	bios.insert(bios.begin(),
			   std::istreambuf_iterator<char>(bios_file),
               std::istreambuf_iterator<char>());
	
	const char* rom_path = argv[2];
	std::vector<GBA::byte> rom;
	std::ifstream rom_file(rom_path, std::ios::binary);
	rom_file >> std::noskipws;
	rom_file.seekg(0, std::ios::end);
	rom.reserve(rom_file.tellg());
	rom_file.seekg(0, std::ios::beg);
	rom.insert(rom.begin(),
			   std::istreambuf_iterator<char>(rom_file),
               std::istreambuf_iterator<char>());

	GBA::GamePak gamePak(rom);

	return 0;
}
