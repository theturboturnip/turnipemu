// Copyright Samuel Stark 2017

#include "gb/cpu.h"
#include "gb/rom_data.h"

#include <cstdlib>
#include <cstring>
#include <assert.h>

namespace GB{
	Instructions::InstructionSet CPU::instruction_set;

	bool CPU::debug_data = CPU::allow_debug;
	bool CPU::extended_debug_data = CPU::allow_extended_debug && CPU::debug_data;
	bool CPU::limit_fps = true;

	CPU::CPU(std::array<uint8_t, MMU::BIOS_SIZE> bios, std::vector<uint8_t> rom, void (*on_vblank)(CPU&)) : mmu(*this), gpu(*this), input(*this), interrupts(*this), cartridge(std::move(rom)), timer(*this), on_vblank(on_vblank){
		mmu.load_bios(std::move(bios));

		reset();
	}

	void CPU::reset(){
		clock_cycles = 0;
		clock_cycles_this_step = 0;
		stopped = false;
		halted = false;
		manual_step_requested = false;

		loop_check[0] = 0xffff;
		loop_check[1] = 0xffff;
		loop_check[2] = 0xffff;
	
		// Reset the memory
		registers.a = 0x01;
		registers.f = 0xb0;
		registers.b = 0x00;
		registers.c = 0x13;
		registers.d = 0x00;
		registers.e = 0xd8;
		registers.h = 0x01;
		registers.l = 0x4d;
		registers.sp = 0xfffe;
		registers.pc = 0x000;

		within_bios = true;

		mmu.reset();
		gpu.reset();
		input.reset();
		interrupts.reset();
		cartridge.reset();
	}

	void CPU::check_instructions(){
		instruction_set.print_all();
	}

	template<>
	uint8_t CPU::load_operand<uint8_t>(){
		if (current_operand_size == 0){
			current_operand = static_cast<uint16_t>(mmu.read_byte(registers.pc));
			if (debug_data && !within_bios){
				fprintf(stdout, "        [ OP ] 0x%02x (%d dec)\n", current_operand, current_operand);
			}
			registers.pc += sizeof(uint8_t);
			current_operand_size = sizeof(uint8_t);
		}
		return current_operand;
	}
	template<>
	uint16_t CPU::load_operand<uint16_t>(){
		if (current_operand_size == 0){
			current_operand = mmu.read_word(registers.pc);
			if (debug_data && !within_bios){
				fprintf(stdout, "        [ OP ] 0x%04x (%d dec)\n", current_operand, current_operand);
			}
			registers.pc += sizeof(uint16_t);
			current_operand_size = sizeof(uint16_t);
		}
		return current_operand;
	}

	void CPU::exit_bios(){
		mmu.unload_bios();
		within_bios = false;
		//stopped = true;
	}

	bool CPU::is_flag_set(CPUFlag flag){
		return registers.f & static_cast<uint8_t>(flag);
	}
	void CPU::set_flag(CPUFlag flag, bool should_set){
		if (should_set){
			registers.f = registers.f | static_cast<uint8_t>(flag);
			/*if (CPU::extended_debug_data){
			  fprintf(stdout, "Set flag 0x%02x to true\n", static_cast<uint8_t>(flag));
			  }*/
			assert(is_flag_set(flag));
		}else{
			registers.f = registers.f & ~(static_cast<uint8_t>(flag));
			/*if (CPU::extended_debug_data){
			  fprintf(stdout, "Set flag 0x%02x to false\n", static_cast<uint8_t>(flag));
			  }*/
			assert(!is_flag_set(flag));
		}
	}

	void CPU::jump_to(uint16_t new_pc){
		pending_cpu_increment = 0;
		if (CPU::extended_debug_data){
			fprintf(stdout, "Jump from 0x%04x to 0x%04x\n", registers.pc, new_pc);
		}
		registers.pc = new_pc;
	}

	void CPU::push_to_stack(uint16_t value){
		registers.sp -= 2;
		mmu.write_word(registers.sp, value);

		// Set the new PC
		if (CPU::extended_debug_data){
			fprintf(stdout, "Pushing 0x%04x to stack at 0x%04x\n", value, registers.sp);
		}
	}
	uint16_t CPU::pop_from_stack(void){
		uint16_t value = mmu.read_word(registers.sp);
		registers.sp += 2;

		// Set the new PC
		if (CPU::extended_debug_data){
			fprintf(stdout, "Popping 0x%04x from stack\n", value);
		}

		return value;
	}

