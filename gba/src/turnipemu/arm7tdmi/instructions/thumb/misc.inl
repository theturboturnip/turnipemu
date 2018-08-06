#include "turnipemu/arm7tdmi/instruction_category.h"

namespace TurnipEmu::ARM7TDMI::Instructions::Thumb {
	class OffsetStackPointerInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;

		struct InstructionData {
			int offset;

			InstructionData(halfword instruction){
				offset = (instruction & 0b1111111) << 2;
				if ((instruction >> 7) & 1) offset = -offset;	
			}
		};
		
	public:
		std::string disassembly(word instruction) const override {
			InstructionData data(instruction);

			return Utils::streamFormat("Offset the Stack Pointer by ", data.offset);
		}
		void execute(CPU& cpu, InstructionRegisterInterface registers, word instruction) const override {
			InstructionData data(instruction);

			registers.set(registers.SP, registers.get(registers.SP) + data.offset);
		}
	};
}
