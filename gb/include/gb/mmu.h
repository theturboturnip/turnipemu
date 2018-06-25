// Copyright Samuel Stark 2017

#pragma once

#include <cstdint>
#include <array>
#include <vector>

#include "rom_data.h"

namespace GB{
	class CPU;

	class MMU{
	public:
		constexpr static uint16_t BIOS_SIZE = 0x100;

		constexpr static uint16_t TIMER_DIVIDER_ADDRESS = 0xFF04;
		constexpr static uint16_t TIMER_COUNTER_ADDRESS = 0xFF05;
		constexpr static uint16_t TIMER_MODULO_ADDRESS = 0xFF06;
		constexpr static uint16_t TIMER_CONTROL_ADDRESS = 0xFF07;
		
		constexpr static uint16_t INTERRUPTS_FLAGGED_ADDRESS = 0xFF0F;
		constexpr static uint16_t INTERRUPTS_ENABLED_ADDRESS = 0xFFFF;

		constexpr static uint16_t GPU_CONTROL_ADDRESS = 0xFF40;
		constexpr static uint16_t GPU_LCDC_STATUS_ADDRESS = 0xFF41;
		constexpr static uint16_t GPU_SCROLL_Y_ADDRESS = 0xFF42;
		constexpr static uint16_t GPU_SCROLL_X_ADDRESS = 0xFF43;
		constexpr static uint16_t GPU_SCANLINE_ADDRESS = 0xFF44;
		constexpr static uint16_t GPU_SCANLINE_INTERRUPT_VALUE_ADDRESS = 0xFF45;
		constexpr static uint16_t DMA_TRANSFER_TO_OAM_ADDRESS = 0xFF46;
		constexpr static uint16_t GPU_BG_PALETTE_ADDRESS = 0xFF47;
		constexpr static uint16_t GPU_SPRITE_PALETTE_0_ADDRESS = 0xFF48;
		constexpr static uint16_t GPU_SPRITE_PALETTE_1_ADDRESS = 0xFF49;
		constexpr static uint16_t GPU_WINDOW_Y_ADDRESS = 0xFF4A;
		constexpr static uint16_t GPU_WINDOW_X_ADDRESS = 0xFF4B;

		constexpr static uint16_t INPUT_JOYPAD_ADDRESS = 0xFF00;
	
		constexpr static uint16_t ROM_BANK_ZERO_START = 0x0;
		constexpr static uint16_t ROM_BANK_ONE_START = 0x4000;
		constexpr static uint16_t GPU_VRAM_START = 0x8000;
		constexpr static uint16_t EXT_RAM_START = 0xA000;
		constexpr static uint16_t INT_RAM_START = 0xC000;
		constexpr static uint16_t INT_RAM_MIRROR_START = 0xE000;
		constexpr static uint16_t GPU_SPRITE_INFO_START = 0xFE00;
		constexpr static uint16_t IO_RAM_START = 0xFF00;
		constexpr static uint16_t ZP_RAM_START = 0xFF80;
	
	public:
		MMU(CPU& cpu);
	
		void load_bios(std::array<uint8_t, BIOS_SIZE> bios_data);
		void unload_bios();

		void load_rom(std::vector<uint8_t> rom_data, uint8_t ram_bank_count);
		void switch_bank_one(uint16_t bank_index);

		void step();
		void reset();
	
		void write_byte(uint16_t address, uint8_t byte);
		void write_word(uint16_t address, uint16_t word);

		uint8_t read_byte(uint16_t address);
		uint16_t read_word(uint16_t address);

		constexpr static int OAM_DMA_LENGTH = 160; // 671 cycles
		int dma_timer = -1;
	protected:
		CPU& cpu;
	
		// The BIOS. Known size.
		// Until the BIOS is finished running (i.e. when unload_bios() is called and use_bios is set to false),
		// 0x0 to (BIOS_SIZE - 1) maps to the BIOS instead of the ROM.
		std::array<uint8_t, BIOS_SIZE> bios;
		bool use_bios = true;

		// The ROM. Unknown size, can be a multiple of 16kB long. 
		/*std::vector<uint8_t> rom;
		bool has_rom = false;
		// The ROM can be addressed in two "banks".
		// Bank 0 (0x0 to 0x3FFF) is always the first 16kB of the ROM (unless the BIOS is loaded, see previous comment);
		// but Bank 1 (0x4000 to 0x7FFF) can be any subsequent 16kB block of the ROM.
		// This is the index in the ROM where Bank 1 starts.
		uint16_t rom_bank_one_mapped_index = RomData::ROM_BANK_SIZE * 1;
		// [GPU] 0x8000 to 0x9FFF contains sprite data
		// 0xA000 to 0xBFFF, extra bits of ram that can be mapped from the cartridge
		std::vector<std::array<uint8_t, 0x2000>> ext_ram_banks;
		uint8_t ext_ram_bank = 0;
		bool ext_ram_enabled = false;*/
		
		// 0xC000 to 0xDFFF, the ram inside the gameboy. 0xE000 to 0xFDFF also shadows 0xC000 to 0xDDFF.
		std::array<uint8_t, 0x2000> int_ram;
		// [GPU] 0xFE00 to 0xFE9F contains the information about the 40 sprites that should be rendered
		// 0xFF00 to 0xFF7F, RAM used to communicate with I/O
		std::array<uint8_t, 0x100> io_ram;
		// 0xFF80 to 0xFFFF, Zero-Page RAM. It's fast, I guess?
		//std::array<uint8_t, (0xFFFF - 0xFF80)> zero_page_ram;

		uint8_t* map_address(uint16_t address); // Converts an address into a byte pointer
	};
}
