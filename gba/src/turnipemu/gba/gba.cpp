#include "turnipemu/gba/gba.h"
#include "turnipemu/log.h"

#include "imgui/imgui.h"

namespace TurnipEmu::GBA{
	GBA::GBA(Display& display, GamePak gamePak) : Emulator(display, "GBA"), memoryMap(*this), cpu(memoryMap), gamePak(gamePak) {
		//display.registerCustomWindow(cpu);
		display.registerCustomWindow(this, &this->gamePak);
		display.registerCustomWindow(this, &this->cpu);

		memoryMap.registerMemoryController(&this->gamePak);

		reset();
	}

	void GBA::tick(){
		paused = true;
	}

	void GBA::reset(){
		LogLine(logTag, "GBA Reset");
		Emulator::reset();
		paused = true;
	}
	void GBA::reset(GamePak newGamePak){
		gamePak = newGamePak;
		reset();
	}
}
