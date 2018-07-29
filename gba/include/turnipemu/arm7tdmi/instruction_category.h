#pragma once

#include "turnipemu/arm7tdmi/registers.h"

#include <functional>

namespace TurnipEmu::ARM7TDMI {
	class CPU;

	class InstructionCategory {
	public:
		struct Mask {
			struct Range {
				const uint8_t end;
				const uint8_t start;
				const word mask;
				const word value;

				Range(uint8_t end, uint8_t start, uint32_t value) : end(end), start(start), mask((word(~0) << start) & (word(~0) >> (32 - end - 1))), value((value << start) & mask)  {
					assert(start <= end);
				}
				Range(uint8_t bit, bool set) : Range(bit, bit, set){}
				
				inline void updateMask(word& theirMask) const {
					theirMask |= mask; 
				}
				inline void updateExpectedValue(word& expectedValue) const {
					expectedValue |= value;
				}
			};
			word mask;
			word expectedValue;
			Mask(std::initializer_list<Range>);
			inline bool matches(word input) const {
				return (input & mask) == expectedValue;
			}
		};
		
		struct Condition {
			char name[2];
			std::string debugString;
			std::function<bool(ProgramStatusRegister)> fulfilsCondition;
		};
			
		InstructionCategory(std::string name, Mask mask);
		virtual ~InstructionCategory(){}
			
		const Condition& getCondition(word instructionWord);
			
		virtual std::string disassembly(word instructionWord){
			return "NO DISASSEMBLY PRESENT";
		}
		virtual void execute(CPU& cpu, const RegisterPointers, word instructionWord){
			throw std::runtime_error("Instruction is not implemented!");
		}

		const std::string name;
		const Mask mask;
	protected:
		const static std::array<const Condition, 15> conditions;
	};
}
