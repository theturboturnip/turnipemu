#include "gb/mbc.h"

GB::MBC::MBC(std::vector<uint8_t> rom, uint8_t rom_bank_count, uint8_t ram_bank_count){
	rom_banks.resize(rom_bank_count);
	for (uint16_t i = 0; i < rom_bank_count; i++){
		std::copy(rom.begin() + (i * 0x4000), rom.begin() + ((i + 1) * 0x4000), rom_banks[i].data());
	}

	ram_banks.resize(ram_bank_count);

	reset();
}
