// Copyright Samuel Stark 2017

#pragma once

#include <cstdint>
#include <array>
#include <vector>
#include <map>

#include "gb/cartridge.h"
#include "gb/mmu.h"
#include "gb/gpu.h"
#include "gb/input.h"
#include "gb/interrupts.h"
#include "gb/rom_data.h"
#include "gb/instructions/instruction_set.h"
#include "gb/timer.h"

namespace GB{
	enum class CPUFlag{
		Zero = 1 << 7,
		Negative = 1 << 6,
		HalfCarry = 1 << 5,
		Carry = 1 << 4
	};
	
	class CPU{
	public:
		friend class GPU;
		
		CPU(std::array<uint8_t, MMU::BIOS_SIZE> bios, std::vector<uint8_t> rom, void (*on_vblank)(CPU&));
		void reset();
		void step();
		template<typename T>
		T load_operand();

		bool is_flag_set(CPUFlag flag);
		void set_flag(CPUFlag flag, bool new_value);

		void jump_to(uint16_t new_pc);

		void push_to_stack(uint16_t value);
		uint16_t pop_from_stack();

		void exit_bios();

		void check_instructions();
	
		struct Registers{
			union {
				struct {
					uint8_t f;
					uint8_t a;
				};
				uint16_t af;
			};
			union {
				struct {
					uint8_t c;
					uint8_t b;
				};
				uint16_t bc;
			};
			union {
				struct {
					uint8_t e;
					uint8_t d;
				};
				uint16_t de;
			};
			union {
				struct {
					uint8_t l;
					uint8_t h;
				};
				uint16_t hl;
			};
		
			uint16_t sp;
			uint16_t pc;
		} registers;
		MMU mmu;
		GPU gpu;
		Input input;
		Interrupts interrupts;
		Cartridge cartridge;
		Timer timer;
	
		unsigned int clock_cycles = 0;
		unsigned int clock_cycles_this_step = 0;
		bool stopped = false;
		bool halted = false;
		bool manual_step_requested = false;
		bool waiting_for_ret = false;
		constexpr static bool allow_debug = false;
		constexpr static bool allow_debug_during_loops = false;
		constexpr static bool allow_extended_debug = false;
		static bool debug_data;
		static bool extended_debug_data;
		static bool limit_fps;
	protected:
		static GB::Instructions::InstructionSet instruction_set;

		uint16_t loop_check[3];
	
		void (*on_vblank)(CPU&); // TODO: This should take const GPU
	
		void load_rom(std::vector<uint8_t> rom);

		size_t current_operand_size = 0;
		uint16_t current_operand = 0;
		uint16_t pending_cpu_increment = 0;

		bool within_bios = true;
	};
}
