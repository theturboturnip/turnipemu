#pragma once

#include "registers.h"

#include <functional>

namespace TurnipEmu::ARM7TDMI {

	class CPU;
	namespace Instructions {
		class BaseInstructionCategory;
	};
		
	class PipelineBase {
	public:
		bool hasFetchedInstruction;
		word fetchedInstructionAddress;

		bool hasDecodedInstruction;
		word decodedInstructionAddress;
		const Instructions::BaseInstructionCategory* decodedInstructionCategory;

		bool hasExecutedInstruction;
		word executedInstructionAddress;

		size_t instructionTypeSize;
		
		inline void queueFlushFromInstruction(){
			flushQueuedByInstruction = true;
		}
	protected:
		bool flushQueuedByInstruction;
	};
	
	template<typename InstructionCategoryType, typename InstructionType>
	class Pipeline : public PipelineBase {
	public:
		InstructionType fetchedInstruction;
			
		InstructionType decodedInstruction;

		void tick(CPU& cpu, RegisterPointers currentRegisters, std::function<const InstructionCategoryType*(InstructionType)> decodeInstructionFunction);
		void flush();
	};
}
