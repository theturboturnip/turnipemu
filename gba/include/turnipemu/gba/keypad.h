#pragma once

#include "turnipemu/types.h"
#include "turnipemu/memory/controllers.h"

namespace TurnipEmu::GBA {
	class InterruptControl;
	
	class Keypad : public Memory::RangeController {
	public:
		Keypad();

		enum class Button : halfword {
			A = 1 << 0,
			B = 1 << 1,
			Select = 1 << 2,
			Start = 1 << 3,
			Right = 1 << 4,
			Left = 1 << 5,
			Up = 1 << 6,
			Down = 1 << 7,
			R = 1 << 8,
			L = 1 << 9,
		};

		void setKeysPressed(halfword pressedKeys, InterruptControl&);

		bool allowRead(uint32_t address) const override;
		byte read(uint32_t address) const override;
		bool allowWrite(uint32_t address) const override;
		void write(uint32_t address, byte value) override;

	private:
		// TODO: Figure out a nicer way to do this?
		// Reading a byte of this in an endian-independent fashion requires shifts. Would be nice to not have to do that.
		halfword releasedKeys; // When reading the key register a value of 0 for a key means it is pressed, => for a value of 1 the key is released.

#pragma pack(1)
		union InterruptSettings {
			byte data[2];
			struct {
				// TODO: Is this endian-independent? Consider a macro that could auto define a > byte quantity in terms of bytes and generate an inline function to read it.
				halfword selectedKeys : 10; // Bits 0-9
				uint8_t dummy : 4; // Bits 10-13
				bool irqOnPressed : 1; // Bit 14. If set, an interrupt is fired when the selected keys are pressed, otherwise no interrupt is fired.
				bool irqCondition : 1; // Bit 15. False=fire interrupt when ANY key is pressed, True=fire interrupt when ALL keys are pressed.
			};
		} interruptSettings;
		static_assert(sizeof(InterruptSettings) == 2);
#pragma pack()
	};
}
