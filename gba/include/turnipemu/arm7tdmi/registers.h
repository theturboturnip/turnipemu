#pragma once

#include "modes.h"
#include "turnipemu/types.h"

namespace TurnipEmu::ARM7TDMI {
	enum class CPUExecState : bool {
		ARM = false,
		Thumb = true
	};
	
	union ProgramStatusRegister{
		word value;
#pragma pack(1) // No Padding
		struct {
			Mode mode : 5; // Bits 0-4
			CPUExecState state : 1; // Bit 5, determines if in Thumb or Arm mode
			bool fiqDisable : 1; // Bit 6
			bool irqDisable : 1; // Bit 7
			uint32_t reserved : 20; // Bits 8 through 27 inclusive
			bool overflow : 1;
			bool carry : 1;
			bool zero : 1;
			bool negative : 1;
		};
#pragma pack() // Reset packing mode
	};
	static_assert(sizeof(ProgramStatusRegister) == 4, "ProgramStatusRegister must be a single word");
		
    // This is used by the instructions 
	struct RegisterPointers{
		word* main[16] = {nullptr};
		bool* changedRegisters[16] = {nullptr};
			
		ProgramStatusRegister* cpsr = nullptr;
		ProgramStatusRegister* spsr = nullptr; // Not enabled in System/User mode

		inline word& pc() const {
			return *main[15];
		}
		inline word& lr() const {
			return *main[14];
		}
		inline word& sp() const {
			return *main[13];
		}
	};

	struct AllRegisters{	
		word main[16];

		inline word& pc(){
			return main[15];
		}
		
		ProgramStatusRegister cpsr;
			
		struct{
			word r8;
			word r9;
			word r10;
			word r11;
			word r12;
			word r13;
			word r14;

			ProgramStatusRegister spsr;
		} fiq;
		struct{
			word r13;
			word r14;

			ProgramStatusRegister spsr;
		} svc;
		struct{
			word r13;
			word r14;

			ProgramStatusRegister spsr;
		} abt;
		struct{
			word r13;
			word r14;

			ProgramStatusRegister spsr;
		} irq;
		struct{
			word r13;
			word r14;

			ProgramStatusRegister spsr;
		} und;
	};
}
