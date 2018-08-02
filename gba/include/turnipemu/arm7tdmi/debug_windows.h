#pragma once

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
		std::vector<CPUState> stateHistory;
		int selectedStateIndex = 0;
		bool teleportToSelected = false;
		constexpr static int maxStateMemory = 100;
	};
}
