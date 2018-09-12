#pragma once

#include "turnipemu/types.h"
#include "turnipemu/memory/map.h"

namespace TurnipEmu::GBA{
	class SoundEngine : public Memory::RangeController, public Memory::NoopController {
	public:
		SoundEngine() : Memory::RangeController(0x0'0400'0060, 0x0'0400'00B0), Memory::NoopController(0){}

		const std::vector<uint8_t>& generateSamples(size_t count);
		
		// TODO: Define the registers and stuff. This shouldn't be a NoopController
		
	private:
		std::vector<uint8_t> samples;
	};
}
