#pragma once

#include "turnipemu/display.h"
#include "turnipemu/log.h"
#include "turnipemu/types.h"
#include "turnipemu/memory/map.h"

#include "turnipemu/arm7tdmi/debug_windows.h"
#include "turnipemu/arm7tdmi/exceptions.h"
#include "turnipemu/arm7tdmi/instruction.h"
#include "turnipemu/arm7tdmi/modes.h"
#include "turnipemu/arm7tdmi/registers.h"

#include <string>
#include <memory>

namespace TurnipEmu::ARM7TDMI {
	class CPU : public Display::CustomWindow {
	public:	   
		CPU(const Memory::Map& memoryMap);
		
		void tick();

		void queuePipelineFlush();
		
		void addCycles(uint32_t cycles);

		void reset();

		void drawCustomWindowContents() override;

		const Memory::Map& memoryMap;
	protected:		
		// Determines the register pointers for the current state, taking into account the execution state and current instruction type
		const RegisterPointers registersForCurrentState();
		
		AllRegisters registers;

		uint32_t cyclesThisTick;
		uint32_t cyclesTotal;

		struct {
			bool hasFetchedInstruction;
			word fetchedInstructionWord;
			word fetchedInstructionAddress;
			
			bool hasDecodedInstruction;
			Instruction* decodedInstruction;
			word decodedInstructionWord;
			word decodedInstructionAddress;

			bool hasExecutedInstruction;
			word executedInstructionAddress;

			bool queuedFlush;
		} pipeline;
		void flushPipeline();
		void tickPipeline();
		
		// This has to be vector of unique_ptr because Instruction is virtual
		std::vector<std::unique_ptr<Instruction>> instructions;
		void setupInstructions();
		Instruction* matchInstruction(word instructionWord);

		const char* const logTag = "ARM7";
	};
}