	void CPU::step(){
		if (stopped) return;

		clock_cycles_this_step = 0;
	
		if (mmu.dma_timer < 0){
			auto interrupt_data = interrupts.next_interrupt();
			if (interrupt_data){
				interrupts.disable();
				if (allow_debug){
					fprintf(stderr, "Triggered Interrupt 0x%02x!\n", interrupt_data->flag_value);
				}
				push_to_stack(registers.pc);
				jump_to(interrupt_data->handler_pc);
				interrupts.flagged = interrupts.flagged & ~(interrupt_data->flag_value);
				interrupts.find_next_interrupt();
				clock_cycles_this_step += 12;
				halted = false;
			}
		}

		uint16_t old_pc = registers.pc;
		uint8_t instruction_index = mmu.read_byte(registers.pc);
		Instructions::Instruction* instruction;
		if (!halted){
			if (registers.pc == loop_check[0] || registers.pc == loop_check[1] || registers.pc == loop_check[2]){
				debug_data = allow_debug_during_loops;
				extended_debug_data = allow_debug_during_loops && allow_extended_debug;
				if (allow_debug && !debug_data){
					fprintf(stdout, "\rloop");
				}
			}else{
				debug_data = allow_debug && !waiting_for_ret;
				extended_debug_data = allow_extended_debug && debug_data;
			}
	
			loop_check[2] = loop_check[1];
			loop_check[1] = loop_check[0];
			loop_check[0] = registers.pc;

			registers.pc++;
	
			instruction = instruction_set.get_instruction(*this, instruction_index);
			if (debug_data && !within_bios){
				fprintf(stdout, "0x%04x: [0x%02x] %s\n", old_pc, instruction_index, instruction->disassembly);
			}

			clock_cycles_this_step += instruction->execute(*this);
		}else{
			clock_cycles_this_step = 1;
		}

		clock_cycles += clock_cycles_this_step;

		registers.f = registers.f & 0xF0;

		mmu.step();
		gpu.step();
		timer.step();
	
		if (within_bios && registers.pc >= MMU::BIOS_SIZE){
			fprintf(stdout, "Ran the BIOS successfully!\n");
			exit_bios();
		}

		/*if (registers.pc > MMU::GPU_VRAM_START && !manual_step_requested){
		  fprintf(stdout, "Execution left the ROM!\n");
		  manual_step_requested = true;
		  }*/

		/*if (registers.pc == 0x291){
		  manual_step_requested = true;
		  }
		  if (registers.pc == 0x282a){
		  manual_step_requested = true;
		  }
		  if (registers.pc == 0x2817){
		  manual_step_requested = true;
		  }*/

		/*if (registers.pc == 0xc360){
			manual_step_requested = true;
			}*/
		/*if (registers.pc == 0xC67F){
			manual_step_requested = true;
			waiting_for_ret = false;
		}
		if (registers.pc == 0xC682){
			manual_step_requested = true;
			waiting_for_ret = false;
		}*/
		if (registers.pc == 0x20b1){
			//manual_step_requested = true;
			//waiting_for_ret = false;
		}
		if (waiting_for_ret && (instruction_index == 0xC0 || instruction_index == 0xC8 || instruction_index == 0xC9 || instruction_index == 0xD0 || instruction_index == 0xD8 || instruction_index == 0xD9)){
			manual_step_requested = true;
			waiting_for_ret = false;
		}
		
		if (registers.pc == 0x38 && mmu.read_byte(registers.pc) == 0xff){
			fprintf(stdout, "Encountered infinite RST 0x38 loop!\n");
			stopped = true;
		}
		/*if (instruction_index == 0xD1 && registers.pc > 0xc090){
			manual_step_requested = true;
			}*/
	
		if (stopped){
			fprintf(stdout, "Execution was stopped with PC at 0x%04x (instruction at 0x%04x), instruction 0x%02x \"%s\".\n", registers.pc, old_pc, instruction_index, instruction->disassembly);
		}
	
		current_operand_size = 0;
	}
}
