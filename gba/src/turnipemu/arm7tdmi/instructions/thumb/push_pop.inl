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
				if (data.registerList[i]){
					if (hasPrevious) os << ", ";
					os << "R" << i;
					hasPrevious = true;
				}
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
		//void execute(CPU& cpu, RegisterPointers registers, halfword instruction) const override {}
	};
}
