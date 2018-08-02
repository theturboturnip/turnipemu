#include "turnipemu/arm7tdmi/instruction_category.h"
#include "turnipemu/utils.h"
#include "turnipemu/log.h"

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

	public:
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
		void execute(CPU& cpu, RegisterPointers registers, halfword instruction) const override {
			InstructionData data(instruction);

			word address = registers.pc();
			address = address & (word(~0) << 2); // Set bits 0 and 1 to 0, to make sure it's a multiple of 4
			address += data.immediateValue;

			if (auto dataOptional = cpu.memoryMap.read<word>(address))
				*registers.main[data.destinationRegister] = dataOptional.value();
		}
	};
}
