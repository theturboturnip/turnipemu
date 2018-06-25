// Copyright Samuel Stark 2017

#pragma once

#include <cstdint>
#include <array>

namespace GB{
	class CPU;
	
	enum class Interrupt{
		VBlank = 1 << 0,
		LcdStat = 1 << 1,
		Timer = 1 << 2,
		Serial = 1 << 3,
		Joypad = 1 << 4
	};
	struct InterruptData{
		Interrupt key;
		uint16_t handler_pc;
		uint8_t flag_value;
		const char* const string;
	};
	const std::array<InterruptData, 5> interrupts = {{
			{ Interrupt::VBlank,  0x40, 1 << 0, "VBlank"},
			{ Interrupt::LcdStat, 0x48, 1 << 1, "LCDStat"},
			{ Interrupt::Timer,   0x50, 1 << 2, "Timer"},
			{ Interrupt::Serial,  0x58, 1 << 3, "Serial"},
			{ Interrupt::Joypad,  0x60, 1 << 4, "Joypad"},
		}};

	class Interrupts{
	public:
		Interrupts(CPU& cpu) : cpu(cpu){}
		
		void trigger(Interrupt interrupt);
		inline void disable(){
			master_enabled = false;
		}
		inline void enable(){
			master_enabled = true;
			find_next_interrupt();
		}

		void reset();

		void find_next_interrupt();
		inline const InterruptData* next_interrupt(){
			return cached_next_interrupt;
		}

		uint8_t flagged;
		uint8_t enabled;
	protected:
		bool master_enabled;
		CPU& cpu;

		const InterruptData* cached_next_interrupt = nullptr;
	};
}
