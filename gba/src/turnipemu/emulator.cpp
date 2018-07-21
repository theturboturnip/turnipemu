#include "emulator.h"

#include "imgui/imgui.h"

namespace TurnipEmu {
	void Emulator::drawCustomWindowContents(){
		ImGui::Checkbox("Paused", &paused);
		if (paused){
			bool wantsTick = ImGui::Button("Tick");
			if (wantsTick){
				tick();
			}
		}
	}
}
