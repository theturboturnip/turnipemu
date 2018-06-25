#include "gb/cartridge.h"
#include "gb/cpu.h"
#include <assert.h>
#include <cstring>

namespace GB{

	Cartridge::Cartridge(std::vector<uint8_t> rom){
		fprintf(stdout, "Loading Rom...\n");

		char rom_name[17];
		rom_name[16] = '\0';
		strncpy(rom_name, reinterpret_cast<char*>(rom.data()) + RomData::ROM_OFFSET_NAME, 16);
		fprintf(stdout, "Rom Name: '%s'\n", rom_name);

		RomData::RomType rom_type = static_cast<RomData::RomType>(rom[RomData::ROM_OFFSET_TYPE]);
		if (RomData::RomType_strings.find(rom_type) == RomData::RomType_strings.end()){
			fprintf(stderr, "Unknown Rom type: %#02x\n", rom_type);
			assert(false);
			return;
		}
		const char* const rom_type_string = RomData::RomType_strings.at(rom_type);
		fprintf(stdout, "Rom Type: %s\n", rom_type_string);

		uint16_t rom_bank_count = RomData::ROM_SIZE_TO_BANK_COUNT.at(rom[RomData::ROM_OFFSET_ROM_SIZE]);
		uint64_t rom_total_size = rom_bank_count * RomData::ROM_BANK_SIZE;
		fprintf(stdout, "Rom Bank Count: %du (Total Size: %lu bytes)\n", rom_bank_count, rom_total_size);
		
		if (rom.size() != rom_total_size){
			fprintf(stderr, "Invalid Rom Size %lu, expected %lu\n", rom.size(), rom_total_size);
			assert(false);
			return;
		}

		uint8_t ram_bank_count = RomData::RAM_SIZE_TO_BANK_COUNT.at(rom[RomData::ROM_OFFSET_RAM_SIZE]);
		fprintf(stdout, "Extra Ram Pages in cartridge: %d\n", ram_bank_count);

		switch(rom_type){
		case RomData::ROM_PLAIN:
			// TODO: Plain MBC
		case RomData::ROM_MBC1:
		case RomData::ROM_MBC1_RAM:
			mbc = std::make_unique<MBC1>(rom, rom_bank_count, ram_bank_count);
			break;
		case RomData::ROM_MBC3_RAM_BATT:
			mbc = std::make_unique<MBC3>(rom, rom_bank_count, ram_bank_count);
			break;
		default:
			assert(false);
		}
	}
	void Cartridge::reset(){
		mbc->reset();
	}
}
