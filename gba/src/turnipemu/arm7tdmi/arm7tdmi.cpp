#include "turnipemu/arm7tdmi.h"

#include <memory>
#include <string>

#include "turnipemu/log.h"
#include "turnipemu/imgui.h"

namespace TurnipEmu::ARM7TDMI{
	CPU::CPU(const Memory::Map& memoryMap) : Display::CustomWindow("ARM7TDMI Instruction Inspector", 450, 0), memoryMap(memoryMap)
	{
		setupInstructions();
		reset();
	}
	void CPU::reset(){
		memset(&registers, 0, sizeof(registers));

		constexpr bool RunBIOS = true;
		if constexpr (RunBIOS){
			registers.main[15] = 0x00000000;
			registers.cpsr.mode = Mode::Supervisor;
			registers.cpsr.state = CPUExecState::ARM;
			registers.cpsr.fiqDisable = true;
			registers.cpsr.irqDisable = true;
		}else{
			// TODO: There are more requirements to boot outside of the BIOS
			registers.main[15] = 0x08000000; // Start at the beginning of ROM
			registers.cpsr.mode = Mode::System;
			registers.cpsr.state = CPUExecState::ARM;
		}
	}

	void CPU::executeNextInstruction(){
		if (auto instructionWordOptional = memoryMap.read<word>(registers.main[15])){
			executeInstruction(instructionWordOptional.value());
		}else{
			throw std::runtime_error("PC is in invalid memory!");
		}
	}
	void CPU::executeInstruction(word instructionWord){
		const auto currentRegisters = registersForCurrentState();
		if (currentRegisters.cpsr->state == CPUExecState::Thumb){
			throw std::runtime_error("Thumb mode is not supported yet!");
		}
		
		Instruction* instruction = matchInstruction(instructionWord);
		if (instruction){
			if (instruction->getCondition(instructionWord).fulfilsCondition(*currentRegisters.cpsr)){
				instruction->execute(*this, currentRegisters, instructionWord);
			}
			registers.main[15] += 4;
		}else{
			throw std::runtime_error("Couldn't match instructionWord to an actual instruction!");
		}
	}

	const RegisterPointers CPU::registersForCurrentState() {
		RegisterPointers pointers;
			
		switch (registers.cpsr.mode){
		case Mode::User:
		case Mode::Supervisor:
			for (int i = 0; i < 16; i++){
				pointers.main[i] = &registers.main[i];
			}
			pointers.cpsr = &registers.cpsr;
			break;
		default:
			assert(false);
		}
		
		return pointers;
	}

	void CPU::drawCustomWindowContents(){
		static word interpreting = registers.main[15];
		static word lastPC = registers.main[15];
		static char jumpBuf[9] = "00000000";
		static bool firstRun = true;
		
		ImGui::Text("PC: 0x%08x", registers.main[15]);
		ImGui::Text("Custom Address 0x"); ImGui::SameLine();
		ImGui::PushItemWidth(8 * 20); // 8 chars * font size?
		ImGui::InputText("", jumpBuf, 9, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
		ImGui::PopItemWidth();
		ImGui::SameLine();
		jumpBuf[8] = '\0';
		
		bool shouldInspect = ImGui::Button("Inspect");
		bool shouldReset = firstRun || (lastPC != registers.main[15]);
		
		if (!shouldReset && interpreting != registers.main[15]) {
			ImGui::SameLine();
			shouldReset = ImGui::Button("Reset to PC");
		}
		shouldInspect |= shouldReset;
		if (shouldReset){
			for (int i = 0; i < 8; i++){
				uint8_t charVal = (registers.main[15] >> (28 - i * 4)) & 0xF;
				if (charVal <= 9) jumpBuf[i] = '0' + charVal;
				else jumpBuf[i] = 'A' + (charVal - 0xA);
			}
		}
		if (shouldInspect){
			interpreting = std::strtoul(jumpBuf, nullptr, 16);
		}

		const RegisterPointers currentRegisters = registersForCurrentState();
		
		if (auto instructionWordOptional = memoryMap.read<word>(interpreting, false)){
			word instructionWord = instructionWordOptional.value();
			Instruction* instruction = matchInstruction(instructionWord);
			
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

		ImGui::Separator();
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
			ImGui::Text("Mode: %hhu", value.mode);
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

		firstRun = false;
		lastPC = registers.main[15];
	}
}
