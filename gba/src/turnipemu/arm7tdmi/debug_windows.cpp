#include "turnipemu/arm7tdmi/debug_windows.h"

#include "turnipemu/arm7tdmi/cpu.h"

namespace TurnipEmu::ARM7TDMI::Debug {
	CPUStateWindow::CPUStateWindow(CPU& cpu)
		: Display::CustomWindow("ARM7TDMI Instruction Inspector", 600, 500), cpu(cpu) {
		stateHistory = std::vector<CPUState>();
		stateHistory.reserve(maxStateMemory + 1);
	}

	void CPUStateWindow::onCPUTick(){
		stateHistory.push_back(cpu.state);
		if (stateHistory.size() > maxStateMemory) stateHistory.erase(stateHistory.begin());
		selectedStateIndex = stateHistory.size() - 1;
		teleportToSelected = true;
	}
	void CPUStateWindow::reset(){
		stateHistory.clear();
		stateHistory.push_back(cpu.state);
		selectedStateIndex = 0;
	}

	template<class PipelineType>
	void displayPipeline(RegisterPointers currentRegisters, PipelineType pipeline){
		ImGui::Text("Pipeline Status");
		{
			ImGui::Indent();
			if (pipeline.hasFetchedInstruction){
				ImGui::Text("Fetched Address: 0x%08x", pipeline.fetchedInstructionAddress);
				if (pipeline.hasDecodedInstruction){
					ImGui::Text("Decoded Address: 0x%08x", pipeline.decodedInstructionAddress);
					if (pipeline.hasExecutedInstruction){
						ImGui::Text("Executed Address: 0x%08x", pipeline.executedInstructionAddress);
					}
				}
			}else{
				ImGui::Text("Pipeline Flushed");
			}
			ImGui::Unindent();
		}

		if (pipeline.hasDecodedInstruction){
			ImGui::Separator();
			ImGui::Text("Just Decoded Instruction");
			ImGui::Indent();

			const auto* instructionCategory = pipeline.decodedInstructionCategory;
			if (instructionCategory){
				const auto& condition = instructionCategory->getCondition(pipeline.decodedInstruction);
				ImGui::Text(
					sizeof(PipelineType::decodedInstruction) == 4 ? "0x%08x: 0x%08x %s" : "0x%08x: 0x%04x %s",
					pipeline.decodedInstructionAddress, pipeline.decodedInstruction, condition.name);
				{
					ImGui::Indent();
					ImGui::TextWrapped("Condition Code: %s [Fulfilled: %d]",
								condition.debugString.c_str(),
								condition.fulfilsCondition(*currentRegisters.cpsr));
					ImGui::TextWrapped("Instruction Category: %s", instructionCategory->name.c_str());
					ImGui::TextWrapped("Instruction Disassembly: %s", instructionCategory->disassembly(pipeline.decodedInstruction).c_str());
					ImGui::Unindent();
				}
			}else{
				ImGui::Text("0x%08x: 0x%08x (Not an instruction)", pipeline.decodedInstructionAddress, pipeline.decodedInstruction);
			}

			ImGui::Unindent();
		}
	}

