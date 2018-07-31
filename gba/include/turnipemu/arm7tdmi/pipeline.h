#pragma once

#include "registers.h"

#include <functional>

namespace TurnipEmu::ARM7TDMI {

	class CPU;
	
	template<typename InstructionCategoryType, typename InstructionType>
	class Pipeline {		
	public:
		bool hasFetchedInstruction;
		InstructionType fetchedInstruction;
		word fetchedInstructionAddress;
			
		bool hasDecodedInstruction;
		const InstructionCategoryType* decodedInstructionCategory;
		InstructionType decodedInstruction;
		word decodedInstructionAddress;

		bool hasExecutedInstruction;
		word executedInstructionAddress;

		void tick(CPU& cpu, RegisterPointers currentRegisters, std::function<const InstructionCategoryType*(InstructionType)> decodeInstructionFunction);
		void flush();
	};
}
