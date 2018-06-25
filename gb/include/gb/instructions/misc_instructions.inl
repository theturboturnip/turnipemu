// Copyright Samuel Stark 2017

#pragma once

#include "gb/cpu.h"
#include "instruction.h"
#include "instruction_sources.h"

#include <cstdint>

namespace GB::Instructions{

	class NoopInstruction : public Instruction{	
		using Instruction::Instruction;
		uint8_t execute(CPU& cpu) override{
			return 4;
		}
	};
	class HaltInstruction : public Instruction{
		using Instruction::Instruction;
		uint8_t execute(CPU& cpu) override{
			cpu.halted = true;
			cpu.interrupts.find_next_interrupt();
			return 4;
		}
	};
	class StopInstruction : public Instruction{
		using Instruction::Instruction;

		uint8_t execute(CPU& cpu) override{
			cpu.registers.pc++; // TODO: Necessary?
			return 4;
		}
	};

	template<typename StoreToType, typename LoadFromType>
	class LoadInstruction : public Instruction{
		using Instruction::Instruction;
	
		uint8_t execute(CPU& cpu) override{
			auto val = LoadFromType::load(cpu);
			StoreToType::store(cpu, val);
			return 4 + LoadFromType::cycles + StoreToType::cycles;
		}
	};

	enum class JumpCondition{
		Always,
		Zero,
		NotZero,
		Carry,
		NotCarry
	};
	enum class JumpMode{
		SignedOffset,
		AbsoluteValue
	};
	template<JumpCondition Condition>
	class JumpInstructionBase : public Instruction{
		using Instruction::Instruction;

	protected:
		bool should_jump(CPU& cpu){
			switch(Condition){
			case JumpCondition::Always:
				return true;
			case JumpCondition::Zero:
				return cpu.is_flag_set(CPUFlag::Zero);
			case JumpCondition::NotZero:
				return !cpu.is_flag_set(CPUFlag::Zero);
			case JumpCondition::Carry:
				return cpu.is_flag_set(CPUFlag::Carry);
			case JumpCondition::NotCarry:
				return !cpu.is_flag_set(CPUFlag::Carry);
			default:
				assert(false && "Invalid Jump Condition");
				return true;
			}
		}
	};
	template<JumpCondition Condition, JumpMode Mode, typename JumpValueType>
	class JumpInstruction{};
	template<JumpCondition Condition, typename JumpValueType>
	class JumpInstruction<Condition, JumpMode::SignedOffset, JumpValueType> : public JumpInstructionBase<Condition>{
		using JumpInstructionBase<Condition>::JumpInstructionBase;

		uint8_t execute(CPU& cpu){
			int8_t jump_by = static_cast<int8_t>(JumpValueType::load(cpu));
			if (!JumpInstructionBase<Condition>::should_jump(cpu)) return 8;

			if (CPU::extended_debug_data)
				fprintf(stdout, "jump_by: 0x%02x (%d dec)\n", jump_by, jump_by);
			cpu.jump_to(cpu.registers.pc + jump_by);
		
			return 8 + JumpValueType::cycles;
		}
	};
	template<JumpCondition Condition, typename JumpValueType>
	class JumpInstruction<Condition, JumpMode::AbsoluteValue, JumpValueType> : public JumpInstructionBase<Condition>{
		using JumpInstructionBase<Condition>::JumpInstructionBase;

		uint8_t execute(CPU& cpu){
			uint16_t jump_to = JumpValueType::load(cpu);
			if (!JumpInstructionBase<Condition>::should_jump(cpu)) return JumpValueType::cycles + 4;

			cpu.jump_to(jump_to);
		
			return 4 + 4 + JumpValueType::cycles;
		}
	};

	template<JumpCondition Condition>
	class ReturnInstruction : public JumpInstructionBase<Condition>{
		using JumpInstructionBase<Condition>::JumpInstructionBase;

		uint8_t execute(CPU& cpu){
			if (!JumpInstructionBase<Condition>::should_jump(cpu)) return 8;
			cpu.jump_to(cpu.pop_from_stack());
			return 20;
		}
	};
	template<>
	class ReturnInstruction<JumpCondition::Always> : public Instruction{
		using Instruction::Instruction;

	protected:
		uint8_t execute(CPU& cpu){
			cpu.jump_to(cpu.pop_from_stack());
			return 16;
		}
	};
	class ReturnInterruptInstruction : public ReturnInstruction<JumpCondition::Always>{
		using ReturnInstruction<JumpCondition::Always>::ReturnInstruction;

		uint8_t execute(CPU& cpu){
			if (CPU::debug_data){
				fprintf(stdout, "Returning from Interrupt!\n");
			}
			cpu.interrupts.enable();
			return ReturnInstruction<JumpCondition::Always>::execute(cpu);
		}
	};


	template<typename PopInto>
	class PopStackInstruction : public Instruction{
		using Instruction::Instruction;

		uint8_t execute(CPU& cpu){
			uint16_t value = cpu.pop_from_stack();
			PopInto::store(cpu, value);
			return 12;
		}
	};
	template<typename PushFrom>
	class PushStackInstruction : public Instruction{
		using Instruction::Instruction;

