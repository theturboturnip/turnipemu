#include "arm7tdmi.h"
#include "log.h"
#include <memory>
#include <string>

#include "imgui/imgui.h"

namespace TurnipEmu{
#define CONDITION(NAME, CODE) ARM7TDMI::Instruction::Condition{ {NAME[0], NAME[1]}, {#CODE}, [](ARM7TDMI::ProgramStatusRegister status) { return CODE; } }
	const std::array<const ARM7TDMI::Instruction::Condition, 15> ARM7TDMI::Instruction::conditions = {
		CONDITION("EQ", status.zero),
		CONDITION("NE", !status.zero),
		CONDITION("CS", status.carry),
		CONDITION("CC", !status.carry),
		CONDITION("MI", status.negative),
		CONDITION("PL", !status.negative),
		CONDITION("VS", status.overflow),
		CONDITION("VC", !status.overflow),
		CONDITION("HI", status.carry && !status.zero),
		CONDITION("LS", !status.carry || status.zero),
		CONDITION("GE", status.negative == status.overflow),
		CONDITION("LT", status.negative != status.overflow),
		CONDITION("GT", !status.zero && (status.negative == status.overflow)),
		CONDITION("LE", status.zero || (status.negative != status.overflow)),
		CONDITION("AL", true),
	};
#undef CONDITION
	
	ARM7TDMI::Instruction::Instruction(std::string category, InstructionMask mask)
		: category(category), mask(mask)  {
		LogLine("INST", "Created Instruction with category %s, mask 0x%08x, value 0x%08x", category.c_str(), mask.mask, mask.expectedValue);
	}

	const ARM7TDMI::Instruction::Condition& ARM7TDMI::Instruction::getCondition(word instructionWord) {
		return conditions[(instructionWord >> 28) & 0xF];
	}
	ARM7TDMI::InstructionMask::InstructionMask(std::initializer_list<MaskRange> list){
		mask = 0;
		expectedValue = 0;
		for (const auto& range : list){
			range.updateMask(mask);
			range.updateExpectedValue(expectedValue);
		}
	}

	ARM7TDMI::ARM7TDMI(const MemoryMap& memoryMap) : Display::CustomWindow("ARM7TDMI", 0, 0), memoryMap(memoryMap)
	{
		setupInstructions();
		reset();
	}
	void ARM7TDMI::reset(){
		memset(&registers, sizeof(registers), 0);
		registers.regs[15] = 0x08000000; // Start at the beginning of ROM
	}
	void ARM7TDMI::setupInstructions(){
		instructions = std::vector<std::unique_ptr<Instruction>>();
		instructions.push_back(std::make_unique<Instruction>("Software Interrupt", InstructionMask{
					{27, 24, 0b1111}
				}));
		instructions.push_back(std::make_unique<Instruction>("Undefined", InstructionMask{
					{27, 25, 0b011},
					{4, 1}
				}));
		instructions.push_back(std::make_unique<Instruction>("Branch and Exchange", InstructionMask{
					{27, 4, 0b0'0001'0010'1111'1111'1111'0001}
				}));
		instructions.push_back(std::make_unique<Instruction>("Branch", InstructionMask{
					{27, 25, 0b101}
				}));
		instructions.push_back(std::make_unique<Instruction>("Multiply", InstructionMask{
					{27, 23, 0b00000},
					{7, 4, 0b1001}
				}));
		instructions.push_back(std::make_unique<Instruction>("Single Data Swap", InstructionMask{
					{27, 23, 0b00010},
					{21, 20, 0b00},
					{11, 4, 0b0'0000'1001}
				}));
		instructions.push_back(std::make_unique<Instruction>("Halfword Data Transfer", InstructionMask{
					{27, 25, 0b000},
					{7, 1},
					{4, 1}
				}));
		instructions.push_back(std::make_unique<Instruction>("Data Processing", InstructionMask{
					{27, 26, 0b00}
				}));
		instructions.push_back(std::make_unique<Instruction>("Single Data Transfer", InstructionMask{
					{27, 26, 0b01}
				}));
		instructions.push_back(std::make_unique<Instruction>("Block Data Transfer", InstructionMask{
					{27, 26, 0b10}
				}));
		instructions.push_back(std::make_unique<Instruction>("Coprocessor Ops", InstructionMask{
					{27, 26, 0b11}
				}));
	}
	ARM7TDMI::Instruction* ARM7TDMI::matchInstruction(word instructionWord){
		for (auto& instructionUniquePtr : instructions){
			if (instructionUniquePtr->mask.matches(instructionWord))
				return instructionUniquePtr.get();
		}
		return nullptr;
	}

	void ARM7TDMI::drawCustomWindowContents(){
		static word interpreting = registers.regs[15];
		static char buf[9] = "00000000";
		ImGui::Text("PC: 0x%08x", registers.regs[15]);
		ImGui::Text("Jump To 0x"); ImGui::SameLine();
		ImGui::InputText("", buf, 9, ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase);
		buf[8] = '\0';
		bool correctHex = true;
		for (char character : buf){
			if (character == '\0') break;
			bool charFits = (character >= '0' && character <= '9') ||
				(character >= 'A' && character <= 'F') ||
				(character >= 'a' && character <= 'f');
			if (!charFits){
				correctHex = false;
				break;
			}
		}
		if (correctHex){
			bool shouldInspect = ImGui::Button("Inspect"); ImGui::SameLine();
			bool shouldJump = ImGui::Button("Jump Manually");
			if (shouldInspect){
				interpreting = std::strtoul(buf, nullptr, 16);
			}else if (shouldJump){
				interpreting = registers.regs[15] = std::strtoul(buf, nullptr, 16);
			}
		}else{
			ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,0,0,1));
			ImGui::Text("Not Valid Hex!");
			ImGui::PopStyleColor();
		}

		if (auto instructionWordOptional = memoryMap.read<word>(interpreting, false)){
			word instructionWord = instructionWordOptional.value();
			Instruction* instruction = matchInstruction(instructionWord);
			if (instruction){
				const Instruction::Condition& condition = instruction->getCondition(instructionWord);
				condition.fulfilsCondition(ProgramStatusRegister{});
				ImGui::Text("0x%08x: 0x%08x %c%c", interpreting, instructionWord, condition.name[0], condition.name[1]);
				{
					ImGui::Indent();
					ImGui::Text("Condition Code: %s", condition.debugString.c_str());
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