	void CPUStateWindow::drawCPUState(CPUState& state){
		const RegisterPointers registers = state.usableRegisters();
		
		if (registers.cpsr->state == CPUExecState::Thumb){
			displayPipeline(registers, state.thumbPipeline);
		}else{
			displayPipeline(registers, state.armPipeline);
		}
			
		ImGui::Separator();

		ImGui::Text("Registers");
		{
			ImGui::Indent();
			ImGui::Columns(2, "Registers");
			for (int i = 0; i < 16; i++){
				ImGui::Text("r%-2d: 0x%08x", i, *registers.main[i]);
				if (i == 7) ImGui::NextColumn();
			}
			ImGui::Columns(1);
			ImGui::Unindent();
		}
		auto showStatusRegister = [](ProgramStatusRegister value) {
			ImGui::Text("Actual Value: 0x%08x", value.value);
			ImGui::Text("Mode: %hhu (%s)", value.mode, ModeString(value.mode).c_str());
			ImGui::Text("Exec State: %s", (value.state == CPUExecState::ARM) ? "ARM" : "Thumb");
			ImGui::Text("FIQ: %d", !value.fiqDisable); ImGui::SameLine();
			ImGui::Text("IRQ: %d", !value.irqDisable);
			ImGui::Text("N Z C V");
			if (ImGui::IsItemHovered())
			{
				ImGui::BeginTooltip();
				ImGui::Text("Negative Zero Carry oVerflow");
				ImGui::EndTooltip();
			}
			
			ImGui::Text("%d %d %d %d", value.negative, value.zero, value.carry, value.overflow);
		};
			
		ImGui::Separator();
			
		if (registers.spsr){
			ImGui::BeginChild("CPSR", ImVec2(ImGui::GetContentRegionAvailWidth() / 2, 0));
		}
		{
			ImGui::Text("CPSR");
			ImGui::Indent();
			showStatusRegister(*registers.cpsr);
			ImGui::Unindent();
		}
		if (registers.spsr){
			ImGui::EndChild();
			ImGui::SameLine();
			ImGui::BeginChild("SPSR", ImVec2(0, 0));
			{
				ImGui::Text("SPSR");
				ImGui::Indent();
				showStatusRegister(*registers.spsr);
				ImGui::Unindent();
			}
			ImGui::EndChild();
		}
	}
	
	void CPUStateWindow::drawCustomWindowContents(){
		ImGui::BeginChild("Sidebar", ImVec2(175, 0));
		{
			ImGui::Text("Breakpoints");
			ImGui::Indent();
			int index = 0;
			int indexPendingRemoval = -1;
			for (word breakpoint : cpu.breakpoints){
				ImGui::PushID(index);
				ImGui::Text("0x%08x", breakpoint); ImGui::SameLine();
				if (ImGui::SmallButton("X")){
					indexPendingRemoval = index;
				}
				ImGui::PopID();
				index++;
			}
			if (indexPendingRemoval >= 0) cpu.breakpoints.erase(cpu.breakpoints.begin() + indexPendingRemoval);
			ImGui::Text("0x"); ImGui::SameLine();
			ImGui::PushItemWidth(10 * 8);
			ImGui::InputText("", newBreakpointIndex, 9, ImGuiInputTextFlags_CharsHexadecimal); ImGui::SameLine();
			ImGui::PopItemWidth();
			if (ImGui::Button("+")){
				cpu.breakpoints.push_back(std::strtoul(newBreakpointIndex, nullptr, 16));
			}
			ImGui::Unindent();

			ImGui::BeginChild("CPU History", ImVec2(0,0), true);
			for (int i = 0; i < stateHistory.size(); i++){
				const auto* statePipelineBaseData = stateHistory[i].currentPipelineBaseData();

				if (!statePipelineBaseData->hasDecodedInstruction && i != stateHistory.size() - 1) continue;
				
				char buf[15];
				sprintf(buf, "0x%08x %c%c%c",
						statePipelineBaseData->decodedInstructionAddress,
						statePipelineBaseData->hasFetchedInstruction ? 'F' : '/',
						statePipelineBaseData->hasDecodedInstruction ? 'D' : '/',
						statePipelineBaseData->hasExecutedInstruction ? 'E' : '/'
					);
				bool selected = selectedStateIndex == i;

				ImGui::PushID(i);
				if (ImGui::Selectable(buf, &selected)){
					selectedStateIndex = i;
				}
				if (selected && teleportToSelected){
					ImGui::SetScrollHere();
					teleportToSelected = false;
				}
				ImGui::PopID();
			}
			ImGui::EndChild();
		}
		ImGui::EndChild();

		ImGui::SameLine();

		ImGui::BeginChild("CPU DATA");
		{
			if (selectedStateIndex != stateHistory.size() - 1){
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,0,0,1));
				ImGui::TextWrapped("Warning: This is based on a previous CPU state. Any memory values this uses may have changed.");
				ImGui::PopStyleColor();
				ImGui::Separator();
			}
			drawCPUState(stateHistory[selectedStateIndex]);
		}
		ImGui::EndChild();
		
		//firstRun = false;
		//lastDecoded = pipeline.decodedInstructionAddress;
	}
}
