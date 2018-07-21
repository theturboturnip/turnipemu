#pragma once

#include "types.h"
#include "memory.h"
#include "arm7tdmi.h"
#include "dma.h"
#include "gamepak.h"
#include "lcd.h"
#include "turnipemu/display.h"
#include "turnipemu/emulator.h"

namespace TurnipEmu::GBA{
	class GBA : public Emulator {
	public:
		GBA(Display& display, GamePak gamePak);

		void tick() override;
		
		void reset() override;
		void reset(GamePak newGamePak);
		
	protected:
		MemoryMap memoryMap;

		ARM7TDMI::CPU cpu;
		
		GamePak gamePak;
		//struct {
			//LCDEngine lcdEngine; // TODO: Define fully, should control both the Internal Display Memory (0x05000000 - 0x07000400) and the LCD Control Registers (0x04000000 - 0x04000060)
			//SoundEngine soundEngine; // TODO: Define
			//DMAEngine dmaEngine; // TODO: Define behaviour
			//Timer timer; // TODO: Define
			//SerialEngine serialEngine; // TODO: Define. Can be a no-op, serial support isn't planned for now
			//Keypad keypad; // TODO: Define
			//MiscControls miscControls; // TODO: Define
		//} io;

		const char* const logTag = "GBA";
	};
}
