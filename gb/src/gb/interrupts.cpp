#include "gb/interrupts.h"
#include <assert.h>
#include "gb/cpu.h"

namespace GB{
	void Interrupts::trigger(Interrupt interrupt){
		for (const auto& interrupt_data : interrupts){
			if (interrupt_data.key == interrupt){
				flagged = flagged | interrupt_data.flag_value;
				break;
			}
		}

		if (cpu.halted){
			cpu.halted = false;
			return;
		}
		
		if (master_enabled)
			find_next_interrupt();
	}
	
	void Interrupts::reset(){
		master_enabled = true;
		enabled = 0x0;
		flagged = 0x0;
		cached_next_interrupt = nullptr;
	}

	void Interrupts::find_next_interrupt(){
		if (!master_enabled && !cpu.halted){
			cached_next_interrupt = nullptr;
			return;
		}

		uint8_t flagged_and_enabled = flagged & enabled;
		if (CPU::extended_debug_data)
			fprintf(stdout, "flagged = 0x%02x, enabled = 0x%02x, flagged_and_enabled = 0x%02x\n", flagged, enabled, flagged_and_enabled);
		if (flagged_and_enabled == 0){
			cached_next_interrupt = nullptr;
			return;
		}
		for (const auto& interrupt_data : interrupts){
			if (flagged_and_enabled & interrupt_data.flag_value){
				cached_next_interrupt = &interrupt_data;
				return;
			}
		}
		assert(false && "Couldn't find interrupt for flagged_and_enabled");
	}
}
