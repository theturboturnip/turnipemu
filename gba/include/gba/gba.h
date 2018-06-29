#pragma once

#include "types.h"
#include "memory.h"
#include "arm7tdmi.h"
#include "dma.h"

namespace GBA{
	class GBA{

		void reset();
		void reset(GamePak newGamePak);
		
	protected:
		ARM7TDMI cpu;
		
		MemoryMap memoryMap;

		GamePak gamePak; // TODO: Define. This isn't a pointer because a cartridge should have a state per GBA, in case it's reused by multiple GBA instances
		struct {
			LCDEngine lcdEngine; // TODO: Define, should control both the Internal Display Memory (0x05000000 - 0x07000400) and the LCD Control Registers (0x04000000 - 0x04000060)
			SoundEngine soundEngine; // TODO: Define
			DMAEngine dmaEngine;
			Timer timer; // TODO: Define
			SerialEngine serialEngine; // TODO: Define. Can be a no-op, serial support isn't planned for now
			Keypad keypad; // TODO: Define
			MiscControls miscControls; // TODO: Define
		} io;
	};
}

