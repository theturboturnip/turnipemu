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
		void execute(CPU& cpu, InstructionRegisterInterface registers, halfword instruction) const override {
			InstructionData data(instruction);

			registers.set(registers.PC, registers.get(registers.PC) + data.offset);
		}
	};

	class LongBranchLinkInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;

		struct InstructionData {
			uint8_t instructionPart : 1; // This instruction is in two 'parts' that have to be executed consecutively
			uint32_t offset;
			
			InstructionData(halfword instruction){
				instructionPart = (instruction >> 11) & 1;
				uint16_t baseOffset = instruction & (0xFFF >> 1); // to get 11 bits take 12 bits (0xFFF) and shift right 1

				if (instructionPart == 0){
					offset = baseOffset << 12;
					if ((offset >> 22) & 1){
						// Sign bit set, extend to the left
						offset |= word(~0) << 23;
					}
				}else{
					offset = baseOffset << 1;
				}
			}
		};
	public:
		std::string disassembly(halfword instruction) const override {
			InstructionData data(instruction);

			return Utils::streamFormat("Long Branch with Link part #", data.instructionPart + 1, ": offset to apply = ", Utils::HexFormat(data.offset));
		}
		void execute(CPU& cpu, InstructionRegisterInterface registers, halfword instruction) const override {
			InstructionData data(instruction);

			if (data.instructionPart == 0){
				registers.set(registers.LR, registers.get(registers.PC) + data.offset);
			}else{
				word jumpingTo = registers.get(registers.LR) + data.offset;
				registers.set(registers.LR, registers.getNextInstructionAddress() | 1); // Set bit 0 to 1 so that any jumps to the link register will go into thumb mode
				registers.set(registers.PC, jumpingTo);
			}
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
		void execute(CPU& cpu, InstructionRegisterInterface registers, halfword instruction) const override {
			InstructionData data(instruction);

			word newAddress = registers.get(data.baseRegister);
			CPUExecState newState = (newAddress & 1) ? CPUExecState::Thumb : CPUExecState::ARM;
			newAddress = newAddress - (newAddress % 2);
			registers.set(registers.PC, newAddress);
			registers.cpsr().state = newState;
		}
	};
}
