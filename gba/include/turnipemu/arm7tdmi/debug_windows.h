#pragma once

#include "turnipemu/display.h"

namespace TurnipEmu::ARM7TDMI {
	class CPU;
}

namespace TurnipEmu::ARM7TDMI::Debug {
	class CPUStateWindow : public Display::CustomWindow {
	public:
		CPUStateWindow(CPU& cpu);
		
		void drawCustomWindowContents() override;
	protected:
		CPU& cpu;
		char newBreakpointIndex[9];
	};
	class CPUHistoryWindow : public Display::CustomWindow {
	public:
		CPUHistoryWindow(CPU& cpu);

		void addCurrentStateToHistory();
		
		void drawCustomWindowContents() override;
	protected:
		CPU& cpu;
	};
}
