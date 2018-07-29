#pragma once

#include "turnipemu/arm7tdmi/registers.h"

#include <functional>

namespace TurnipEmu::ARM7TDMI {
	class CPU;

	template<typename InstructionType>
	struct Mask {
		static_assert(std::is_same<InstructionType, halfword>::value || std::is_same<InstructionType, word>::value);
		
		struct Range {
			const uint8_t end;
			const uint8_t start;
			const InstructionType mask;
			const InstructionType value;

			Range(uint8_t end, uint8_t start, uint32_t value)
				: end(end), start(start), mask((InstructionType(~0) << start) & (InstructionType(~0) >> (32 - end - 1))), value((value << start) & mask)  {
				assert(start <= end);
			}
			Range(uint8_t bit, bool set) : Range(bit, bit, set){}
				
			inline void updateMask(InstructionType& theirMask) const {
				theirMask |= mask; 
			}
			inline void updateExpectedValue(InstructionType& expectedValue) const {
				expectedValue |= value;
			}
		};
		InstructionType mask;
		InstructionType expectedValue;

		Mask(std::initializer_list<Range> list){
			mask = 0;
			expectedValue = 0;
			for (const auto& range : list){
				range.updateMask(mask);
				range.updateExpectedValue(expectedValue);
			}
		}
		
		inline bool matches(InstructionType input) const {
			return (input & mask) == expectedValue;
		}
	};
	
	class ARMInstructionCategory {
	public:
		struct Condition {
			char name[2];
			std::string debugString;
			std::function<bool(ProgramStatusRegister)> fulfilsCondition;
		};
			
		ARMInstructionCategory(std::string name, Mask<word> mask);
		virtual ~ARMInstructionCategory(){}
			
		const Condition& getCondition(word instructionWord);
			
		virtual std::string disassembly(word instructionWord){
			return "NO DISASSEMBLY PRESENT";
		}
		virtual void execute(CPU& cpu, const RegisterPointers, word instructionWord){
			throw std::runtime_error("Instruction is not implemented!");
		}

		const std::string name;
		const Mask<word> mask;
	protected:
		const static std::array<const Condition, 15> conditions;
	};
}
