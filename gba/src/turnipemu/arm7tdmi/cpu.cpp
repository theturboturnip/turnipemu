#include "turnipemu/arm7tdmi/cpu.h"

#include <memory>
#include <string>

#include "turnipemu/emulator.h"
#include "turnipemu/imgui.h"
#include "turnipemu/log.h"
#include "turnipemu/utils.h"

namespace TurnipEmu::ARM7TDMI{
	CPU::CPU(Emulator& emulator, const Memory::Map& memoryMap) : emulator(emulator), memoryMap(memoryMap), debugStateWindow(*this)
	{
		setupInstructions();
		reset();
	}
	void CPU::reset(){
		memset(&state.registers, 0, sizeof(state.registers));

		breakpoints = { 0xc28 };

		constexpr bool RunBIOS = true;
		if constexpr (RunBIOS){
			state.registers.main[15] = 0x00000000;
			state.registers.cpsr.mode = Mode::Supervisor;
			state.registers.cpsr.state = CPUExecState::ARM;
			state.registers.cpsr.fiqDisable = true;
			state.registers.cpsr.irqDisable = true;
		}else{
			// TODO: There are more requirements to boot outside of the BIOS
			state.registers.main[15] = 0x08000000; // Start at the beginning of ROM
			state.registers.cpsr.mode = Mode::System;
			state.registers.cpsr.state = CPUExecState::ARM;
		}

		switch(state.registers.cpsr.state){
		case CPUExecState::ARM:
			state.armPipeline.flush();
			break;
		case CPUExecState::Thumb:
			state.thumbPipeline.flush();
			break;
		default:
			assert(false);
		}

		debugStateWindow.reset();
	}

	
	void CPU::tick(){
		try{
			const auto currentRegisters = state.usableRegisters();
			const CPUExecState oldExecState = state.registers.cpsr.state;

			if (currentRegisters.cpsr->state == CPUExecState::Thumb){
				state.thumbPipeline.tick(*this, currentRegisters, CPU::matchThumbInstruction);
			}else{
				state.armPipeline.tick(*this, currentRegisters, CPU::matchArmInstruction);
			}

			const CPUExecState newExecState = state.registers.cpsr.state;
			if (oldExecState != newExecState){
				switch(newExecState){
				case CPUExecState::ARM:
					state.armPipeline.flush();
					break;
				case CPUExecState::Thumb:
					state.thumbPipeline.flush();
					break;
				default:
					assert(false);
				}
			}

			auto* pipeline = state.currentPipelineBaseData();
			if (pipeline->hasDecodedInstruction &&
				std::find(breakpoints.begin(), breakpoints.end(), pipeline->decodedInstructionAddress) != breakpoints.end()){
				emulator.pause();
			}

			debugStateWindow.onCPUTick();
		}catch(...){
			debugStateWindow.onCPUTick();
			
			throw;
		}
	}

	const PipelineBase* CPUState::currentPipelineBaseData(){
		switch(registers.cpsr.state){
		case CPUExecState::ARM:
			return static_cast<const PipelineBase*>(&armPipeline);
		case CPUExecState::Thumb:
			return static_cast<const PipelineBase*>(&thumbPipeline);
			break;
		default:
			assert(false);
			return nullptr;
		}
	}
	
	const RegisterPointers CPUState::usableRegisters() {
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
