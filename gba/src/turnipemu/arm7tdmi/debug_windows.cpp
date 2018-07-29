#include "turnipemu/arm7tdmi/debug_windows.h"

#include "turnipemu/arm7tdmi/cpu.h"

namespace TurnipEmu::ARM7TDMI::Debug {
	CPUStateWindow::CPUStateWindow(CPU& cpu)
		: Display::CustomWindow("ARM7TDMI Instruction Inspector", 450, 0), cpu(cpu) {}
	void CPUStateWindow::drawCustomWindowContents(){
		/*static*/ word interpreting = cpu.pipeline.decodedInstructionAddress;
		//static word lastDecoded = pipeline.decodedInstructionAddress;
		//static char jumpBuf[9] = "00000000";
		//static bool firstRun = true;

		ImGui::Text("Pipeline Status");
		{
			ImGui::Indent();
			if (cpu.pipeline.hasFetchedInstruction){
				ImGui::Text("Fetched Address: 0x%08x", cpu.pipeline.fetchedInstructionAddress);
				if (cpu.pipeline.hasDecodedInstruction){
					ImGui::Text("Decoded Address: 0x%08x", cpu.pipeline.decodedInstructionAddress);
					if (cpu.pipeline.hasExecutedInstruction){
						ImGui::Text("Executed Address: 0x%08x", cpu.pipeline.executedInstructionAddress);
					}
				}
			}else{
				ImGui::Text("Pipeline Flushed");
			}
			ImGui::Unindent();
		}

		ImGui::Separator();
		
		/*ImGui::Text("Custom Address 0x"); ImGui::SameLine();
		ImGui::PushItemWidth(8 * 20); // 8 chars * font size?
		ImGui::InputText("", jumpBuf, 9, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		jumpBuf[8] = '\0';
		
		bool shouldInspect = ImGui::Button("Inspect");
		bool shouldReset = firstRun || (lastDecoded != pipeline.decodedInstructionAddress);
		
		if (!shouldReset && interpreting != pipeline.decodedInstructionAddress) {
			ImGui::SameLine();
			shouldReset = ImGui::Button("Reset to PC");
		}
		shouldInspect |= shouldReset;
		if (shouldReset){
			for (int i = 0; i < 8; i++){
				uint8_t charVal = (pipeline.decodedInstructionAddress >> (28 - i * 4)) & 0xF;
				if (charVal <= 9) jumpBuf[i] = '0' + charVal;
				else jumpBuf[i] = 'A' + (charVal - 0xA);
			}
		}
		if (shouldInspect){
			interpreting = std::strtoul(jumpBuf, nullptr, 16);
			}*/

		const RegisterPointers currentRegisters = cpu.registersForCurrentState();

		if (cpu.pipeline.hasDecodedInstruction){
			ImGui::Text("Just Decoded Instruction");
			ImGui::Indent();
			
			if (auto instructionWordOptional = cpu.memoryMap.read<word>(interpreting, false)){
				word instructionWord = instructionWordOptional.value();
				Instruction* instruction = cpu.matchInstruction(instructionWord);
			
				if (instruction){
					const Instruction::Condition& condition = instruction->getCondition(instructionWord);
					ImGui::Text("0x%08x: 0x%08x %c%c", interpreting, instructionWord, condition.name[0], condition.name[1]);
					{
						ImGui::Indent();
						ImGui::Text("Condition Code: %s [Fulfilled: %d]", condition.debugString.c_str(), condition.fulfilsCondition(*currentRegisters.cpsr));
						ImGui::Text("Instruction Category: %s", instruction->category.c_str());
						ImGui::TextWrapped("Instruction Disassembly: %s", instruction->disassembly(instructionWord).c_str());
						ImGui::Unindent();
					}
				}else{
					ImGui::Text("0x%08x: 0x%08x (Not an instruction)", interpreting, instructionWord);
				}
			}else{
				ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,0,0,1));
				ImGui::Text("Not in a valid memory location!");
				ImGui::PopStyleColor();
			}

			ImGui::Unindent();
			ImGui::Separator();
		}

		ImGui::Text("CPU State");

		ImGui::Text("Registers");
		{
			ImGui::Indent();
			ImGui::Columns(2, "Registers");
			for (int i = 0; i < 16; i++){
				ImGui::Text("r%-2d: 0x%08x", i, *currentRegisters.main[i]);
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
		{
			ImGui::Text("Current Status");
			ImGui::Indent();
			showStatusRegister(*currentRegisters.cpsr);
			ImGui::Unindent();
		}
		if (currentRegisters.spsr){
			ImGui::Text("Stored Status");
			ImGui::Indent();
			showStatusRegister(*currentRegisters.spsr);
			ImGui::Unindent();
		}	

		//firstRun = false;
		//lastDecoded = pipeline.decodedInstructionAddress;
	}

	CPUHistoryWindow::CPUHistoryWindow(CPU& cpu)
		: Display::CustomWindow("ARM7TDMI History", 450, 0), cpu(cpu) {}
	void CPUHistoryWindow::addCurrentStateToHistory(){
		// TODO: Define
	}
	void CPUHistoryWindow::drawCustomWindowContents(){
		// TODO: Define
	}
}
