#include "turnipemu/gba/interrupt_control.h"
#include "turnipemu/utils.h"

#include <cstring>

namespace TurnipEmu::GBA{  
	bool InterruptControl::ownsAddress(uint32_t address) const {
		return (0x0'0400'0200 <= address || address < 0x0'0400'0204) ||
			(0x0'0400'0208 <= address || address < 0x0'0400'0212);
	}
	bool InterruptControl::allowRead(uint32_t address) const {
		return true;
	}
	byte InterruptControl::read(uint32_t address) const {
		if (address == 0x0'0400'0200){
			return enabledInterrupts.data[0];
		}else if (address == 0x0'0400'0201){
			return enabledInterrupts.data[1];
		}else if (address == 0x0'0400'0202){
			return flaggedInterrupts.data[0];
		}else if (address == 0x0'0400'0203){
			return flaggedInterrupts.data[1];
		}else if (address >= 0x0'0400'0208){
			return interruptsMasterEnable.data[address - 0x0'0400'0208];
		}
		assert(false);
		return 0x0;
	}
	bool InterruptControl::allowWrite(uint32_t address) const {
		return true;
	}
	void InterruptControl::write(uint32_t address, byte value) {
		if (address == 0x0'0400'0200){
			enabledInterrupts.data[0] = value;
		}else if (address == 0x0'0400'0201){
			enabledInterrupts.data[1] = value;
		}else if (address == 0x0'0400'0202){
			flaggedInterrupts.data[0] = value;
		}else if (address == 0x0'0400'0203){
			flaggedInterrupts.data[1] = value;
		}else if (address >= 0x0'0400'0208){
			interruptsMasterEnable.data[address - 0x0'0400'0208] = value;
		}else{
			assert(false);
		}
	}

	void InterruptControl::reset(){
		enabledInterrupts.data[0] = 0;
		enabledInterrupts.data[1] = 0;
		
		flaggedInterrupts.data[0] = 0;
		flaggedInterrupts.data[1] = 0;

		interruptsMasterEnable.data[0] = 0;
		interruptsMasterEnable.data[1] = 0;
		interruptsMasterEnable.data[2] = 0;
		interruptsMasterEnable.data[3] = 0;
	}
}
