#include "turnipemu/gba/gamepak.h"
#include "turnipemu/display.h"

#include <assert.h>
#include <fstream>
#include <iostream>
#include <string>

int main(int argc, char* argv[]){
	assert(argc >= 2);
	
	const char* bios_path = argv[1];
	std::vector<TurnipEmu::byte> bios;
	std::ifstream bios_file(bios_path, std::ios::binary);
	bios_file >> std::noskipws;
	bios_file.seekg(0, std::ios::end);
	bios.reserve(bios_file.tellg());
	bios_file.seekg(0, std::ios::beg);
	bios.insert(bios.begin(),
			   std::istreambuf_iterator<char>(bios_file),
               std::istreambuf_iterator<char>());
	
	const char* rom_path = argv[2];
	std::vector<TurnipEmu::byte> rom;
	std::ifstream rom_file(rom_path, std::ios::binary);
	rom_file >> std::noskipws;
	rom_file.seekg(0, std::ios::end);
	rom.reserve(rom_file.tellg());
	rom_file.seekg(0, std::ios::beg);
	rom.insert(rom.begin(),
			   std::istreambuf_iterator<char>(rom_file),
               std::istreambuf_iterator<char>());

	TurnipEmu::GBA::GamePak gamePak(rom);

	TurnipEmu::Display display("GameBoy Advance", 1280, 720);
	display.loop();
	
	return 0;
}
