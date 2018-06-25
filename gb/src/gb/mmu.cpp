// Copyright Samuel Stark 2017

#include "gb/mmu.h"
#include "gb/input.h"
#include "gb/cpu.h"

#include <assert.h>
#include <memory>

namespace GB{
	MMU::MMU(CPU& cpu) : cpu(cpu) {}

	void MMU::load_bios(std::array<uint8_t, BIOS_SIZE> bios_data){
		bios = std::move(bios_data);
		constexpr bool bios_dump = false;
		if (bios_dump){
			uint16_t address = 0;
			for (uint8_t value : bios){
				fprintf(stdout, "0x%04x: 0x%02x\n", address, value);
				address++;
			}
		}
		use_bios = true;
	}
	void MMU::unload_bios(){
		use_bios = false;
	}

	void MMU::step(){
		if (dma_timer >= 0){
			dma_timer -= cpu.clock_cycles_this_step;
		}
	}
	void MMU::reset(){
		int_ram.fill(0);
		io_ram.fill(0);
	
		write_byte(0xFF05, 0);
		write_byte(0xFF06, 0);
		write_byte(0xFF07, 0);
		write_byte(0xFF10, 0x80);
		write_byte(0xFF11, 0xBF);
		write_byte(0xFF12, 0xF3);
		write_byte(0xFF14, 0xBF);
		write_byte(0xFF16, 0x3F);
		write_byte(0xFF17, 0x00);
		write_byte(0xFF19, 0xBF);
		write_byte(0xFF1A, 0x7A);
		write_byte(0xFF1B, 0xFF);
		write_byte(0xFF1C, 0x9F);
		write_byte(0xFF1E, 0xBF);
		write_byte(0xFF20, 0xFF);
		write_byte(0xFF21, 0x00);
		write_byte(0xFF22, 0x00);
		write_byte(0xFF23, 0xBF);
		write_byte(0xFF24, 0x77);
		write_byte(0xFF25, 0xF3);
		write_byte(0xFF26, 0xF1);
		write_byte(0xFF40, 0x91);
		write_byte(0xFF42, 0x00);
		write_byte(0xFF43, 0x00);
		write_byte(0xFF45, 0x00);
		write_byte(0xFF47, 0xFC);
		write_byte(0xFF48, 0xFF);
		write_byte(0xFF49, 0xFF);
		write_byte(0xFF4A, 0x00);
		write_byte(0xFF4B, 0x00);
		write_byte(0xFFFF, 0x00);

		// TODO: Check if this needs to write to the fast HRAM too
		io_ram = {{
				0x0F, 0x00, 0x7C, 0xFF, 0x00, 0x00, 0x00, 0xF8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x01,
				0x80, 0xBF, 0xF3, 0xFF, 0xBF, 0xFF, 0x3F, 0x00, 0xFF, 0xBF, 0x7F, 0xFF, 0x9F, 0xFF, 0xBF, 0xFF,
				0xFF, 0x00, 0x00, 0xBF, 0x77, 0xF3, 0xF1, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF, 0x00, 0xFF,
				0x91, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFC, 0x00, 0x00, 0x00, 0x00, 0xFF, 0x7E, 0xFF, 0xFE,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0x3E, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xC0, 0xFF, 0xC1, 0x00, 0xFE, 0xFF, 0xFF, 0xFF,
				0xF8, 0xFF, 0x00, 0x00, 0x00, 0x8F, 0x00, 0x00, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
				0xCE, 0xED, 0x66, 0x66, 0xCC, 0x0D, 0x00, 0x0B, 0x03, 0x73, 0x00, 0x83, 0x00, 0x0C, 0x00, 0x0D,
				0x00, 0x08, 0x11, 0x1F, 0x88, 0x89, 0x00, 0x0E, 0xDC, 0xCC, 0x6E, 0xE6, 0xDD, 0xDD, 0xD9, 0x99,
				0xBB, 0xBB, 0x67, 0x63, 0x6E, 0x0E, 0xEC, 0xCC, 0xDD, 0xDC, 0x99, 0x9F, 0xBB, 0xB9, 0x33, 0x3E,
				0x45, 0xEC, 0x52, 0xFA, 0x08, 0xB7, 0x07, 0x5D, 0x01, 0xFD, 0xC0, 0xFF, 0x08, 0xFC, 0x00, 0xE5,
				0x0B, 0xF8, 0xC2, 0xCE, 0xF4, 0xF9, 0x0F, 0x7F, 0x45, 0x6D, 0x3D, 0xFE, 0x46, 0x97, 0x33, 0x5E,
				0x08, 0xEF, 0xF1, 0xFF, 0x86, 0x83, 0x24, 0x74, 0x12, 0xFC, 0x00, 0x9F, 0xB4, 0xB7, 0x06, 0xD5,
				0xD0, 0x7A, 0x00, 0x9E, 0x04, 0x5F, 0x41, 0x2F, 0x1D, 0x77, 0x36, 0x75, 0x81, 0xAA, 0x70, 0x3A,
				0x98, 0xD1, 0x71, 0x02, 0x4D, 0x01, 0xC1, 0xFF, 0x0D, 0x00, 0xD3, 0x05, 0xF9, 0x00, 0x0B, 0x00
			}};
	

		use_bios = true;
	}

