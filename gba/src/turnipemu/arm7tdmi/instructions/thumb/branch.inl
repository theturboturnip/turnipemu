#include "turnipemu/arm7tdmi/instruction_category.h"

#include "../arm/conditions.inl"

namespace TurnipEmu::ARM7TDMI::Instructions::Thumb {
	class ConditionalBranchInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;

		struct InstructionData {
			int16_t offset : 9;

			InstructionData(halfword instruction){
				offset = (instruction & 0xFF) << 1;
			}
		};
	public:
		const Condition& getCondition(halfword instruction) const override {
			return ARM::InstructionCategory::conditions[(instruction >> 8) & 0xF];
		}
		std::string disassembly(halfword instruction) const override {
			InstructionData data(instruction);

			return Utils::streamFormat("Branch by ", (int)data.offset);
		}
		void execute(CPU& cpu, const RegisterPointers registers, halfword instruction) const override {
			InstructionData data(instruction);

			registers.pc() += data.offset;
		}
	};
}
