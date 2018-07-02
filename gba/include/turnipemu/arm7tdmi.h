#pragma once

#include "types.h"
#include "memory.h"

namespace TurnipEmu {
	class ARM7TDMI{
	public:
		union ProgramStatusRegister{
			word value;
#pragma pack(1) // No Padding
			struct {
				uint8_t modeBits : 5; // Bits 0-4
				uint8_t stateBit : 1; // Bit 5, determines if in Thumb or Arm mode
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
		
		// TODO: Does this need to be public?
		// This is used by the instructions 
		struct RegisterPointers{
			const word* main[16] = {nullptr};
			
			const word* cpsr = nullptr;
			const word* spsr = nullptr; // Not enabled in System/User mode
		};

		ARM7TDMI(const MemoryMap& memoryMap);
		
		void executeNextInstruction();
		void executeInstruction(word instruction);

		void addCycles(uint32_t cycles);

		void reset();
	protected:
		const MemoryMap& memoryMap;
		
		// Determines the register pointers for the current state, taking into account the execution state and current instruction type
		RegisterPointers registersForCurrentState() const;
		
		struct{
			word regs[16];

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
		} registers;

		// TODO: Store in the CPSR register instead of separately
		enum class InstructionType{
			ARM,
			Thumb
		};
		InstructionType instructionType;
		enum class Mode{
			System,
			User,
			FIQ,
			Supervisor,
			Abort,
			IRQ,
			Undefined
		};
		Mode mode;

		uint32_t cyclesThisTick;
		uint32_t cyclesTotal;
	};
}
