#pragma once

#include "turnipemu/arm7tdmi/instruction_category.h"
#include "turnipemu/arm7tdmi/misc.h"

namespace TurnipEmu::ARM7TDMI::Instructions::Thumb {
	class PushPopInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;
		
		struct InstructionData : public MultipleLoadStore::InstructionData {
			InstructionData(halfword instruction) {
				registerList = (instruction & 0xFF);

				// NOTES: Stack is assumed to be Full Descending
				//     Full -> The Stack Pointer points to the last item stored, NOT empty memory
				//     Descending -> The Stack Pointer starts high and continuously decrements
				if ((instruction >> 11) & 1){
					genericInfo.transferMode = DataTransferInfo::TransferMode::Load;
					genericInfo.indexMode = DataTransferInfo::IndexMode::PostIndex;
					genericInfo.offsetSign = 1;
					if ((instruction >> 8) & 1) registerList.set(15); // Load PC
				}else{
					genericInfo.transferMode = DataTransferInfo::TransferMode::Store;
					genericInfo.indexMode = DataTransferInfo::IndexMode::PreIndex;
					genericInfo.offsetSign = -1;
					if ((instruction >> 8) & 1) registerList.set(14); // Store LR
				}
				genericInfo.writeback = true;
				genericInfo.addressRegister = 13; // TODO: Static definition of "Stack Pointer" somewhere
			}
		};

	public:
		std::string disassembly(word instruction) const override {
			InstructionData data(instruction);

			std::stringstream os;
			os << data;
			return os.str();
		}
		void execute(CPU& cpu, InstructionRegisterInterface registers, word instruction) const override {
			InstructionData data(instruction);

			MultipleLoadStore::Execute(data, cpu, registers);
		}
	};
}
