#pragma once

#include "turnipemu/arm7tdmi/registers.h"
#include "turnipemu/utils.h"

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
				: end(end),
				  start(start),
				  mask(
					  ( InstructionType(~0) << start ) &
					  ( InstructionType(~0) >> ( (sizeof(InstructionType) * 8) - end - 1 ) )
					  ),
				  value((value << start) & mask)  {
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

	template<typename InstructionType>
	class InstructionCategory {
	public:
		struct Condition {
			char name[3];
			std::string debugString;
			std::function<bool(ProgramStatusRegister)> fulfilsCondition;
		};
		
		InstructionCategory(std::string name, Mask<InstructionType> mask) : name(name), mask(mask){}
		virtual ~InstructionCategory() = default;

		virtual const Condition& getCondition(InstructionType instruction) const = 0;
		virtual std::string disassembly(InstructionType instruction) const {
			return "NO DISASSEMBLY PRESENT";
		}
		virtual void execute(CPU& cpu, const RegisterPointers, InstructionType instruction) const {
			throw std::runtime_error(Utils::streamFormat("Instruction '", name ,"' is not implemented!"));
		}
		
		const std::string name;
		const Mask<InstructionType> mask;
	};
	
	class ARMInstructionCategory : public InstructionCategory<word> {
	public:
			
		ARMInstructionCategory(std::string name, Mask<word> mask);
		~ARMInstructionCategory() override = default;
			
		const Condition& getCondition(word instruction) const override;

	protected:
		const static std::array<const Condition, 15> conditions;
	};

	class ThumbInstructionCategory : public InstructionCategory<halfword> {
	public:
		ThumbInstructionCategory(std::string name, Mask<halfword> mask);
		~ThumbInstructionCategory() override = default;

		const Condition& getCondition(halfword instruction) const override {
			return always;
		}

	protected:
		const static Condition always;
	};
}
