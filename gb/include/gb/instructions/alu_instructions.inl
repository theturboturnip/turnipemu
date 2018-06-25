// Copyright Samuel Stark 2017

#pragma once

#include "instruction_sources.h"

namespace GB::Instructions::ALU{

	// Adds
	template<typename T, typename AddToSource, typename AddFromSource, bool WithCarry>
	class AddInstruction{};
	template<typename AddToSource, typename AddValueSource, bool WithCarry>
	class AddInstruction<uint8_t, AddToSource, AddValueSource, WithCarry> : public Instruction{
		using Instruction::Instruction;

		STATIC_ASSERT_IS_SOURCE_INTERFACE(AddToSource, uint8_t, SourceUsage::ReadAndWrite);
		//STATIC_ASSERT_IS_SOURCE_INTERFACE(AddValueSource, uint8_t, SourceUsage::ReadOnly);
		static_assert(AddValueSource::load);
		
	public:
		uint8_t execute(CPU& cpu) override{
			bool should_carry = WithCarry && cpu.is_flag_set(CPUFlag::Carry);
			uint8_t add_from = AddValueSource::load(cpu);
			uint8_t add_to = AddToSource::load(cpu);
			int addition_result = add_from + add_to + (should_carry ? 1 : 0);
			uint8_t wrapped_result = addition_result & 0xff;

			cpu.set_flag(CPUFlag::Zero, wrapped_result == 0);
			cpu.set_flag(CPUFlag::Negative, false);
			cpu.set_flag(CPUFlag::Carry, (addition_result >> 8) & 1);
			// The "& 0xf" zeroes out the top 4 bits.
			// A Half Carry occurs if the result of the bottom 4 bits added together
			// has the 5th bit set. "& 0x10" makes the number 0 if the 5th bit isn't set,
			// or 0x10 if it is.
			cpu.set_flag(CPUFlag::HalfCarry, (((add_from & 0xf) + (add_to & 0xf) + (should_carry ? 1 : 0)) >> 4) & 1);
			
			AddToSource::store(cpu, wrapped_result);

			return 4 + AddValueSource::cycles + AddToSource::cycles;
		}
	};
	template<typename AddToSource, typename AddValueSource>
	class AddInstruction<uint16_t, AddToSource, AddValueSource, false> : public Instruction{
		using Instruction::Instruction;

		STATIC_ASSERT_IS_SOURCE_INTERFACE(AddToSource, uint16_t, SourceUsage::ReadAndWrite);
		//STATIC_ASSERT_IS_SOURCE_INTERFACE(AddValueSource, uint16_t, SourceUsage::ReadOnly);
		static_assert(AddValueSource::load);
		
	public:
		uint8_t execute(CPU& cpu) override{
			uint16_t add_from = AddValueSource::load(cpu);
			uint16_t add_to = AddToSource::load(cpu);
			int addition_result = add_from + add_to;
			uint16_t wrapped_result = addition_result & 0xffff;

			// Ignore Zero flag
			cpu.set_flag(CPUFlag::Negative, false);
			cpu.set_flag(CPUFlag::Carry, (addition_result >> 16) & 1);
			// The "& 0xff" zeroes out the top 8 bits.
			// A Half Carry occurs if the result of the bottom 8 bits added together
			// has the 5th bit set. "& 0x100" makes the number 0 if the 9th bit isn't set,
			// or 0x100 if it is.
			cpu.set_flag(CPUFlag::HalfCarry, (((add_from & 0xfff) + (add_to & 0xfff)) & 0x1000) == 0x1000);

			AddToSource::store(cpu, wrapped_result);

			return 4 + AddValueSource::cycles + AddToSource::cycles;
		}
	};

    // Subtracts
	template<typename SubFromSource, typename SubValueSource, bool WithCarry>
	class SubInstruction : public Instruction{
		using Instruction::Instruction;

		STATIC_ASSERT_IS_SOURCE_INTERFACE(SubFromSource, uint8_t, SourceUsage::ReadAndWrite);
		STATIC_ASSERT_IS_SOURCE_INTERFACE(SubValueSource, uint8_t, SourceUsage::ReadOnly);

		uint8_t execute(CPU& cpu) override{
			bool should_carry = WithCarry && cpu.is_flag_set(CPUFlag::Carry);
			uint8_t sub_from = SubFromSource::load(cpu);
			uint8_t sub_value = SubValueSource::load(cpu);
			int subtraction_result = sub_from - sub_value - (should_carry ? 1 : 0);
			uint8_t wrapped_result = static_cast<uint8_t>(subtraction_result);

			if (CPU::extended_debug_data){
				fprintf(stdout, "sub_from = %d, sub_value = %d, result = %d (wrapped %d)\n", sub_from, sub_value, subtraction_result, wrapped_result);
			}

			cpu.set_flag(CPUFlag::Zero, wrapped_result == 0);
			cpu.set_flag(CPUFlag::Negative, true);
			cpu.set_flag(CPUFlag::Carry, subtraction_result < 0);
			// The "& 0xf" zeroes out the top 4 bits.
			// A Half Carry occurs if the result of the bottom 4 bits subtracted is < 0,
			// i.e. if the bottom 4 bits of sub_value > the bottom 4 bits of sub_from
			cpu.set_flag(CPUFlag::HalfCarry, ((sub_value & 0xf) + (should_carry ? 1 : 0)) > (sub_from & 0xf));
			
			SubFromSource::store(cpu, wrapped_result);
		
			return 4 + SubFromSource::cycles + SubValueSource::cycles;
		}
	};


