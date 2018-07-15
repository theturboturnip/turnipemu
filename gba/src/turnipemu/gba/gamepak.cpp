#include "turnipemu/config.h"
#include "turnipemu/log.h"
#include "turnipemu/gba/gamepak.h"

#include <regex>

namespace TurnipEmu::GBA{
	GamePak::GamePak(std::vector<byte>& rom) : rom(rom) {
		assert(rom.size() >= 0x400000 && (rom.size() % 0x100000) == 0);
		assert(rom[0xB2] == 0x96);
		
		memcpy(&gameInfo.title, rom.data() + 0xA0, 12);
		gameInfo.title[12] = '\0';
		memcpy(&gameInfo.gameCode, rom.data() + 0xAC, 4);
		gameInfo.gameCode[4] = '\0';
		memcpy(&gameInfo.makerCode, rom.data() + 0xB0, 2);
		gameInfo.makerCode[2] = '\0';

		gameInfo.version = rom[0xBC];

		// Backup Type detection, only works for ROMs built with Nintendo libraries
		std::regex regexes[] = {
			std::regex("EEPROM_V\\d\\d\\d"),
			std::regex("SRAM_V\\d\\d\\d"),
			std::regex("FLASH(512)?_V\\d\\d\\d"),
			std::regex("FLASH1M_V\\d\\d\\d")
		};
		for (int i = 0; i < 4; ++i){
			if (std::regex_search(reinterpret_cast<char*>(rom.data()), reinterpret_cast<char*>(&rom.back()), regexes[i])){
				backupType = static_cast<BackupType>(i);
				break;
			}
		}

		if (StaticConfig::DebugRomData){
			LogLine("ROM", "Successfully loaded ROM");
			LogLine("ROM", "Title: %s", gameInfo.title);
			LogLine("ROM", "Game Code: %s", gameInfo.gameCode);
			LogLine("ROM", "Maker Code: %s", gameInfo.makerCode);
			LogLine("ROM", "Version: %d", gameInfo.version);
			LogLine("ROM", "Backup Type: %s", backupTypeStrings[(int)backupType]);
		}
	}
}
