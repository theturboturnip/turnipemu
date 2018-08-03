#include "turnipemu/arm7tdmi/instruction_category.h"

#include "../arm/conditions.inl"

namespace TurnipEmu::ARM7TDMI::Instructions::Thumb {
	class ConditionalBranchInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;

	public:
		const Condition& getCondition(halfword instruction) const override {
			return ARM::InstructionCategory::conditions[(instruction >> 8) & 0xF];
		}
	};
}
