#include "turnipemu/arm7tdmi/instruction_category.h"

#include "../arm/conditions.inl"

namespace TurnipEmu::ARM7TDMI::Instructions::Thumb {
	class ConditionalBranchInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;

		struct InstructionData {
			int16_t offset : 9;

			InstructionData(halfword instruction){
				offset = (instruction & 0xFF) << 1;
			}
		};
	public:
		const Condition& getCondition(halfword instruction) const override {
			return ARM::InstructionCategory::conditions[(instruction >> 8) & 0xF];
		}
		std::string disassembly(halfword instruction) const override {
			InstructionData data(instruction);

			return Utils::streamFormat("Branch by ", (int)data.offset);
		}
		void execute(CPU& cpu, const RegisterPointers registers, halfword instruction) const override {
			InstructionData data(instruction);

			registers.pc() += data.offset;
		}
	};

	class BranchExchangeInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;

		struct InstructionData {
			bool link;
			uint8_t baseRegister : 4;

			InstructionData(word instruction){
				link = false;
				baseRegister = (instruction >> 3) & 0xF;
			}
		};

	public:
		std::string disassembly(halfword instruction) const override {
			InstructionData data(instruction);

			return Utils::streamFormat("Branch to [Register ", (int)data.baseRegister, "], if bottom bit is 1 then continue in Thumb else continue in ARM");
		}
		void execute(CPU& cpu, const RegisterPointers registers, halfword instruction) const override {
			InstructionData data(instruction);

			word newAddress = *registers.main[data.baseRegister];
			CPUExecState newState = (newAddress & 1) ? CPUExecState::Thumb : CPUExecState::ARM;
			newAddress = newAddress - (newAddress % 2);
			registers.pc() = newAddress;
			registers.cpsr->state = newState;
		}
	};
}
