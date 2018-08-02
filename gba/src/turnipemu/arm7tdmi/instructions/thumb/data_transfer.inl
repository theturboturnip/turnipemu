#include "turnipemu/arm7tdmi/instruction_category.h"
#include "turnipemu/utils.h"

namespace TurnipEmu::ARM7TDMI::Thumb {
	class PCRelativeLoadInstruction : public ThumbInstructionCategory {
		using ThumbInstructionCategory::ThumbInstructionCategory;

		struct InstructionData {
			uint16_t immediateValue : 10;
			uint8_t destinationRegister : 3;
			
			InstructionData(halfword instruction){
				immediateValue = (instruction & 0xFF) << 2;
				destinationRegister = (instruction >> 8) & 0b111;
			}
		};

		std::string disassembly(halfword instruction) const override {
			InstructionData data(instruction);

			return Utils::streamFormat(
				"Load from PC + ",
				(int)data.immediateValue,
				" into Register ",
				(int)data.destinationRegister,
				". The PC is forced to be word-aligned by zeroing bit 0."
				);
		}
	};
}
