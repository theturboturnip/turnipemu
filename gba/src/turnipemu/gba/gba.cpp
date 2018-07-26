#include "turnipemu/gba/gba.h"

#include "turnipemu/log.h"

namespace TurnipEmu::GBA{
	GBA::GBA(Display& display, std::vector<byte> biosData, GamePak gamePak) : Emulator(display, "GBA"), memoryMap(*this), cpu(memoryMap), bios(biosData, 0x0), gamePak(gamePak) {
		display.registerCustomWindow(this, &this->gamePak);
		display.registerCustomWindow(this, &this->cpu);

		memoryMap.registerMemoryController(&this->bios);
		memoryMap.registerMemoryController(&this->gamePak);
		memoryMap.registerMemoryController(&this->io.lcdEngine);
		memoryMap.registerMemoryController(&this->systemControl);

		reset();
	}

	void GBA::tick(){
		try {
			cpu.tick();
		} catch (const std::exception& e) {
			LogLine(logTag, "Exception encountered, stopping...");
			LogLine(logTag, "%s", e.what());
			stop(std::string(e.what()));
		}
	}

	void GBA::reset(){
		LogLine(logTag, "GBA Reset");
		Emulator::reset();
		paused = true;
		cpu.reset();
		systemControl.reset();
	}
	void GBA::reset(GamePak newGamePak){
		gamePak = newGamePak;
		reset();
	}
}
