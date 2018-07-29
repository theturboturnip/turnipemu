#pragma once

#include "turnipemu/log.h"
#include "turnipemu/types.h"
#include "turnipemu/memory/map.h"

#include "turnipemu/arm7tdmi/debug_windows.h"
#include "turnipemu/arm7tdmi/exceptions.h"
#include "turnipemu/arm7tdmi/instruction_category.h"
#include "turnipemu/arm7tdmi/modes.h"
#include "turnipemu/arm7tdmi/registers.h"

#include <string>
#include <memory>

namespace TurnipEmu::ARM7TDMI {
	class CPU {
		friend class Debug::CPUStateWindow;
		friend class Debug::CPUHistoryWindow;
		
	public:	   
		CPU(const Memory::Map& memoryMap);
		
		void tick();

		void queuePipelineFlush();
		
		void addCycles(uint32_t cycles);

		void reset();

		const Memory::Map& memoryMap;

		// TODO: These should be protected and added through a CPU::registerDebugWindows function
		Debug::CPUStateWindow debugStateWindow;
		Debug::CPUHistoryWindow debugHistoryWindow;
	protected:		
		// Determines the register pointers for the current state, taking into account the execution state and current instruction type
		const RegisterPointers registersForCurrentState();
		
		AllRegisters registers;

		uint32_t cyclesThisTick;
		uint32_t cyclesTotal;

		struct {
			bool hasFetchedInstruction;
			union {
				word fetchedInstructionWord;
				halfword fetchedInstructionHalfword;
			};
			word fetchedInstructionAddress;
			
			bool hasDecodedInstruction;
			union {
				ARMInstructionCategory* decodedArmInstruction;
				ThumbInstructionCategory* decodedThumbInstruction;
			};
			union {
				word decodedInstructionWord;
				halfword decodedInstructionHalfword;
			};
			word decodedInstructionAddress;

			bool hasExecutedInstruction;
			word executedInstructionAddress;

			bool queuedFlush;
		} pipeline;
		void flushPipeline();
		void tickPipeline();
		
		void setupInstructions();
		// These have to be vectors of unique_ptr because InstructionCategory is virtual
		std::vector<std::unique_ptr<ARMInstructionCategory>> armInstructions;
		std::vector<std::unique_ptr<ThumbInstructionCategory>> thumbInstructions;
		ARMInstructionCategory* matchArmInstruction(word instruction);
		ThumbInstructionCategory* matchThumbInstruction(halfword instruction);

		const char* const logTag = "ARM7";
	};
}
