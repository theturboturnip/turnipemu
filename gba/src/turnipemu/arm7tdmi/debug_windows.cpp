#include "turnipemu/arm7tdmi/debug_windows.h"

#include "turnipemu/arm7tdmi/cpu.h"

namespace TurnipEmu::ARM7TDMI::Debug {
	CPUStateWindow::CPUStateWindow(CPU& cpu)
		: Display::CustomWindow("ARM7TDMI Instruction Inspector", 450, 0), cpu(cpu) {}

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

		ImGui::Separator();

		if (pipeline.hasDecodedInstruction){
			ImGui::Text("Just Decoded Instruction");
			ImGui::Indent();


			const auto* instructionCategory = pipeline.decodedInstructionCategory;
			if (instructionCategory){
				const auto& condition = instructionCategory->getCondition(pipeline.decodedInstruction);
				ImGui::Text(sizeof(PipelineType::decodedInstruction) == 4 ? "0x%08x: 0x%08x %s" : "0x%08x: 0x%04x %s",
							pipeline.decodedInstructionAddress, pipeline.decodedInstruction, condition.name);
				{
					ImGui::Indent();
					ImGui::Text("Condition Code: %s [Fulfilled: %d]", condition.debugString.c_str(), condition.fulfilsCondition(*currentRegisters.cpsr));
					ImGui::TextWrapped("Instruction Category: %s", instructionCategory->name.c_str());
					ImGui::TextWrapped("Instruction Disassembly: %s", instructionCategory->disassembly(pipeline.decodedInstruction).c_str());
					ImGui::Unindent();
				}
			}else{
				ImGui::Text("0x%08x: 0x%08x (Not an instruction)", pipeline.decodedInstructionAddress, pipeline.decodedInstruction);
			}

			ImGui::Unindent();
			ImGui::Separator();
		}
	}
	
	void CPUStateWindow::drawCustomWindowContents(){

		const RegisterPointers currentRegisters = cpu.registersForCurrentState();

		if (currentRegisters.cpsr->state == CPUExecState::Thumb){
			displayPipeline(currentRegisters, cpu.thumbPipeline);
		}else{
			displayPipeline(currentRegisters, cpu.armPipeline);
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