    // Logical Ops
	template<typename IntoSource, typename ValueSource>
	class AndInstruction : public Instruction{
		using Instruction::Instruction;

		STATIC_ASSERT_IS_SOURCE_INTERFACE(IntoSource, uint8_t, SourceUsage::ReadAndWrite);
		STATIC_ASSERT_IS_SOURCE_INTERFACE(ValueSource, uint8_t, SourceUsage::ReadOnly);
		
		uint8_t execute(CPU& cpu) override{
			auto result = IntoSource::load(cpu) & ValueSource::load(cpu);

			cpu.set_flag(CPUFlag::Zero,      result == 0);
			cpu.set_flag(CPUFlag::Negative,  false);
			cpu.set_flag(CPUFlag::Carry,     false);
			cpu.set_flag(CPUFlag::HalfCarry, true);
		
			IntoSource::store(cpu, result);
		
			return 4 + IntoSource::cycles + ValueSource::cycles;
		}
	};
	template<typename IntoSource, typename ValueSource>
	class OrInstruction : public Instruction{
		using Instruction::Instruction;

		STATIC_ASSERT_IS_SOURCE_INTERFACE(IntoSource, uint8_t, SourceUsage::ReadAndWrite);
		STATIC_ASSERT_IS_SOURCE_INTERFACE(ValueSource, uint8_t, SourceUsage::ReadOnly);

		uint8_t execute(CPU& cpu) override{
			auto result = IntoSource::load(cpu) | ValueSource::load(cpu);

			cpu.set_flag(CPUFlag::Zero,      result == 0);
			cpu.set_flag(CPUFlag::Negative,  false);
			cpu.set_flag(CPUFlag::Carry,     false);
			cpu.set_flag(CPUFlag::HalfCarry, false);
		
			IntoSource::store(cpu, result);
		
			return 4 + IntoSource::cycles + ValueSource::cycles;
		}
	};
	template<typename IntoSource, typename ValueSource>
	class XorInstruction : public Instruction{
		using Instruction::Instruction;

		STATIC_ASSERT_IS_SOURCE_INTERFACE(IntoSource, uint8_t, SourceUsage::ReadAndWrite);
		STATIC_ASSERT_IS_SOURCE_INTERFACE(ValueSource, uint8_t, SourceUsage::ReadOnly);

		uint8_t execute(CPU& cpu) override{
			auto result = IntoSource::load(cpu) ^ ValueSource::load(cpu);

			cpu.set_flag(CPUFlag::Zero,      result == 0);
			cpu.set_flag(CPUFlag::Negative,  false);
			cpu.set_flag(CPUFlag::Carry,     false);
			cpu.set_flag(CPUFlag::HalfCarry, false);
		
			IntoSource::store(cpu, result);
		
			return 4 + IntoSource::cycles + ValueSource::cycles;
		}
	};
	template<typename ASource, typename BSource>
	class CompareInstruction : public Instruction{
		using Instruction::Instruction;

		STATIC_ASSERT_IS_SOURCE_INTERFACE(ASource, uint8_t, SourceUsage::ReadAndWrite);
		STATIC_ASSERT_IS_SOURCE_INTERFACE(BSource, uint8_t, SourceUsage::ReadOnly);

		uint8_t execute(CPU& cpu) override{
			uint8_t a = ASource::load(cpu);
			uint8_t b = BSource::load(cpu);

			if (CPU::extended_debug_data){
				fprintf(stdout, "Comparing 0x%02x (%d dec) to 0x%02x (%d dec)\n", a, a, b, b);
			}

			cpu.set_flag(CPUFlag::Zero,      a == b);
			cpu.set_flag(CPUFlag::Negative,  true);
			cpu.set_flag(CPUFlag::Carry,     b > a);
			cpu.set_flag(CPUFlag::HalfCarry, (b & 0xf) > (a & 0xf));
		
			return 4 + ASource::cycles + BSource::cycles;
		}
	};
	template<typename FromSource, typename IntoSource>
	class NotInstruction : public Instruction{
		using Instruction::Instruction;

		STATIC_ASSERT_IS_SOURCE_INTERFACE(FromSource, uint8_t, SourceUsage::ReadAndWrite);
		STATIC_ASSERT_IS_SOURCE_INTERFACE(IntoSource, uint8_t, SourceUsage::ReadOnly);

