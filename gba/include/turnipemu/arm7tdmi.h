#pragma once

#include "types.h"
#include "memory.h"

#include <string>
#include <memory>

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

		struct InstructionMask {
			struct MaskRange {
				uint8_t end;
				uint8_t start;
				uint32_t value;
				
			MaskRange(uint8_t end, uint8_t start, uint32_t value) : end(end), start(start), value(value) {
				assert(start <= end);
			}
			MaskRange(uint8_t bit, bool set) : end(bit), start(bit), value(set << bit){}
				
				inline void updateMask(word& mask) const {
					mask |= ((~0) << start) & ((0) << (end + 1)); 
				}
				inline void updateExpectedValue(word& expectedValue) const {
					word maskedValue = value & ((~0) << start) & ((0) << (end + 1));
					// TODO: This should already be done, this is kinda redundant
					expectedValue |= maskedValue;
				}
			};
			word mask;
			word expectedValue;
			InstructionMask(std::initializer_list<MaskRange>);
			inline bool matches(word input) const {
				return (input & mask) == expectedValue;
			}
		};
		class Instruction {
		public:
			Instruction(std::string category, InstructionMask mask);
			
			//const InstructionMask mask;
			//virtual void execute(ARM7TDMI& cpu, RegisterPointers);
		};
		friend class Instruction;
		// This has to be vector of unique_ptr because Instruction is virtual
		std::vector<std::unique_ptr<Instruction>> instructions;
		void setupInstructions();
	};
}
