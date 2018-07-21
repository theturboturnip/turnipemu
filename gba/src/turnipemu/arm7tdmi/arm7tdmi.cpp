#include "arm7tdmi.h"
#include "log.h"
#include <memory>
#include <string>

#include "imgui/imgui.h"

namespace TurnipEmu::ARM7TDMI{
	CPU::CPU(const MemoryMap& memoryMap) : Display::CustomWindow("ARM7TDMI", 0, 0), memoryMap(memoryMap)
	{
		setupInstructions();
		reset();
	}
	void CPU::reset(){
		memset(&registers, sizeof(registers), 0);
		registers.regs[15] = 0x08000000; // Start at the beginning of ROM
	}

	void CPU::drawCustomWindowContents(){
		static word interpreting = registers.regs[15];
		static char jumpBuf[9] = "00000000";
		ImGui::Text("PC: 0x%08x", registers.regs[15]);
		ImGui::Text("Jump To 0x"); ImGui::SameLine();
		ImGui::InputText("", jumpBuf, 9, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
		jumpBuf[8] = '\0';
		
		bool reset = ImGui::Button("Reset"); ImGui::SameLine();
		bool shouldInspect = ImGui::Button("Inspect"); ImGui::SameLine();
		bool shouldJump = ImGui::Button("Jump Manually");
		if (reset){
			for (int i = 0; i < 8; i++){
				uint8_t charVal = (interpreting >> (28- i * 4)) & 0xF;
				if (charVal <= 9) jumpBuf[i] = '0' + charVal;
				else jumpBuf[i] = 'A' + (charVal - 0xA);
			}
		}
		if (shouldInspect){
			interpreting = std::strtoul(jumpBuf, nullptr, 16);
		}
		if (shouldJump){
			interpreting = registers.regs[15] = std::strtoul(jumpBuf, nullptr, 16);
		}

		if (auto instructionWordOptional = memoryMap.read<word>(interpreting, false)){
			word instructionWord = instructionWordOptional.value();
			Instruction* instruction = matchInstruction(instructionWord);
			if (instruction){
				const Instruction::Condition& condition = instruction->getCondition(instructionWord);
				ImGui::Text("0x%08x: 0x%08x %c%c", interpreting, instructionWord, condition.name[0], condition.name[1]);
				{
					ImGui::Indent();
					ImGui::Text("Condition Code: %s [Fulfilled: %d]", condition.debugString.c_str(), 0);
					ImGui::Text("Instruction Category: %s", instruction->category.c_str());
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
	}
}
