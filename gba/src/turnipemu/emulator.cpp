#include "turnipemu/emulator.h"

#include "turnipemu/imgui.h"

namespace TurnipEmu {
	void Emulator::drawCustomWindowContents(){
		if (stopped){
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,0,0,1));
			ImGui::Text("Stopped!");
			ImGui::PopStyleColor();
		}else{
			ImGui::Checkbox("Paused", &paused);
			if (paused){
				bool wantsTick = ImGui::Button("Tick");
				if (wantsTick){
					tick();
				}
			}
		}

		if (ImGui::Button("Reset")){
			reset();
		}
	}
}
