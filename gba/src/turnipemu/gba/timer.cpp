#include "turnipemu/gba/timer.h"
#include "turnipemu/log.h"

namespace TurnipEmu::GBA{
	TimerEngine::TimerEngine()
		: Memory::RangeController(0x0'0400'0100, 0x0'0400'0110) {}
	
	void TimerEngine::execute(InterruptControl& interrupts){
		if (timers[0].enabled() || timers[1].enabled() || timers[2].enabled() || timers[3].enabled())
			throw std::runtime_error("Timers have not been implemented!");
	}
	
	bool TimerEngine::allowRead(uint32_t address) const {
		return true;
	}
	byte TimerEngine::read(uint32_t address) const {
		const uint32_t deltaAddress = (address - 0x0'0400'0100);
		const uint32_t timerIndex = deltaAddress / 4;
		const uint32_t byteIndex = deltaAddress % 4;
		switch(byteIndex){
		case 0:
			return (timers[timerIndex].counter >> 0) & 0xF;
		case 1:
			return (timers[timerIndex].counter >> 8) & 0xF;
		case 2:
		case 3:
			return timers[timerIndex].control.data[byteIndex - 2];
		}
		assert(false);
		return 0x0;
	}
	bool TimerEngine::allowWrite(uint32_t address) const {
		return true;
	}
	void TimerEngine::write(uint32_t address, byte value) {
		const uint32_t deltaAddress = (address - 0x0'0400'0100);
		const uint32_t timerIndex = deltaAddress / 4;
		const uint32_t byteIndex = deltaAddress % 4;
		switch(byteIndex){
		case 0:
			timers[timerIndex].reload = (timers[timerIndex].reload & 0xF0) | (value << 0);
			break;
		case 1:
			timers[timerIndex].reload = (timers[timerIndex].reload & 0x0F) | (value << 8);
			break;
		case 2:
		case 3:
			timers[timerIndex].control.data[byteIndex - 2] = value;
			break;
		}
	}
}
