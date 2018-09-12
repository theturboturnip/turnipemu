#pragma once

#include "turnipemu/types.h"

#include <array>

namespace TurnipEmu::GBA {
	template<size_t RangeStart, size_t RangeEnd, size_t Size>
	class MirroredRangeController : public Memory::RangeController {
	public:
		MirroredRangeController() : Memory::RangeController(RangeStart, RangeEnd) {
			data.fill(0);
		}

		bool allowRead(uint32_t address) const override {
			return true;
		}
		byte read(uint32_t address) const override {
			return data[(address - RangeStart) % Size];
		}
		bool allowWrite(uint32_t address) const override {
			return true;
		}
		void write(uint32_t address, uint8_t value) override {
			data[(address - RangeStart) % Size] = value;
		}
	protected:
		std::array<byte, Size> data;
	};


	using OnBoardRam = MirroredRangeController<0x0'0200'0000, 0x0'0300'0000, 0x4'0000>;
	using OnChipRam  = MirroredRangeController<0x0'0300'0000, 0x0'0400'0000, 0x0'8000>;
}
