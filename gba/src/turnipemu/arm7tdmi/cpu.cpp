#include "turnipemu/arm7tdmi/cpu.h"

#include <memory>
#include <string>

#include "turnipemu/log.h"
#include "turnipemu/imgui.h"
#include "turnipemu/utils.h"

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

		flushPipeline();
	}
	void CPU::queuePipelineFlush(){
		pipeline.queuedFlush = true;
	}
	void CPU::flushPipeline(){
		pipeline.hasFetchedInstruction = false;
		pipeline.fetchedInstructionWord = 0x0;
		pipeline.fetchedInstructionAddress = registers.main[15];
		
		pipeline.hasDecodedInstruction = false;
		pipeline.decodedInstruction = nullptr;
		pipeline.decodedInstructionWord = 0x0;
		pipeline.decodedInstructionAddress = 0x0;

		pipeline.hasExecutedInstruction = false;

		pipeline.queuedFlush = false;
	}

	void CPU::tick(){
		tickPipeline();
	}
	void CPU::tickPipeline(){
		const auto currentRegisters = registersForCurrentState();
		if (currentRegisters.cpsr->state == CPUExecState::Thumb){
			throw std::runtime_error("Thumb mode is not supported yet!");
		}
		
		if (pipeline.hasFetchedInstruction){
			if (pipeline.hasDecodedInstruction){
				// Execute previously decoded instruction
				if (pipeline.decodedInstruction->getCondition(pipeline.decodedInstructionWord).fulfilsCondition(*currentRegisters.cpsr)){
					// This can flush the pipeline.
					word oldPC = registers.main[15];
					pipeline.decodedInstruction->execute(*this, currentRegisters, pipeline.decodedInstructionWord);
					if (oldPC != registers.main[15] && !pipeline.queuedFlush){
						LogLine(logTag, "PC was changed, but a flush was not setup! Instruction Word: 0x%08x", pipeline.decodedInstructionWord);
						LogLine(logTag, "Pipeline will be automatically flushed");
						queuePipelineFlush();
					}
				}

				pipeline.hasExecutedInstruction = true;
				pipeline.executedInstructionAddress = pipeline.decodedInstructionAddress;
			}

			// Decode fetched instruction
			pipeline.decodedInstructionAddress = pipeline.fetchedInstructionAddress;
			pipeline.decodedInstructionWord = pipeline.fetchedInstructionWord;
			pipeline.decodedInstruction = matchInstruction(pipeline.decodedInstructionWord);
			if (!pipeline.decodedInstruction) throw std::runtime_error("Couldn't decode instruction!");
			pipeline.hasDecodedInstruction = true;
		}else{
			assert(!pipeline.hasDecodedInstruction);
		}

		if (pipeline.queuedFlush){
			flushPipeline();
		}else{
            // Fetch the instruction
			if (auto instructionWordOptional = memoryMap.read<word>(registers.main[15])){
				pipeline.fetchedInstructionAddress = registers.main[15];
				pipeline.fetchedInstructionWord = instructionWordOptional.value();
				pipeline.hasFetchedInstruction = true;
			}else{
				throw std::runtime_error("PC is in invalid memory!");
			}
			
			registers.main[15] += 4; // TODO: Specialize for Thumb
		}
	}
	
	const RegisterPointers CPU::registersForCurrentState() {
		RegisterPointers pointers;

		for (int i = 0; i < 16; i++){
			pointers.main[i] = &registers.main[i];
		}
		pointers.cpsr = &registers.cpsr;
		
		switch (registers.cpsr.mode){
		case Mode::User:
		case Mode::System:
			break;
		case Mode::Supervisor:
			pointers.main[13] = &registers.svc.r13;
			pointers.main[14] = &registers.svc.r14;
			pointers.spsr = &registers.svc.spsr;
			break;
		case Mode::FIQ:
			pointers.main[8] = &registers.fiq.r8;
			pointers.main[9] = &registers.fiq.r9;
			pointers.main[10] = &registers.fiq.r10;
			pointers.main[11] = &registers.fiq.r11;
			pointers.main[12] = &registers.fiq.r12;
			pointers.main[13] = &registers.fiq.r13;
			pointers.main[14] = &registers.fiq.r14;
			pointers.spsr = &registers.fiq.spsr;
			break;
		case Mode::IRQ:
			pointers.main[13] = &registers.irq.r13;
			pointers.main[14] = &registers.irq.r14;
			pointers.spsr = &registers.irq.spsr;
			break;
		case Mode::Abort:
			pointers.main[13] = &registers.abt.r13;
			pointers.main[14] = &registers.abt.r14;
			pointers.spsr = &registers.abt.spsr;
			break;
		case Mode::Undefined:
			pointers.main[13] = &registers.und.r13;
			pointers.main[14] = &registers.und.r14;
			pointers.spsr = &registers.und.spsr;
			break;
		default:
			// This can still happen, if an invalid value is written to the CPSR by the program.
			throw std::runtime_error(Utils::streamFormat("Illegal Mode for CPSR. Value = ", (int)registers.cpsr.mode, " which is '", ModeString(registers.cpsr.mode), "'"));
		}
		
		return pointers;
	}

	void CPU::drawCustomWindowContents(){
		/*static*/ word interpreting = pipeline.decodedInstructionAddress;
		//static word lastDecoded = pipeline.decodedInstructionAddress;
		//static char jumpBuf[9] = "00000000";
		//static bool firstRun = true;

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

		const RegisterPointers currentRegisters = registersForCurrentState();

		if (pipeline.hasDecodedInstruction){
			ImGui::Text("Just Decoded Instruction");
			ImGui::Indent();
			
		
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

			ImGui::Unindent();
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
}
