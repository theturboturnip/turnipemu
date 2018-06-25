#include "gb/mbc.h"
#include "gb/cpu.h"
#include <assert.h>

namespace GB{
	void MBC3::write_rom_byte(uint16_t address, uint8_t byte){
		if (CPU::extended_debug_data)
			fprintf(stdout, "Writing 0x%02x to ROM address 0x%04x\n", byte, address);

		if (address >= 0x6000){
			if (byte == 0b0)
				latch_change_status = 0b0;
			else if (byte == 0b1 && latch_change_status == 0b0){
				latched = !latched;
				latch_change_status = 0b1;
			}
		}else if (address >= 0x4000){
			rtc_mapped = byte >= 0x8;
			ram_bank_and_rtc_number = byte;
		}else if (address >= 0x2000){
			selected_rom_bank = byte;
			if (selected_rom_bank == 0) selected_rom_bank = 1;
		}else if (address >= 0x0000){
			ram_or_rtc_enabled = (byte == 0xA);
		}else{
			assert(false);
		}
	}

	void MBC3::reset(){
		ram_or_rtc_enabled = false;
		rtc_mapped = false;
		ram_bank_and_rtc_number = 0;
		
		latched = false;
		latch_change_status = 1;

		selected_rom_bank = 1;
	}

	uint8_t MBC3::read_ram_byte(uint16_t relative_address){
		assert(ram_or_rtc_enabled);
		if (rtc_mapped){
			// TODO: RTC (Remember to respect latching)
			return 0xFF;
		}else{
			return ram_banks[selected_ram_bank][relative_address];
		}
	}

	void MBC3::write_ram_byte(uint16_t relative_address, uint8_t byte){
		assert(ram_or_rtc_enabled);
		if (rtc_mapped){
			// TODO: RTC (Remember to handle the flags correctly, latching doesn't apply)
		}else{
			ram_banks[selected_ram_bank][relative_address] = byte;
		}
	}
}
