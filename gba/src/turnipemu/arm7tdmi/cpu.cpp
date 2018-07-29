#include "turnipemu/arm7tdmi/cpu.h"

#include <memory>
#include <string>

#include "turnipemu/log.h"
#include "turnipemu/imgui.h"
#include "turnipemu/utils.h"

namespace TurnipEmu::ARM7TDMI{
	CPU::CPU(const Memory::Map& memoryMap) : memoryMap(memoryMap), debugStateWindow(*this), debugHistoryWindow(*this)
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
		pipeline.decodedArmInstruction = nullptr;
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
		}else{
			if (pipeline.hasFetchedInstruction){
				if (pipeline.hasDecodedInstruction){
					// Execute previously decoded instruction
					if (pipeline.decodedArmInstruction->getCondition(pipeline.decodedInstructionWord).fulfilsCondition(*currentRegisters.cpsr)){
						// This can flush the pipeline.
						word oldPC = registers.pc();
						pipeline.decodedArmInstruction->execute(*this, currentRegisters, pipeline.decodedInstructionWord);
						if (oldPC != registers.pc() && !pipeline.queuedFlush){
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
				pipeline.decodedArmInstruction = matchArmInstruction(pipeline.decodedInstructionWord);
				if (!pipeline.decodedArmInstruction) throw std::runtime_error("Couldn't decode instruction!");
				pipeline.hasDecodedInstruction = true;
			}else{
				assert(!pipeline.hasDecodedInstruction);
			}

			if (pipeline.queuedFlush){
				flushPipeline();
			}else{
				// Fetch the instruction
				if (auto instructionWordOptional = memoryMap.read<word>(registers.pc())){
					pipeline.fetchedInstructionAddress = registers.pc();
					pipeline.fetchedInstructionWord = instructionWordOptional.value();
					pipeline.hasFetchedInstruction = true;
				}else{
					throw std::runtime_error("PC is in invalid memory!");
				}
			
				registers.pc() += 4; // TODO: Specialize for Thumb
			}
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
}
