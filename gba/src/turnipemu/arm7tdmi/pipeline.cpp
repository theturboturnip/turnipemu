#include "turnipemu/arm7tdmi/pipeline.h"

#include "turnipemu/arm7tdmi/cpu.h"
#include "turnipemu/arm7tdmi/instruction_category.h"
#include "turnipemu/log.h"

namespace TurnipEmu::ARM7TDMI {
	
	template<typename InstructionCategoryType, typename InstructionType>
	void Pipeline<InstructionCategoryType, InstructionType>::flush(){
		hasFetchedInstruction = false;
		fetchedInstruction = 0x0;
		fetchedInstructionAddress = 0x0;
		
		hasDecodedInstruction = false;
		decodedInstructionCategory = nullptr;
		decodedInstruction = 0x0;
		decodedInstructionAddress = 0x0;

		hasExecutedInstruction = false;
		executedInstructionAddress = 0x0;
	}

	template<typename InstructionCategoryType, typename InstructionType>
	void Pipeline<InstructionCategoryType, InstructionType>::tick(CPU& cpu, const RegisterPointers registers, std::function<const InstructionCategoryType*(InstructionType)> decodeInstructionFunction){

		const word oldPC = registers.pc();
		
		if (hasFetchedInstruction){
			if (hasDecodedInstruction){
				// Execute previously decoded instruction
				const auto& condition = decodedInstructionCategory->getCondition(decodedInstruction);
				if (condition.fulfilsCondition(*registers.cpsr)){
                    // This can flush the pipeline.
					decodedInstructionCategory->execute(cpu, registers, decodedInstruction);
				}

				hasExecutedInstruction = true;
				executedInstructionAddress = decodedInstructionAddress;
			}

			// Decode fetched instruction
			decodedInstructionAddress = fetchedInstructionAddress;
			decodedInstruction = fetchedInstruction;
			decodedInstructionCategory = decodeInstructionFunction(decodedInstruction);
			if (!decodedInstructionCategory) throw std::runtime_error("Couldn't decode instruction!");
			hasDecodedInstruction = true;
		}else{
			assert(!hasDecodedInstruction);
		}

		if (oldPC != registers.pc()){
			flush();
		}else{
			// Fetch the instruction
			if (auto instructionOptional = cpu.memoryMap.read<InstructionType>(registers.pc())){
				fetchedInstructionAddress = registers.pc();
				fetchedInstruction = instructionOptional.value();
				hasFetchedInstruction = true;
			}else{
				throw std::runtime_error("PC is in invalid memory!");
			}
			
			registers.pc() += sizeof(InstructionType);
		}
	}

	template class Pipeline<ARMInstructionCategory, word>;
	template class Pipeline<ThumbInstructionCategory, halfword>;
}
