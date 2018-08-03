#pragma once

#include "turnipemu/types.h"

#include <array>

namespace TurnipEmu::GBA {
	class InternalRam : public Memory::RangeController {
	public:
		InternalRam() : Memory::RangeController(0x0'0300'0000, 0x0'0400'0000) {
			data.fill(0);
		}

		bool allowRead(uint32_t address) const override {
			return true;
		}
		byte read(uint32_t address) const override {
			// Internal RAM is mirrored every 0x8000 bytes
			return data[(address - 0x0'0300'0000) % 0x8000];
		}
		bool allowWrite(uint32_t address) const override {
			return true;
		}
		void write(uint32_t address, uint8_t value) override {
			data[(address - 0x0'0300'0000) % 0x8000] = value;
		}
	protected:
		std::array<byte, 0x8000> data;
	};
}
