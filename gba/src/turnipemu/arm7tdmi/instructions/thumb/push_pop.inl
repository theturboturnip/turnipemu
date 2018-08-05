#include "turnipemu/arm7tdmi/instruction_category.h"

#include <bitset>

namespace TurnipEmu::ARM7TDMI::Instructions::Thumb {
	class PushPopInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;

		enum class TransferMode : bool {
			Load = true,
			Store = false
		};
		
		struct InstructionData {
			std::bitset<8> registerList;

			TransferMode transferMode;
			union {
				bool storeLinkRegister;
				bool loadProgramCounter;
			};

			InstructionData(halfword instruction)
				: registerList(instruction & 0xFF) {
				transferMode = static_cast<TransferMode>((instruction >> 11) & 1);
				storeLinkRegister = (instruction >> 8) & 1; // Equivalent to setting loadProgramCounter
			}
		};

	public:
		// NOTES: Stack is assumed to be Full Descending
		//     Full -> The Stack Pointer points to the last item stored, NOT empty memory
		//     Descending -> The Stack Pointer starts high and continuously decrements
		// I define this instruction to push starting from 0, and to pop starting from 7
		std::string disassembly(halfword instruction) const override {
			InstructionData data(instruction);

			std::stringstream os;
			bool popping = (data.transferMode == TransferMode::Load);
			os << (popping ? "Pop (Load) " : "Push (Store) ");
			bool hasPrevious = false;
			for (int i = 0; i < 8; i++){
				if (!data.registerList[i]) continue;

				if (hasPrevious) os << ", ";
				os << "R" << i;
				hasPrevious = true;
			}
			if (popping && data.loadProgramCounter){
				if (hasPrevious) os << ", ";
				os << "PC";
				hasPrevious = true;
			}else if (!popping && data.storeLinkRegister){
				if (hasPrevious) os << ", ";
				os << "LR";
				hasPrevious = true;
			}

			assert(hasPrevious);
			os << (popping ? " from SP" : " to SP");

			return os.str();
		}
		void execute(CPU& cpu, InstructionRegisterInterface registers, halfword instruction) const override {
			InstructionData data(instruction);

			if (data.transferMode == TransferMode::Load){
				auto pop = [&](int registerIndex){
					word sp = registers.get(registers.SP);
					registers.set(registerIndex, cpu.memoryMap.read<word>(sp).value());
					registers.set(registers.SP, sp + 4);
				};
				if (data.loadProgramCounter) pop(15);
				for (int i = 7; i >= 0; i--){
					if (!data.registerList[i]) continue;

					pop(i);
				}
			}else{
				auto push = [&](int registerIndex){
					word sp = registers.get(registers.SP) - 4;
					registers.set(registers.SP, sp);
					cpu.memoryMap.write<word>(sp, registers.get(registerIndex));
				};
				for (int i = 0; i < 8; i++){
					if (!data.registerList[i]) continue;

					push(i);
				}
				if (data.storeLinkRegister) push(13);
			}
		}
	};
}
