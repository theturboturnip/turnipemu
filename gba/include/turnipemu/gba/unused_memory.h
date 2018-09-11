#pragma once

#include "turnipemu/memory/controllers.h"

namespace TurnipEmu::GBA {
	class UnusedIOMemoryController : public Memory::NoopController {
		bool ownsAddress(uint32_t address) const override {
			return (0x0'0400'00E0 <= address && address < 0x0'0400'0100) // Spare space between DMA and Timer registers
				|| (0x0'0400'0120 <= address && address < 0x0'0400'0130) // Serial Comm. Area 1
				|| (0x0'0400'0134 <= address && address < 0x0'0400'0200) // Serial Comm. Area 2
				;
		}
	};
}
