#pragma once

#include "turnipemu/arm7tdmi/registers.h"
#include "turnipemu/utils.h"

#include <functional>

namespace TurnipEmu::ARM7TDMI {
	class CPU;
	class PipelineBase;
}

namespace TurnipEmu::ARM7TDMI::Instructions {

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

	struct InstructionRegisterInterface {
		InstructionRegisterInterface(PipelineBase* pipeline, RegisterPointers rp)
			: pipeline(pipeline), registers(rp), reads(0){}
		
		const int SP = 13;
		const int LR = 14;
		const int PC = 15;

		word getNextInstructionAddress() const;

		word get(int index);
		void set(int index, word value) const;
		ProgramStatusRegister& cpsr() const;
		ProgramStatusRegister& spsr() const;
	private:
	    PipelineBase* const pipeline;
		const RegisterPointers registers;

		int reads = 0;
	};
	
	struct Condition {
		char name[3];
		std::string debugString;
		std::function<bool(ProgramStatusRegister)> fulfilsCondition;
	};

	template<typename InstructionType>
	class BaseInstructionCategory {
	public:		
		BaseInstructionCategory(std::string name, Mask<InstructionType> mask) : name(name), mask(mask){}
		virtual ~BaseInstructionCategory() = default;

		virtual const Condition& getCondition(InstructionType instruction) const = 0;
		virtual std::string disassembly(InstructionType instruction) const {
			return "NO DISASSEMBLY PRESENT";
		}
		virtual void execute(CPU& cpu, InstructionRegisterInterface, InstructionType instruction) const {
			throw std::runtime_error(Utils::streamFormat("Instruction '", name ,"' is not implemented!"));
		}
		
		const std::string name;
		const Mask<InstructionType> mask;
	};

	namespace ARM {
		class InstructionCategory : public BaseInstructionCategory<word> {
		public:
			
			InstructionCategory(std::string name, Mask<word> mask);
			~InstructionCategory() override = default;
			
			const Condition& getCondition(word instruction) const override;

			const static std::array<const Condition, 15> conditions;
		};
	}
	namespace Thumb {
		class InstructionCategory : public BaseInstructionCategory<halfword> {
		public:
			InstructionCategory(std::string name, Mask<halfword> mask);
			~InstructionCategory() override = default;

			const Condition& getCondition(halfword instruction) const override {
				return always;
			}

		protected:
			const static Condition always;
		};
	}
}