		uint8_t execute(CPU& cpu) override{
			auto result = ~FromSource::load(cpu);

			cpu.set_flag(CPUFlag::Negative,  true);
			cpu.set_flag(CPUFlag::HalfCarry, true);
		
			IntoSource::store(cpu, result);
		
			return IntoSource::cycles + FromSource::cycles;
		}
	};

	template<typename ValueType, typename InSource>
	class IncrementInstruction{};
	template<typename InSource>
	class IncrementInstruction<uint8_t, InSource> : public Instruction{
		using Instruction::Instruction;

		STATIC_ASSERT_IS_SOURCE_INTERFACE(InSource, uint8_t, SourceUsage::ReadAndWrite);

		uint8_t execute(CPU& cpu) override{
			uint8_t value = InSource::load(cpu);
			uint8_t new_value = value + 1;
			InSource::store(cpu, new_value);

			cpu.set_flag(CPUFlag::Zero, new_value == 0);
			cpu.set_flag(CPUFlag::Negative, false);
			// Ignore Carry flag
			// If the last 4 bits are all set then the increment resulted in a half-carry
			cpu.set_flag(CPUFlag::HalfCarry, (value & 0xf) == 0xf);
		
			return InSource::cycles;
		};
	};
	template<typename InSource>
	class IncrementInstruction<uint16_t, InSource> : public Instruction{
		using Instruction::Instruction;

		STATIC_ASSERT_IS_SOURCE_INTERFACE(InSource, uint16_t, SourceUsage::ReadAndWrite);

		uint8_t execute(CPU& cpu) override{
			uint16_t value = InSource::load(cpu);
			uint16_t new_value = value + 1;
			InSource::store(cpu, new_value);
		
			return InSource::cycles;
		};
	};
	template<typename ValueType, typename InSource>
	class DecrementInstruction{};
	template<typename InSource>
	class DecrementInstruction<uint8_t, InSource> : public Instruction{
		using Instruction::Instruction;

		STATIC_ASSERT_IS_SOURCE_INTERFACE(InSource, uint8_t, SourceUsage::ReadAndWrite);

		uint8_t execute(CPU& cpu) override{
			uint8_t value = InSource::load(cpu);
			uint8_t new_value = value - 1;
			InSource::store(cpu, new_value);

			cpu.set_flag(CPUFlag::Zero, new_value == 0);
			cpu.set_flag(CPUFlag::Negative, true);
			// Ignore Carry flag
			// If the last 4 bits are all zero then the decrement resulted in a half-carry?
			cpu.set_flag(CPUFlag::HalfCarry, (value & 0xf) == 0x0);
		
			return InSource::cycles;
		};
	};
	template<typename InSource>
	class DecrementInstruction<uint16_t, InSource> : public Instruction{
		using Instruction::Instruction;

		STATIC_ASSERT_IS_SOURCE_INTERFACE(InSource, uint16_t, SourceUsage::ReadAndWrite);

		uint8_t execute(CPU& cpu) override{
			uint16_t value = InSource::load(cpu);
			uint16_t new_value = value - 1;
			InSource::store(cpu, new_value);
		
			return InSource::cycles;
		};
	};

	template<typename InSource, bool RotateLeft, bool IncludeCarry, bool IsCB = false>
	class RotateInstruction : public Instruction{
		using Instruction::Instruction;

		STATIC_ASSERT_IS_SOURCE_INTERFACE(InSource, uint8_t, SourceUsage::ReadAndWrite);

		uint8_t execute(CPU& cpu) override{
			uint8_t value = InSource::load(cpu);
			uint8_t rotated = rotate_value(cpu, value);
			InSource::store(cpu, rotated);

			// This is a special case, Rotates always take at least 4 cycles even though they only use one register
			if (IsCB){
				return (4 + InSource::cycles) * 2; // TODO: This seems tenuous at best.
			}
			return 4;
		};

		static inline uint8_t rotate_value(CPU& cpu, uint8_t value){
			uint8_t result;
			if (RotateLeft){
				result = (value << 1);
				if (IncludeCarry){
					result = result | (cpu.is_flag_set(CPUFlag::Carry) ? 0b1 : 0b0);
				}else{
					result = result | ((value & 0b10000000) ? 0b1 : 0b0);
				}
				cpu.set_flag(CPUFlag::Carry, value & 0b10000000);
			}else{
				result = (value >> 1);
				if (IncludeCarry){
					result = result | (cpu.is_flag_set(CPUFlag::Carry) ? 0b10000000 : 0b0);
				}else{
					result = result | ((value & 0b1) ? 0b10000000 : 0b0);
				}
				cpu.set_flag(CPUFlag::Carry, value & 0b1);
			}
			cpu.set_flag(CPUFlag::Zero, IsCB ? (result == 0) : false);
			cpu.set_flag(CPUFlag::Negative, false);
			cpu.set_flag(CPUFlag::HalfCarry, false);
			return result;
		}
	};
}
