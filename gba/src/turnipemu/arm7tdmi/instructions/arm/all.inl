#pragma once

#include "conditions.inl"
#include "alu.inl"
#include "branch.inl"
#include "data_transfer.inl"
#include "msr_mrs.inl"

namespace TurnipEmu::ARM7TDMI::Instructions::ARM {
	InstructionCategory::InstructionCategory(std::string name, Mask<word> mask)
		: BaseInstructionCategory(name, mask)  {
		LogLine("INST", "Created ARMInstructionCategory with name %s, mask 0x%08x, value 0x%08x", name.c_str(), mask.mask, mask.expectedValue);
	}

	const Condition& InstructionCategory::getCondition(word instructionWord) const {
		return conditions[(instructionWord >> 28) & 0xF];
	}
}
