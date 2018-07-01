#include "gba/gamepak.h"
#include "gba/config.h"

#include <cstdio>
#include <regex>

namespace GBA{
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
			fprintf(stdout, "Successfully loaded ROM\nTitle: %s\nGame Code: %s\nMaker Code: %s\nVersion: %d\nBackup Type: %s\n", gameInfo.title, gameInfo.gameCode, gameInfo.makerCode, gameInfo.version, backupTypeStrings[(int)backupType]);
		}
	}
}