	void MMU::write_byte(uint16_t address, uint8_t byte){
		if (dma_timer >= 0 && address < ZP_RAM_START){
			cpu.stopped = true;
			fprintf(stderr, "Tried to write to 0x%04x (i.e. outside of HRAM) before the DMA transfer had finished! DMA Status: %d\n", address, dma_timer);
			return;
		}
		if (address < GPU_VRAM_START){
			cpu.cartridge.mbc->write_rom_byte(address, byte);
		}else if (address < INT_RAM_START && address >= EXT_RAM_START){
			return cpu.cartridge.mbc->write_ram_byte(address, byte);
		}else if (address == INPUT_JOYPAD_ADDRESS){
			cpu.input.set_value(byte);
		}else if (address == GPU_LCDC_STATUS_ADDRESS){
			cpu.gpu.set_lcdc_status(byte);
		}else if (address == DMA_TRANSFER_TO_OAM_ADDRESS){
			if (byte < 0xF1){
				uint16_t start = (byte << 8);
				for (uint8_t offset = 0; offset < 0xA0; ++offset){
					cpu.gpu.spriteinfo[offset] = *(map_address(start + offset));
				}
			}else{
				fprintf(stderr, "Invalid value 0x%02x written to DMA address!\n", byte);
			}
			dma_timer = OAM_DMA_LENGTH;
		}else if (address == INTERRUPTS_FLAGGED_ADDRESS){
			cpu.interrupts.flagged = byte;
			cpu.interrupts.find_next_interrupt();
		}else if (address == INTERRUPTS_ENABLED_ADDRESS){
			cpu.interrupts.enabled = byte;
			cpu.interrupts.find_next_interrupt();
			if (CPU::extended_debug_data)
				fprintf(stdout, "Set enabled interrupts to 0x%02x\n", byte);
		}else if (address == TIMER_DIVIDER_ADDRESS){
			cpu.timer.reset_divider();
		}else if (address == TIMER_COUNTER_ADDRESS){
			cpu.timer.write_counter(byte);
		}else if (address == TIMER_MODULO_ADDRESS){
			cpu.timer.write_modulo(byte);
		}else if (address == TIMER_CONTROL_ADDRESS){
			cpu.timer.write_control(byte);
		}else{
			*(map_address(address)) = byte;
		}
	}
	void MMU::write_word(uint16_t address, uint16_t word){
		write_byte(address, static_cast<uint8_t>(word & 0x00ff));
		write_byte(address + 1, static_cast<uint8_t>((word & 0xff00) >> 8));
	}

	uint8_t MMU::read_byte(uint16_t address){
		if (dma_timer >= 0 && address < ZP_RAM_START){
			cpu.stopped = true;
			fprintf(stderr, "Tried to read from 0x%04x (i.e. outside of HRAM) before the DMA transfer had finished! DMA Status: %d\n", address, dma_timer);
			return 0xFF;
		}

		if (address < GPU_VRAM_START){
			return cpu.cartridge.mbc->read_rom_byte(address);
		}else if (address < INT_RAM_START && address >= EXT_RAM_START){
			return cpu.cartridge.mbc->read_ram_byte(address);
		}else if (address == INPUT_JOYPAD_ADDRESS){
			return cpu.input.get_value();
		}else if (address == GPU_LCDC_STATUS_ADDRESS){
			return cpu.gpu.get_lcdc_status();
		}else if (address == INTERRUPTS_FLAGGED_ADDRESS){
			return cpu.interrupts.flagged;
		}else if (address == INTERRUPTS_ENABLED_ADDRESS){
			return cpu.interrupts.enabled;
		}else if (address == TIMER_DIVIDER_ADDRESS){
			return cpu.timer.read_divider();
		}else if (address == TIMER_COUNTER_ADDRESS){
			return cpu.timer.read_counter();
		}else if (address == TIMER_MODULO_ADDRESS){
			return cpu.timer.read_modulo();
		}else if (address == TIMER_CONTROL_ADDRESS){
			return cpu.timer.read_control();
		}
		return *(map_address(address));
	}
	uint16_t MMU::read_word(uint16_t address){
		auto first_byte = read_byte(address);
		auto second_byte = read_byte(address + 1);
		return static_cast<uint16_t>(first_byte) | (static_cast<uint16_t>(second_byte) << 8);
	}

    // Return a pointer to a byte in memory
	uint8_t* MMU::map_address(uint16_t address){
		if (use_bios && address < BIOS_SIZE){
			return bios.data() + address;
		}

		//if (address >= ZP_RAM_START){
		//	return zero_page_ram.data() + (address - ZP_RAM_START);
		//}
		if (address >= IO_RAM_START){
			return io_ram.data() + (address - IO_RAM_START);
		}
		if (address >= GPU_SPRITE_INFO_START){
			return cpu.gpu.spriteinfo + (address - GPU_SPRITE_INFO_START);
		}
		if (address >= INT_RAM_MIRROR_START){
			return int_ram.data() + (address - INT_RAM_MIRROR_START);
		}
		if (address >= INT_RAM_START){
			return int_ram.data() + (address - INT_RAM_START);
		}
		if (address >= EXT_RAM_START){
			return nullptr;//return cpu.cartridge.mbc->map_ram_byte(address - EXT_RAM_START);
		}
		if (address >= GPU_VRAM_START){
			return cpu.gpu.vram + (address - GPU_VRAM_START);
		}

		assert(false);
		return nullptr;
	}
}
