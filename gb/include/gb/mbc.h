#pragma once

#include <cstdint>
#include <vector>
#include <array>
#include <assert.h>

namespace GB{
	class MBC{
	public:
		MBC(std::vector<uint8_t> rom_data, uint8_t rom_bank_count, uint8_t ram_bank_count);
		
		virtual void write_rom_byte(uint16_t address, uint8_t byte){};
		virtual uint8_t read_rom_byte(uint16_t address){
			return 0xFF;
		}
		virtual uint8_t read_ram_byte(uint16_t relative_address){
			return 0xFF;
		};
		virtual void write_ram_byte(uint16_t relative_address, uint8_t byte){}; 

		virtual void reset(){}

		std::vector<std::array<uint8_t, 0x4000>> rom_banks;
		std::vector<std::array<uint8_t, 0x2000>> ram_banks;
	};

	class MBC1 : public MBC{
	public:
		using MBC::MBC;
		
		void write_rom_byte(uint16_t address, uint8_t byte) override;
		uint8_t read_rom_byte(uint16_t address) override{
			return address < 0x4000 ? rom_banks[0][address] : rom_banks[selected_rom_bank][address - 0x4000];
		}
		uint8_t read_ram_byte(uint16_t relative_address) override;
		void write_ram_byte(uint16_t relative_address, uint8_t byte) override; 
		void reset() override;

	private:
		bool enabled_ram;

		union{
			struct{
				uint8_t bottom_five : 5;
				uint8_t top_two : 2;
				uint8_t dummy : 1; // Bit 7

				operator uint8_t(){
					return top_two << 5 | bottom_five;
				}
			} selected_rom_bank;
			struct{
				uint8_t dummy : 5;
				uint8_t selected_ram_bank : 2;
				uint8_t dummy_1 : 1;
			};
		};
		// If true, use RAM banking mode (Can use RAM banks 0-3, ROM Banks 0x00 - 0x1F)
		// If false, use ROM banking mode (Can use RAM bank 0, ROM Banks 0-128)
		bool mode_select_ram;
	};

	class MBC3 : public MBC{
		using MBC::MBC;

		void write_rom_byte(uint16_t address, uint8_t byte) override;
		uint8_t read_rom_byte(uint16_t address) override{
			return address < 0x4000 ? rom_banks[0][address] : rom_banks[selected_rom_bank][address - 0x4000];
		}
		uint8_t read_ram_byte(uint16_t relative_address) override;
		void write_ram_byte(uint16_t relative_address, uint8_t byte) override; 
		void reset() override;

	private:
		bool ram_or_rtc_enabled;
		bool rtc_mapped; // if false, ram is mapped
		union{
			uint8_t selected_ram_bank;
			uint8_t rtc_register;
			uint8_t ram_bank_and_rtc_number;
		};

		uint8_t selected_rom_bank : 7; // Reset to 1

		bool latched;
		uint8_t latch_change_status : 1; // RESET TO 0b1!!!
	};
}
