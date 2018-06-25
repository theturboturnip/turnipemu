// Copyright Samuel Stark 2017

#pragma once

#include "gb/cpu.h"

#include <type_traits>

namespace GB::Instructions::Sources{
	enum class SourceUsage{
		ReadOnly,
		WriteOnly,
		ReadAndWrite
	};
	template<typename SourceToTest, typename ReturnType, SourceUsage Usage>
	struct StaticAssertIsSourceInterface{

		static ReturnType load_template(CPU& cpu);
		static void store_template(CPU& cpu, ReturnType value);
		
		static_assert(std::is_same<ReturnType, uint8_t>::value || std::is_same<ReturnType, uint16_t>::value, "Source return type must be uint8/uint16");
		static_assert(std::is_same<decltype(SourceToTest::cycles), const uint8_t>::value, "Source must contain a const static uint8 declaring how many cycles it takes to use");
		static_assert((Usage == SourceUsage::WriteOnly) || std::is_same<decltype(SourceToTest::load), decltype(load_template)>::value, "Readable sources must have a 'static ReturnType load(CPU& cpu)' function");
		static_assert((Usage == SourceUsage::ReadOnly) || std::is_same<decltype(SourceToTest::store), decltype(store_template)>::value, "Writeable sources must have a 'static void store(CPU& cpu, ReturnType value)' function");

		constexpr static bool value = true;
	};
#define STATIC_ASSERT_IS_SOURCE_INTERFACE(SourceToTest, ReturnType, Usage) static_assert(GB::Instructions::Sources::StaticAssertIsSourceInterface<SourceToTest, ReturnType, Usage>::value, "Source was not well-defined")

	template<typename ValueType, typename Source, int Offset>
	struct OffsetOnLoad{
		constexpr static uint8_t cycles = Source::cycles;

		static inline ValueType load(CPU& cpu){
			STATIC_ASSERT_IS_SOURCE_INTERFACE(Source, ValueType, SourceUsage::ReadAndWrite);

			ValueType current_value = Source::load(cpu);
			Source::store(cpu, current_value + Offset);

			return current_value;
		}
		constexpr static bool store = false;
	};
	template<typename ValueType, typename Source>
	using IncrementOnLoad = OffsetOnLoad<ValueType, Source, 1>;
	template<typename ValueType, typename Source>
	using DecrementOnLoad = OffsetOnLoad<ValueType, Source, -1>;

	template<typename ValueType, typename Source, int Offset>
	struct LoadWithOffset{
		constexpr static uint8_t cycles = Source::cycles;

		static inline ValueType load(CPU& cpu){
			static_assert(Source::load);
						
			ValueType to_return = Source::load(cpu) + Offset;
			return to_return;
		}
		constexpr static bool store = false;
	};
	
	template<typename ValueType, ValueType CPU::Registers::*RegisterPointer>
	struct CPURegister{
		constexpr static uint8_t cycles = 0;
	
		static inline ValueType load(CPU& cpu){
			return cpu.registers.*RegisterPointer;
		}
		static inline void store(CPU& cpu, ValueType val){
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wformat"
			if (CPU::debug_data){
				fprintf(stdout, "        [WRIT] 0x%04x (%d dec) -> r%p\n", val, val, RegisterPointer);
			}
#pragma clang diagnostic pop
		
			cpu.registers.*RegisterPointer = val;
		}
	};
	using RegisterA = CPURegister<uint8_t, &CPU::Registers::a>;
	using RegisterB = CPURegister<uint8_t, &CPU::Registers::b>;
	using RegisterC = CPURegister<uint8_t, &CPU::Registers::c>;
	using RegisterD = CPURegister<uint8_t, &CPU::Registers::d>;
	using RegisterE = CPURegister<uint8_t, &CPU::Registers::e>;
	using RegisterH = CPURegister<uint8_t, &CPU::Registers::h>;
	using RegisterL = CPURegister<uint8_t, &CPU::Registers::l>;
	using RegisterAF = CPURegister<uint16_t, &CPU::Registers::af>;
	using RegisterBC = CPURegister<uint16_t, &CPU::Registers::bc>;
	using RegisterDE = CPURegister<uint16_t, &CPU::Registers::de>;
	using RegisterHL = CPURegister<uint16_t, &CPU::Registers::hl>;
	using RegisterSP = CPURegister<uint16_t, &CPU::Registers::sp>;

	STATIC_ASSERT_IS_SOURCE_INTERFACE(RegisterC, uint8_t, SourceUsage::ReadOnly);
	
	template<typename ValueType>
	struct Operand{
		constexpr static uint8_t cycles = sizeof(ValueType)*4;
	
		static inline ValueType load(CPU& cpu){
			return cpu.load_operand<ValueType>();
		}
		constexpr static bool store = false;
	};
	
	template<typename ValueType, typename PointerSource>
	struct Pointer{};
	template<typename PointerSource>
	struct Pointer<uint8_t, PointerSource>{
		STATIC_ASSERT_IS_SOURCE_INTERFACE(PointerSource, uint16_t, SourceUsage::ReadOnly);
		
		constexpr static uint8_t cycles = 4;// + LoadPointerFromType::cycles; // TODO: Use this?
	
		static inline uint8_t load(CPU& cpu){
			uint16_t address = PointerSource::load(cpu);
			uint8_t val = cpu.mmu.read_byte(address);
			if (CPU::debug_data){
				fprintf(stdout, "        [READ] (0x%04x) == 0x%02x (%d dec)\n", address, val, val);
			}
			return val;
		}
		static inline void store(CPU& cpu, uint8_t val){
			uint16_t address = PointerSource::load(cpu);
			if (CPU::debug_data){
				fprintf(stdout, "        [WRIT] 0x%02x (%d dec) -> (0x%04x)\n", val, val, address);
			}
			cpu.mmu.write_byte(address, val);
		}
	};
	template<typename PointerSource>
	struct Pointer<uint16_t, PointerSource>{
		STATIC_ASSERT_IS_SOURCE_INTERFACE(PointerSource, uint16_t, SourceUsage::ReadOnly);

		constexpr static uint8_t cycles = 8;
	
		static inline uint16_t load(CPU& cpu){
			return cpu.mmu.read_word(PointerSource::load(cpu));
		}
		static inline void store(CPU& cpu, uint16_t val){
			uint16_t address = PointerSource::load(cpu);
			if (CPU::debug_data){
				fprintf(stdout, "        [WRIT] 0x%04x (%d dec) -> (0x%04x)\n", val, val, address);
			}
			cpu.mmu.write_word(address, val);
		}
	};
	template<typename ValueType>
	using PointerFromOperand = Pointer<ValueType, Operand<uint16_t>>;
	STATIC_ASSERT_IS_SOURCE_INTERFACE(PointerFromOperand<uint8_t>, uint8_t, SourceUsage::ReadAndWrite);
	STATIC_ASSERT_IS_SOURCE_INTERFACE(PointerFromOperand<uint16_t>, uint16_t, SourceUsage::ReadAndWrite);
	template<typename PointerSource>
	using PointerFromOffsetFF00 = Pointer<uint8_t, LoadWithOffset<uint16_t, PointerSource, 0xFF00>>;
	STATIC_ASSERT_IS_SOURCE_INTERFACE(PointerFromOffsetFF00<RegisterC>, uint8_t, SourceUsage::ReadAndWrite);
};
using GB::Instructions::Sources::SourceUsage;
