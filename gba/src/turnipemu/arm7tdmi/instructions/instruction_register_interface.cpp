#include "turnipemu/arm7tdmi/instruction_category.h"

#include "turnipemu/arm7tdmi/cpu.h"
#include "turnipemu/arm7tdmi/pipeline.h"

namespace TurnipEmu::ARM7TDMI::Instructions {
	word InstructionRegisterInterface::getNextInstructionAddress() const {
		assert(pipeline->decodedInstructionAddress + pipeline->instructionTypeSize == *registers.main[15] + pipeline->instructionTypeSize - 8);
		return pipeline->decodedInstructionAddress + pipeline->instructionTypeSize;
	}
	
	word InstructionRegisterInterface::get(int index) {
		assert(0 <= index && index <= 15);
		if (index == 15 && reads >= 2) throw std::runtime_error("Not allowed to read the PC, could have incorrect value");
		reads++;
		return *registers.main[index];
	}
	void InstructionRegisterInterface::set(int index, word value) const {
		assert(0 <= index && index <= 15);
		if (index == 15) pipeline->queueFlushFromInstruction();
		*registers.main[index] = value;
	}
	ProgramStatusRegister& InstructionRegisterInterface::cpsr() const {
		return *registers.cpsr;
	}
	ProgramStatusRegister& InstructionRegisterInterface::spsr() const {
		return *registers.spsr;
	}
}
