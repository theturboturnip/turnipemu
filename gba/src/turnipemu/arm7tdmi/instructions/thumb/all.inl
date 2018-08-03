#pragma once

#include "alu.inl"
#include "branch.inl"
#include "data_transfer.inl"
#include "push_pop.inl"
#include "misc.inl"

namespace TurnipEmu::ARM7TDMI::Instructions::Thumb {
	const Condition InstructionCategory::always = {
		"AL",
		"true",
		[](ProgramStatusRegister status) {
			return true;
		}
	};
	
	InstructionCategory::InstructionCategory(std::string name, Mask<halfword> mask)
		: BaseInstructionCategory(name, mask)  {
		LogLine("INST", "Created ThumbInstructionCategory with name %s, mask 0x%04x, value 0x%04x", name.c_str(), mask.mask, mask.expectedValue);
	}
}
