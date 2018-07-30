#pragma once

#include "instructions_common.inl"
#include "instructions_alu.inl"
#include "instructions_data_transfer.inl"
#include "instructions_msr_mrs.inl"

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

	class BranchInstruction : public ARMInstructionCategory {
		using ARMInstructionCategory::ARMInstructionCategory;
		
		struct InstructionData {
			int64_t offset;
			bool link;

			InstructionData(word instructionWord){
				offset = (int64_t)(int32_t)((instructionWord >> 0) & 0xFFFFFF) << 2;
				link = (instructionWord >> 24) & 1;
			}
		};
		std::string disassembly(word instructionWord) const override {
			InstructionData data(instructionWord);
			std::stringstream stream;
			stream << "Branch by " << data.offset << ", link: " << std::boolalpha << data.link;
			return stream.str();
		}
		void execute(CPU& cpu, const RegisterPointers registers, word instructionWord) const override {
			InstructionData data(instructionWord);
			if (data.link){
				word pcForNextInstruction = *registers.main[15] + 4 - 8; // PC has been prefetched, the -8 undoes that
				*registers.main[14] = pcForNextInstruction;
			}
			word pcWithPrefetch = *registers.main[15];
			*registers.main[15] = pcWithPrefetch + data.offset;
		}
	};
}
