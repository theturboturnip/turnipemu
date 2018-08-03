#pragma once

#include "turnipemu/log.h"
#include "turnipemu/types.h"
#include "turnipemu/memory/map.h"

#include "turnipemu/arm7tdmi/debug_windows.h"
#include "turnipemu/arm7tdmi/exceptions.h"
#include "turnipemu/arm7tdmi/instruction_category.h"
#include "turnipemu/arm7tdmi/modes.h"
#include "turnipemu/arm7tdmi/pipeline.h"
#include "turnipemu/arm7tdmi/registers.h"

#include <string>
#include <memory>

namespace TurnipEmu {
	class Emulator;
};

namespace TurnipEmu::ARM7TDMI {

	using namespace Instructions;
	
	struct CPUState {
		AllRegisters registers;

		uint32_t cyclesThisTick;
		uint32_t cyclesTotal;
			
		// Only one of these can be in use at any one time. If it switches, the new one must be flushed to reset the variables
		union {
			Pipeline<ARM::InstructionCategory, word> armPipeline;
			Pipeline<Thumb::InstructionCategory, halfword> thumbPipeline;
		};
		const PipelineBase* currentPipelineBaseData();

		// Determines the register pointers for the current state, taking into account the execution state and current instruction type
		const RegisterPointers usableRegisters();
	};
	
	class CPU {
		friend class Debug::CPUStateWindow;
		
	public:	   
		CPU(Emulator& emulator, const Memory::Map& memoryMap);
		
		void tick();
		
		void addCycles(uint32_t cycles);

		void reset();

		Emulator& emulator;
		const Memory::Map& memoryMap;

		// TODO: This should be protected and added through a CPU::registerDebugWindows function
		Debug::CPUStateWindow debugStateWindow;
	protected:		

		CPUState state;
		
		static void setupInstructions();
		// These have to be vectors of unique_ptr because InstructionCategory is virtual
		static bool instructionsAreSetup;
		static std::vector<std::unique_ptr<const ARM::InstructionCategory>> armInstructions;
		static std::vector<std::unique_ptr<const Thumb::InstructionCategory>> thumbInstructions;
		static const ARM::InstructionCategory* matchArmInstruction(word instruction);
		static const Thumb::InstructionCategory* matchThumbInstruction(halfword instruction);

		std::vector<word> breakpoints;
		
		const char* const logTag = "ARM7";
	};
}
