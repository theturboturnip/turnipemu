#include "gb/mbc.h"
#include "gb/cpu.h"

#include <cstdio>

namespace GB{
	void MBC1::write_rom_byte(uint16_t address, uint8_t byte){
		if (CPU::extended_debug_data)
			fprintf(stdout, "Writing 0x%02x to ROM address 0x%04x\n", byte, address);
		if (address >= 0x6000){
			mode_select_ram = ((byte & 0x1) == 1);

			// These do the exact same things
			if (mode_select_ram){
				selected_rom_bank.top_two = 0;
			}else{
				selected_ram_bank = 0;
			}
		}else if (address >= 0x4000){
			if (mode_select_ram)
				selected_ram_bank = byte;
			else
				selected_rom_bank.top_two = byte;
		}else if (address >= 0x2000){
			if (byte == 0) byte = 1;
			selected_rom_bank.bottom_five = byte;
			if (CPU::extended_debug_data)
				fprintf(stdout, "Changed ROM bank to %d\n", static_cast<uint8_t>(selected_rom_bank));
		}else if (address >= 0x0000){
			enabled_ram = (byte == 0xA);
		}else{
			assert(false);
		}
	}

	uint8_t MBC1::read_ram_byte(uint16_t relative_address){
		assert(enabled_ram);
		return ram_banks[selected_ram_bank][relative_address];
	}
	void MBC1::write_ram_byte(uint16_t relative_address, uint8_t byte){
		assert(enabled_ram);
		ram_banks[selected_ram_bank][relative_address] = byte;
	}
	
	void MBC1::reset(){
		enabled_ram = false;
		mode_select_ram = false;
		selected_rom_bank.bottom_five = 1;
		selected_ram_bank = 0; // Also sets top two for rom bank
	}
}
