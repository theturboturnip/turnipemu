#pragma once

namespace TurnipEmu::ARM7TDMI::Instructions::ARM {
	class BranchInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;
		
		struct InstructionData {
			int64_t offset;
			bool link;

			InstructionData(word instruction){
				offset = (int64_t)(int32_t)((instruction >> 0) & 0xFFFFFF) << 2;
				link = (instruction >> 24) & 1;
			}
		};
		std::string disassembly(word instruction) const override {
			InstructionData data(instruction);
			std::stringstream stream;
			stream << "Branch by " << data.offset << ", link: " << std::boolalpha << data.link;
			return stream.str();
		}
		void execute(CPU& cpu, InstructionRegisterInterface registers, word instruction) const override {
			InstructionData data(instruction);
			if (data.link){
				registers.set(registers.LR, registers.getNextInstructionAddress());
			}
			word pcWithPrefetch = registers.get(registers.PC);
			registers.set(registers.PC, pcWithPrefetch + data.offset);
		}
	};

	class BranchExchangeInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;

		struct InstructionData {
			bool link;
			uint8_t baseRegister : 4;

			InstructionData(word instruction){
				link = false;
				baseRegister = instruction & 0xF;
			}
		};
		std::string disassembly(word instruction) const override {
			InstructionData data(instruction);
			std::stringstream stream;
			stream << "Branch to location [Register " << (int)data.baseRegister << "], if bottom bit is 1 continue in Thumb, else continue in ARM. links: " << std::boolalpha << data.link;
			return stream.str();
		}
		void execute(CPU& cpu, InstructionRegisterInterface registers, word instruction) const override {
			InstructionData data(instruction);
			if (data.link){
				throw std::runtime_error("Tried to do a Branch and eXchange with link enabled. Is this valid?");
				//word pcForNextInstruction = *registers.main[15] + 4 - 8; // PC has been prefetched, the -8 undoes that
				registers.set(registers.LR, registers.getNextInstructionAddress());
			}
			word newAddress = registers.get(data.baseRegister);
			CPUExecState newState = (newAddress & 1) ? CPUExecState::Thumb : CPUExecState::ARM;
			newAddress = newAddress - (newAddress % 2);
			registers.set(registers.PC, newAddress);
			registers.cpsr().state = newState;
		}
	};
}
