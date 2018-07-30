#pragma once

#include "alu.inl"
#include "branch.inl"
#include "data_transfer.inl"
#include "msr_mrs.inl"

namespace TurnipEmu::ARM7TDMI {
#define CONDITION(NAME, CODE) ARMInstructionCategory::Condition{ NAME, {#CODE}, [](ProgramStatusRegister status) { return CODE; } }
	const std::array<const ARMInstructionCategory::Condition, 15> ARMInstructionCategory::conditions = {
		CONDITION("EQ", status.zero),
		CONDITION("NE", !status.zero),
		CONDITION("CS", status.carry),
		CONDITION("CC", !status.carry),
		CONDITION("MI", status.negative),
		CONDITION("PL", !status.negative),
		CONDITION("VS", status.overflow),
		CONDITION("VC", !status.overflow),
		CONDITION("HI", status.carry && !status.zero),
		CONDITION("LS", !status.carry || status.zero),
		CONDITION("GE", status.negative == status.overflow),
		CONDITION("LT", status.negative != status.overflow),
		CONDITION("GT", !status.zero && (status.negative == status.overflow)),
		CONDITION("LE", status.zero || (status.negative != status.overflow)),
		CONDITION("AL", true),
	};
#undef CONDITION

	ARMInstructionCategory::ARMInstructionCategory(std::string name, Mask<word> mask)
		: InstructionCategory(name, mask)  {
		LogLine("INST", "Created ARMInstructionCategory with name %s, mask 0x%08x, value 0x%08x", name.c_str(), mask.mask, mask.expectedValue);
	}

	const ARMInstructionCategory::Condition& ARMInstructionCategory::getCondition(word instructionWord) const {
		return conditions[(instructionWord >> 28) & 0xF];
	}
}
