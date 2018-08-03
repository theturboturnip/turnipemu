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
		std::string disassembly(halfword instruction) const override {
			InstructionData data(instruction);

			return Utils::streamFormat("Offset the Stack Pointer by ", data.offset);
		}
		void execute(CPU& cpu, RegisterPointers registers, halfword instruction) const override {
			InstructionData data(instruction);

			registers.sp() += data.offset;
		}
	};
}
