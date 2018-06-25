// Copyright Samuel Stark 2017

#pragma once

#include "alu_instructions.inl"

namespace GB::Instructions::CB{
	class UnknownInstruction : public Instruction{
		using Instruction::Instruction;

		uint8_t execute(CPU& cpu) override{
			fprintf(stderr, "--- NOTE ---\nCB instruction opcode was 0x%02x\n", cpu.load_operand<uint8_t>());
			return Instruction::execute(cpu);
		}
	};
	
	template<typename IntoSource>
	using RL  = ALU::RotateInstruction<IntoSource, true,  true,  true>;
	template<typename IntoSource>
	using RLC = ALU::RotateInstruction<IntoSource, true,  false, true>;
	template<typename IntoSource>
	using RR  = ALU::RotateInstruction<IntoSource, false, true,  true>;
	template<typename IntoSource>
	using RRC = ALU::RotateInstruction<IntoSource, false, false, true>;
	
	template<typename Source, int Bit>
	class TestBitInstruction : public Instruction{
		using Instruction::Instruction;
		
		STATIC_ASSERT_IS_SOURCE_INTERFACE(Source, uint8_t, SourceUsage::ReadOnly);
	
		uint8_t execute(CPU& cpu) override{
			uint8_t value = Source::load(cpu);
			if (CPU::extended_debug_data){
				fprintf(stdout, "Testing bit %d of value %02x (%d dec)\n", Bit, value, value);
			}
			cpu.set_flag(CPUFlag::Zero, !(value & (1 << Bit)));
			cpu.set_flag(CPUFlag::Negative, false);
			cpu.set_flag(CPUFlag::HalfCarry, true);

			return 8 + Source::cycles*2;
		}
	};
	template<typename Source, int Bit>
	using BIT = TestBitInstruction<Source, Bit>;
	
	template<typename Source, int Bit>
	class SetBitInstruction : public Instruction{
		using Instruction::Instruction;

		STATIC_ASSERT_IS_SOURCE_INTERFACE(Source, uint8_t, SourceUsage::ReadAndWrite);
	
		uint8_t execute(CPU& cpu) override{
			uint8_t value = Source::load(cpu);
			value = value | (1 << Bit);
			Source::store(cpu, value);
			return 8 + Source::cycles*2;
		}
	};
	template<typename Source, int Bit>
	using SET = SetBitInstruction<Source, Bit>;

	template<typename Source, int Bit>
	class ResetBitInstruction : public Instruction{
		using Instruction::Instruction;

		STATIC_ASSERT_IS_SOURCE_INTERFACE(Source, uint8_t, SourceUsage::ReadAndWrite);
	
		uint8_t execute(CPU& cpu) override{
			uint8_t value = Source::load(cpu);
			value = value & ~(1 << Bit);
			Source::store(cpu, value);
			return 8 + Source::cycles*2;
		}
	};
	template<typename Source, int Bit>
	using RES = ResetBitInstruction<Source, Bit>;

	template<typename Source>
	class SwapNybblesInstruction : public Instruction{
		using Instruction::Instruction;

		STATIC_ASSERT_IS_SOURCE_INTERFACE(Source, uint8_t, SourceUsage::ReadAndWrite);

		uint8_t execute(CPU& cpu) override{
			uint8_t value = Source::load(cpu);
			uint8_t result = ((value & 0b00001111) << 4) | ((value & 0b11110000) >> 4);
			Source::store(cpu, result);

			cpu.set_flag(CPUFlag::Zero, result == 0);
			cpu.set_flag(CPUFlag::Carry, false);//value & 0x01);
			cpu.set_flag(CPUFlag::HalfCarry, false);//value & 0x01);
			cpu.set_flag(CPUFlag::Negative, false);//value & 0x01);
			
			return 8 + Source::cycles*2;
		}
	};
	template<typename Source>
	using SWAP = SwapNybblesInstruction<Source>;

	template<typename Source, bool ShiftLeft, bool ConserveSign>
	class ShiftInstruction : public Instruction{
		using Instruction::Instruction;

		STATIC_ASSERT_IS_SOURCE_INTERFACE(Source, uint8_t, SourceUsage::ReadAndWrite);
		
		uint8_t execute(CPU& cpu) override{
			uint8_t value = Source::load(cpu);
			uint8_t result;
			if (ShiftLeft){
				result = value << 1;
				cpu.set_flag(CPUFlag::Carry, value & 0x80);
			}else{
				result = value >> 1;
				cpu.set_flag(CPUFlag::Carry, value & 0x01);
			}
			if (ConserveSign){
				result = (value & 0b10000000) | (result & 0b011111111); 
			}

			cpu.set_flag(CPUFlag::Zero, result == 0);
			cpu.set_flag(CPUFlag::HalfCarry, false);
			cpu.set_flag(CPUFlag::Negative, false);

			Source::store(cpu, result);
			return 8 + Source::cycles*2;
		}
	};
	template<typename Source>
	using SLA = ShiftInstruction<Source, true, false>;
	template<typename Source>
	using SRA = ShiftInstruction<Source, false, true>;
	template<typename Source>
	using SRL = ShiftInstruction<Source, false, false>;
}
