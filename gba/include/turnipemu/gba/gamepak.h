#pragma once

#include <vector>

#include "turnipemu/display.h"
#include "turnipemu/types.h"
#include "turnipemu/memory/map.h"

namespace TurnipEmu::GBA{
	class GamePak : public Memory::RangeController, public Display::CustomWindow {
	public:
		GamePak(std::vector<byte>& rom); // TODO: Define

		// TODO: Define memory functions. The cycles will be wrong, and the backup data is writeable
		bool allowRead(uint32_t address) const override {
			return true;
		}
		byte read(uint32_t address) const override {
			uint32_t rel_address = address - startAddress;
			if (rel_address < 0x0E000000){
				// The ROM is mirrored 3 times
				// Wait State 0 (0x08000000 - 0x09FFFFFF)
				// Wait State 1 (0x0A000000 - 0x0BFFFFFF)
				// Wait State 2 (0x0C000000 - 0x0DFFFFFF)
				return rom[rel_address % 0x02000000];
			}else{
				return backup[rel_address - 0x0E000000];
			}
		}

		void drawCustomWindowContents() override;
		
	protected:
		struct {
			char title[12 + 1];
			char gameCode[4 + 1];
			char makerCode[2 + 1];
			byte version;
		} gameInfo;
		
		std::vector<byte> rom; // Max 32MB, commonly 4MB or 8MB

		enum class BackupType {
			eEEPROM = 0,   // EEPROM, 512B OR 8KB
			eSRAM = 1,     // SRAM, 32KB
			eFlash512 = 2, // FLASH, 64KB (i.e. 512Kb)
			eFlash1M = 3,  // FLASH, 128KB, (i.e. 1Mb)
			eUnknown = 4
		};
		static char const * const backupTypeStrings[5];
			
		BackupType backupType = BackupType::eUnknown;
		std::vector<byte> backup; 
	};
}
