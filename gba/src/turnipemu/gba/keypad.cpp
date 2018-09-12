#include "turnipemu/gba/keypad.h"
#include "turnipemu/gba/interrupt_control.h"

namespace TurnipEmu::GBA{
	Keypad::Keypad()
		: Memory::RangeController(0x0'0400'0130,  0x0'0400'0134) {
	}

	void Keypad::setKeysPressed(halfword pressedKeys, InterruptControl& interrupts){
		releasedKeys = ~pressedKeys;
		if (interruptSettings.irqOnPressed)
			throw std::runtime_error("Keypad Interrupts have not been implemented!");
	}
	
	bool Keypad::allowRead(uint32_t address) const {
		return true;
	}
	byte Keypad::read(uint32_t address) const {
		const uint32_t deltaAddress = (address - 0x0'0400'0130);
		switch(deltaAddress) {
		case 0:
			return (releasedKeys >> 0) & 0xF;
		case 1:
			return (releasedKeys >> 8) & 0xF;
		case 2:
		case 3:
			return interruptSettings.data[deltaAddress - 2];
		}
		assert(false);
		return 0x0;
	}
	bool Keypad::allowWrite(uint32_t address) const {
		const uint32_t deltaAddress = (address - 0x0'0400'0130);
		return deltaAddress >= 2;
	}
	void Keypad::write(uint32_t address, byte value) {
		const uint32_t deltaAddress = (address - 0x0'0400'0130);
		interruptSettings.data[deltaAddress - 2] = value;
	}
}
