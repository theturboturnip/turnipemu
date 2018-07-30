#pragma once

namespace TurnipEmu::ARM7TDMI {
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
