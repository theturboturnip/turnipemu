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

	class InstructionCategory {
	public:
		InstructionCategory(std::string name) : name(name){}
		virtual ~InstructionCategory() = default;
		
		const std::string name;
	};
	
	class ARMInstructionCategory : public InstructionCategory {
	public:
		struct Condition {
			char name[2];
			std::string debugString;
			std::function<bool(ProgramStatusRegister)> fulfilsCondition;
		};
			
		ARMInstructionCategory(std::string name, Mask<word> mask);
		~ARMInstructionCategory() override = default;
			
		const Condition& getCondition(word instruction);

		virtual std::string disassembly(word instruction){
			return "NO DISASSEMBLY PRESENT";
		}
		virtual void execute(CPU& cpu, const RegisterPointers, word instruction){
			throw std::runtime_error("Instruction is not implemented!");
		}
		
		const Mask<word> mask;
	protected:
		const static std::array<const Condition, 15> conditions;
	};

	class ThumbInstructionCategory : public InstructionCategory {
	public:
		ThumbInstructionCategory(std::string name, Mask<halfword> mask);
		~ThumbInstructionCategory() override = default;

		virtual std::string disassembly(halfword instruction){
			return "NO DISASSEMBLY PRESENT";
		}
		virtual void execute(CPU& cpu, const RegisterPointers, halfword instruction){
			throw std::runtime_error("Instruction is not implemented!");
		}
		
		const Mask<halfword> mask;
	};
}
