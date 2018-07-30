#pragma once

#include "turnipemu/arm7tdmi/instruction_category.h"

namespace TurnipEmu::ARM7TDMI {
	const ThumbInstructionCategory::Condition ThumbInstructionCategory::always = {
		"AL",
		"true",
		[](ProgramStatusRegister status) {
			return true;
		}
	};
	
	ThumbInstructionCategory::ThumbInstructionCategory(std::string name, Mask<halfword> mask)
		: InstructionCategory(name, mask)  {
		LogLine("INST", "Created ThumbInstructionCategory with name %s, mask 0x%04x, value 0x%04x", name.c_str(), mask.mask, mask.expectedValue);
	}
}
