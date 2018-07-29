#pragma once

#include "turnipemu/display.h"
#include "turnipemu/emulator.h"
#include "turnipemu/types.h"
#include "turnipemu/arm7tdmi/cpu.h"
#include "turnipemu/gba/dma.h"
#include "turnipemu/gba/gamepak.h"
#include "turnipemu/gba/lcd.h"
#include "turnipemu/gba/sys_control.h"
#include "turnipemu/gba/interrupt_control.h"
#include "turnipemu/memory/map.h"

namespace TurnipEmu::GBA{
	class GBA : public Emulator {
	public:
		GBA(Display& display, std::vector<byte> biosData, GamePak gamePak);

		void tick() override;
		
		void reset() override;
		void reset(GamePak newGamePak);		
	protected:
		Memory::Map memoryMap;
		
		ARM7TDMI::CPU cpu;
		
		Memory::StaticDataRangeController<std::vector<byte>> bios;
		GamePak gamePak;
		struct {
			LCDEngine lcdEngine; // TODO: Define fully
			//SoundEngine soundEngine; // TODO: Define
			//DMAEngine dmaEngine; // TODO: Define behaviour
			//Timer timer; // TODO: Define
			//SerialEngine serialEngine; // TODO: Define. Can be a no-op, serial support isn't planned for now
			//Keypad keypad; // TODO: Define
		} io;
		SystemControl systemControl;
		InterruptControl interruptControl;

		const char* const logTag = "GBA";
	};
}
