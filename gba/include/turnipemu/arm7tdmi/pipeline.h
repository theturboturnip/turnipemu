#pragma once

#include "registers.h"

#include <functional>

namespace TurnipEmu::ARM7TDMI {

	class CPU;

	class PipelineBase {
	public:
		bool hasFetchedInstruction;
		word fetchedInstructionAddress;

		bool hasDecodedInstruction;
		word decodedInstructionAddress;

		bool hasExecutedInstruction;
		word executedInstructionAddress;
	};
	
	template<typename InstructionCategoryType, typename InstructionType>
	class Pipeline : public PipelineBase {		
	public:
		InstructionType fetchedInstruction;
			
		const InstructionCategoryType* decodedInstructionCategory;
		InstructionType decodedInstruction;

		void tick(CPU& cpu, RegisterPointers currentRegisters, std::function<const InstructionCategoryType*(InstructionType)> decodeInstructionFunction);
		void flush();
	};
}
