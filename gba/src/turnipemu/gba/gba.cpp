#include "turnipemu/gba/gba.h"

#include "turnipemu/log.h"

namespace TurnipEmu::GBA{
	GBA::GBA(Display& display, std::vector<byte> biosData, GamePak gamePak) : Emulator(display, "GBA"), memoryMap(*this), cpu(*this, memoryMap), bios(biosData, 0x0), gamePak(gamePak) {
		display.registerCustomWindow(this, &this->gamePak);
		display.registerCustomWindow(this, &this->cpu.debugStateWindow);

		memoryMap.registerMemoryController(&this->bios);
		memoryMap.registerMemoryController(&this->gamePak);
		memoryMap.registerMemoryController(&this->io.lcdEngine);
		memoryMap.registerMemoryController(&this->io.dmaEngine);
		memoryMap.registerMemoryController(&this->io.timerEngine);
		memoryMap.registerMemoryController(&this->io.unusedMemory);
		memoryMap.registerMemoryController(&this->systemControl);
		memoryMap.registerMemoryController(&this->interruptControl);
		memoryMap.registerMemoryController(&this->iram);

		reset();
	}

	void GBA::tick(){
		try {
			cpu.tick();
			if (io.dmaEngine.canExecute()){
				io.dmaEngine.execute(memoryMap);
			}
			if (io.timerEngine.canExecute()){
				io.timerEngine.execute(interruptControl);
			}
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
