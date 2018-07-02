#pragma once

#include "types.h"
#include "memory.h"
#include "arm7tdmi.h"
#include "dma.h"
#include "gamepak.h"

namespace TurnipEmu::GBA{
	class GBA{
		friend class MemoryMap;

		GBA(GamePak gamePak);
		
		void reset();
		void reset(GamePak newGamePak);
		
	protected:
		ARM7TDMI cpu;
		
		MemoryMap memoryMap;

		GamePak gamePak;
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
