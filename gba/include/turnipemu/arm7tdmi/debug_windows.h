#pragma once

#include <deque>

#include "turnipemu/display.h"

namespace TurnipEmu::ARM7TDMI {
	class CPU;
	struct CPUState;
}

namespace TurnipEmu::ARM7TDMI::Debug {
	class CPUStateWindow : public Display::CustomWindow {
	public:
		CPUStateWindow(CPU& cpu);

		void onCPUTick();
		void reset();
		
		void drawCustomWindowContents() override;
	protected:
		void drawCPUState(CPUState& state);
		
		CPU& cpu;
		char newBreakpointIndex[9];
		char instructionFilter[50];
		std::deque<CPUState> stateHistory;
		int selectedStateIndex = 0;
		bool teleportToSelected = false;
		bool showPartialPipelineStates = false;
		bool mergeTightLoops = true;
		int selectedRegister = -1;
		int registerTraceStartStateIndex = -1;
		constexpr static int maxStateMemory = 100'000;
		constexpr static int maxTightLoopSize = 4;
	};
}