		uint8_t execute(CPU& cpu){
			cpu.push_to_stack(PushFrom::load(cpu));
			return 16;
		}
	};

	template<uint16_t address>
	class CallRoutineInstruction : public Instruction{
		using Instruction::Instruction;

		uint8_t execute(CPU& cpu){
			cpu.push_to_stack(cpu.registers.pc);
			cpu.jump_to(address);
			return 16;
		}
	};
	template<typename LocationSource, JumpCondition Condition = JumpCondition::Always>
	class CallInstruction : public JumpInstructionBase<Condition>{
		using JumpInstructionBase<Condition>::JumpInstructionBase;

		uint8_t execute(CPU& cpu){
			uint16_t location = LocationSource::load(cpu);
			if (JumpInstructionBase<Condition>::should_jump(cpu)){
				cpu.push_to_stack(cpu.registers.pc);
				cpu.jump_to(location);
				return 16 + LocationSource::cycles;
			}else{
				return 8 + LocationSource::cycles;
			}
		}
	};

	template<bool EnableInterrupts>
	class SetInterruptsEnabledInstruction : public Instruction{
		using Instruction::Instruction;

		uint8_t execute(CPU& cpu){
			if (EnableInterrupts){
				cpu.interrupts.enable();
			}else{
				cpu.interrupts.disable();
			}
			return 4;
		}
	};

	class ComplementCarryFlagInstruction : public Instruction{
		using Instruction::Instruction;

		uint8_t execute(CPU& cpu){
			cpu.set_flag(CPUFlag::Negative,  false);
			cpu.set_flag(CPUFlag::Carry,     !cpu.is_flag_set(CPUFlag::Carry));
			cpu.set_flag(CPUFlag::HalfCarry, false);
		
			return 4;
		}
	};


	template<typename InType>
	class BCDCorrectInstruction : public Instruction{
		using Instruction::Instruction;

		uint8_t execute(CPU& cpu){
			uint8_t value = InType::load(cpu);
			
			int low_digit = (value & 0x0F);
			int high_digit = (value >> 4);
			int result = value;
			
			if (cpu.is_flag_set(CPUFlag::Negative)){
				if (cpu.is_flag_set(CPUFlag::HalfCarry)){
					result -= 6;
					if (!cpu.is_flag_set(CPUFlag::Carry)){
						result &= 0xFF;
					}
				}
				if (cpu.is_flag_set(CPUFlag::Carry)){
					result -= 0x60;
				}
			}else{
				if (low_digit > 9 || cpu.is_flag_set(CPUFlag::HalfCarry)){
					result += 0x06;
					high_digit = (result >> 4);
				}
				if (high_digit > 9 || cpu.is_flag_set(CPUFlag::Carry)){
					result += 0x60;
				}
			}
			cpu.set_flag(CPUFlag::Carry, ((result & 0x100) != 0) || cpu.is_flag_set(CPUFlag::Carry));
			cpu.set_flag(CPUFlag::HalfCarry, false);
			cpu.set_flag(CPUFlag::Zero, (result & 0xFF) == 0);
			
			InType::store(cpu, static_cast<uint8_t>(result & 0xFF));

			return 4;
		}
	};

	uint16_t AddSigned8BitToValue(CPU& cpu, int add_to, unsigned int add_amount){
		if (add_amount & 0x80) add_amount |= -256;
		uint32_t result = add_to + add_amount;

		cpu.set_flag(CPUFlag::Zero, false);
		cpu.set_flag(CPUFlag::Negative, false);

		// This adds the high byte first for whatever reason. Hence the carry flags are based on the low byte addition.
		cpu.set_flag(CPUFlag::Carry,     (((add_amount & 0xff) + (add_to & 0xff)) & 0x100) == 0x100);
		cpu.set_flag(CPUFlag::HalfCarry, (((add_amount & 0xf) + (add_to & 0xf)) & 0x10) == 0x10);
		return result;
	}
	template<typename StoreSource, typename AddToSource, typename AddAmountSource>
	class LDHLInstruction : public Instruction{
		using Instruction::Instruction;

		uint8_t execute(CPU& cpu){
			int add_to = AddToSource::load(cpu);
			unsigned int add_amount = AddAmountSource::load(cpu);

			uint16_t result = AddSigned8BitToValue(cpu, add_to, add_amount);
			
			StoreSource::store(cpu, result);
			//AddToSource::store(cpu, result);
			
			return 12;
		}
	};
	class AddSigned8BitImmediateToSPInstruction : public Instruction{
		using Instruction::Instruction;

		uint8_t execute(CPU& cpu){
			int add_to = Sources::RegisterSP::load(cpu);
			unsigned int add_amount = Sources::Operand<uint8_t>::load(cpu);

			uint16_t result = AddSigned8BitToValue(cpu, add_to, add_amount);
			
			Sources::RegisterSP::store(cpu, result);

			return 16;
		}
	};

	template<bool CarryValue>
	class SetCarryFlagInstruction : public Instruction{
		using Instruction::Instruction;

		uint8_t execute(CPU& cpu){
			cpu.set_flag(CPUFlag::Carry, CarryValue);
			cpu.set_flag(CPUFlag::Negative, false);
			cpu.set_flag(CPUFlag::HalfCarry, false);
			return 4;
		}
	};
}
