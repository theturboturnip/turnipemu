#include "turnipemu/gba/gba.h"

#include "turnipemu/log.h"

namespace TurnipEmu::GBA{
	GBA::GBA(Display& display, std::vector<byte> biosData, GamePak gamePak) : Emulator(display, "GBA"), memoryMap(*this), cpu(*this, memoryMap), bios(biosData, 0x0), gamePak(gamePak) {
		display.registerCustomWindow(this, &this->gamePak);
		display.registerCustomWindow(this, &this->cpu.debugStateWindow);

		memoryMap.registerMemoryController(&this->bios);
		memoryMap.registerMemoryController(&this->boardRam);
		memoryMap.registerMemoryController(&this->chipRam);
		memoryMap.registerMemoryController(&this->io.lcdEngine);
		memoryMap.registerMemoryController(&this->io.dmaEngine);
		memoryMap.registerMemoryController(&this->io.timerEngine);
		memoryMap.registerMemoryController(&this->io.keypad);
		memoryMap.registerMemoryController(&this->io.soundEngine);
		memoryMap.registerMemoryController(&this->io.unusedMemory);
		memoryMap.registerMemoryController(&this->systemControl);
		memoryMap.registerMemoryController(&this->interruptControl);
		memoryMap.registerMemoryController(&this->gamePak);

		reset();
	}

	void GBA::tick(){
		try {
			cpu.tick();
			LogLineOverwrite(logTag, "Cycles: %d", cpu.totalCycles());
			static auto vSyncTick = cpu.totalCycles();
			if (cpu.totalCycles() - vSyncTick > 100000){
				vsyncReady = true;
				vSyncTick = cpu.totalCycles();
			}else vsyncReady = false;
			io.dmaEngine.execute(memoryMap);
			io.timerEngine.execute(interruptControl);
			// TODO: Handle user input
			io.keypad.setKeysPressed(0, interruptControl);
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
