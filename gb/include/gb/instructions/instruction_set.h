// Copyright Samuel Stark 2017

#pragma once

#include "instruction.h"

namespace GB{
	class CPU;

	namespace Instructions{
		class InstructionSet{
		public:
			InstructionSet();
			~InstructionSet();
	
			Instruction* get_instruction(CPU& cpu, uint8_t index);

			void print_all();
		protected:
			Instruction* unknown_instruction;
			Instruction* instructions[256] = { nullptr };
			Instruction* unknown_cb_instruction;
			Instruction* cb_instructions[256] = { nullptr };

			template<template <typename InType> typename InstructionType, int Index>
			void populate_cb_instruction_block(const char* const name);
			template<template <typename InType, int Bit> typename InstructionType, int Bit, int Index>
			void populate_cb_instruction_block(const char* const name);
		};
	}
}
