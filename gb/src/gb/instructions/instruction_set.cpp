// Copyright Samuel Stark 2017

#include "gb/cpu.h"
#include "gb/instructions/instruction_set.h"
#include "gb/instructions/all_instructions.h"
#include "gb/instructions/instruction_sources.h"

namespace GB::Instructions{

	uint8_t Instruction::execute(CPU& cpu){
		fprintf(stderr, "Tried to run an unknown instruction!\n");
		cpu.stopped = true;
		return 0;
	}

	Instruction* InstructionSet::get_instruction(CPU& cpu, uint8_t index){
		if (index == 0xCB){
			Instruction* instruction = cb_instructions[cpu.load_operand<uint8_t>()];
			if (instruction == nullptr){
				return unknown_cb_instruction;
			}
			return instruction;
		}
		Instruction* instruction = instructions[index];
		if (instruction == nullptr)
			return unknown_instruction;
		return instruction;
	}

	using namespace GB::Instructions::Sources;

	template<template <typename InType> typename InstructionType, int Index>
	void InstructionSet::populate_cb_instruction_block(const char* const name){
		cb_instructions[Index + 0] = new InstructionType<RegisterB>{name};
		cb_instructions[Index + 1] = new InstructionType<RegisterC>{name};
		cb_instructions[Index + 2] = new InstructionType<RegisterD>{name};
		cb_instructions[Index + 3] = new InstructionType<RegisterE>{name};
		cb_instructions[Index + 4] = new InstructionType<RegisterH>{name};
		cb_instructions[Index + 5] = new InstructionType<RegisterL>{name};
		cb_instructions[Index + 6] = new InstructionType<Pointer<uint8_t, RegisterHL>>{name};
		cb_instructions[Index + 7] = new InstructionType<RegisterA>{name};
	}
	template<template <typename InType, int Bit> typename InstructionType, int Bit, int Index>
	void InstructionSet::populate_cb_instruction_block(const char* const name){
		cb_instructions[Index + 0] = new InstructionType<RegisterB, Bit>{name};
		cb_instructions[Index + 1] = new InstructionType<RegisterC, Bit>{name};
		cb_instructions[Index + 2] = new InstructionType<RegisterD, Bit>{name};
		cb_instructions[Index + 3] = new InstructionType<RegisterE, Bit>{name};
		cb_instructions[Index + 4] = new InstructionType<RegisterH, Bit>{name};
		cb_instructions[Index + 5] = new InstructionType<RegisterL, Bit>{name};
		cb_instructions[Index + 6] = new InstructionType<Pointer<uint8_t, RegisterHL>, Bit>{name};
		cb_instructions[Index + 7] = new InstructionType<RegisterA, Bit>{name};
	}

	InstructionSet::InstructionSet(){
		unknown_instruction = new Instruction{"UNK"};
		unknown_cb_instruction = new CB::UnknownInstruction{"UNK CB"};
	
		/* 
		   0x00
		*/
		instructions[0x00] = new NoopInstruction{"NOOP"};
		instructions[0x01] = new LoadInstruction<RegisterBC,
												 Operand<uint16_t>>{"LD BC,d16"};
		instructions[0x02] = new LoadInstruction<Pointer<uint8_t, RegisterBC>,
												 RegisterA>{"LD (BC),A"};
		instructions[0x03] = new ALU::IncrementInstruction<uint16_t,
														   RegisterBC>{"INC BC"};
		instructions[0x04] = new ALU::IncrementInstruction<uint8_t,
														   RegisterB>{"INC B"};
		instructions[0x05] = new ALU::DecrementInstruction<uint8_t,
														   RegisterB>{"DEC B"};
		instructions[0x06] = new LoadInstruction<RegisterB,
												 Operand<uint8_t>>{"LD B,d8"};
		instructions[0x07] = new ALU::RotateInstruction<RegisterA,
														true,
														false>{"RLC A"};
		instructions[0x08] = new LoadInstruction<PointerFromOperand<uint16_t>,
												 RegisterSP>{"LD (a16),SP"};
		instructions[0x09] = new ALU::AddInstruction<uint16_t,
													 RegisterHL,
													 RegisterBC,
													 false>{"ADD HL,BC"};
		instructions[0x0A] = new LoadInstruction<RegisterA,
												 Pointer<uint8_t, RegisterBC>>{"LD A,(BC)"};
		instructions[0x0B] = new ALU::DecrementInstruction<uint16_t,
														   RegisterBC>{"DEC BC"};
		instructions[0x0C] = new ALU::IncrementInstruction<uint8_t,
														   RegisterC>{"INC C"};
		instructions[0x0D] = new ALU::DecrementInstruction<uint8_t,
														   RegisterC>{"DEC C"};
		instructions[0x0E] = new LoadInstruction<RegisterC,
												 Operand<uint8_t>>{"LD C,d8"};
		instructions[0x0F] = new ALU::RotateInstruction<RegisterA,
														false,
														false>{"RRC A"};

		/* 
		   0x10
		*/
		instructions[0x10] = new StopInstruction{"STOP"};
		instructions[0x11] = new LoadInstruction<CPURegister<uint16_t, &CPU::Registers::de>,
												 Operand<uint16_t>>{"LD DE,d16"};
		instructions[0x12] = new LoadInstruction<Pointer<uint8_t, CPURegister<uint16_t, &CPU::Registers::de>>,
												 RegisterA>{"LD (DE),A"};
		instructions[0x13] = new ALU::IncrementInstruction<uint16_t,
														   CPURegister<uint16_t, &CPU::Registers::de>>{"INC DE"};
		instructions[0x14] = new ALU::IncrementInstruction<uint8_t,
														   RegisterD>{"INC D"};
		instructions[0x15] = new ALU::DecrementInstruction<uint8_t,
														   RegisterD>{"DEC D"};
		instructions[0x16] = new LoadInstruction<RegisterD,
												 Operand<uint8_t>>{"LD D,d8"};
		instructions[0x17] = new ALU::RotateInstruction<RegisterA,
														true,
														true>{"RL A"};
		instructions[0x18] = new JumpInstruction<JumpCondition::Always,
												 JumpMode::SignedOffset,
												 Operand<uint8_t>>{"JR r8"};
		instructions[0x19] = new ALU::AddInstruction<uint16_t,
													 RegisterHL,
													 CPURegister<uint16_t, &CPU::Registers::de>,
													 false>{"ADD HL,DE"};
		instructions[0x1A] = new LoadInstruction<RegisterA,
												 Pointer<uint8_t, CPURegister<uint16_t, &CPU::Registers::de>>>{"LD A,(DE)"};
		instructions[0x1B] = new ALU::DecrementInstruction<uint16_t,
														   CPURegister<uint16_t, &CPU::Registers::de>>{"DEC DE"};
		instructions[0x1C] = new ALU::IncrementInstruction<uint8_t,
														   RegisterE>{"INC E"};
		instructions[0x1D] = new ALU::DecrementInstruction<uint8_t,
														   RegisterE>{"DEC E"};
		instructions[0x1E] = new LoadInstruction<RegisterE,
												 Operand<uint8_t>>{"LD E,d8"};
		instructions[0x1F] = new ALU::RotateInstruction<RegisterA,
														false,
														true>{"RR A"};

		/* 
		   0x20
		*/
		instructions[0x20] = new JumpInstruction<JumpCondition::NotZero,
												 JumpMode::SignedOffset,
												 Operand<uint8_t>>{"JR NZ,r8"};
		instructions[0x21] = new LoadInstruction<RegisterHL,
												 Operand<uint16_t>>{"LD HL,d16"};
		instructions[0x22] = new LoadInstruction<Pointer<uint8_t, IncrementOnLoad<uint16_t, RegisterHL>>,
												 RegisterA>{"LD (HL+),A"};
		instructions[0x23] = new ALU::IncrementInstruction<uint16_t,
														   RegisterHL>{"INC HL"};
		instructions[0x24] = new ALU::IncrementInstruction<uint8_t,
														   RegisterH>{"INC H"};
		instructions[0x25] = new ALU::DecrementInstruction<uint8_t,
														   RegisterH>{"DEC H"};
		instructions[0x26] = new LoadInstruction<RegisterH,
												 Operand<uint8_t>>{"LD H,d8"};
		instructions[0x27] = new BCDCorrectInstruction<RegisterA>{"DAA"};
		instructions[0x28] = new JumpInstruction<JumpCondition::Zero,
												 JumpMode::SignedOffset,
												 Operand<uint8_t>>{"JR Z,r8"};
		instructions[0x29] = new ALU::AddInstruction<uint16_t,
													 RegisterHL,
													 RegisterHL,
													 false>{"ADD HL,HL"};
		instructions[0x2A] = new LoadInstruction<RegisterA,
												 Pointer<uint8_t, IncrementOnLoad<uint16_t, RegisterHL>>>{"LD A,(HL+)"};
		instructions[0x2B] = new ALU::DecrementInstruction<uint16_t,
														   RegisterHL>{"DEC HL"};
		instructions[0x2C] = new ALU::IncrementInstruction<uint8_t,
														   RegisterL>{"INC L"};
		instructions[0x2D] = new ALU::DecrementInstruction<uint8_t,
														   RegisterL>{"DEC L"};
		instructions[0x2E] = new LoadInstruction<RegisterL,
												 Operand<uint8_t>>{"LD L,d8"};
		instructions[0x2F] = new ALU::NotInstruction<RegisterA,
													 RegisterA>{"CPL"};

		/* 
		   0x30
		*/
		instructions[0x30] = new JumpInstruction<JumpCondition::NotCarry,
												 JumpMode::SignedOffset,
												 Operand<uint8_t>>{"JR NC,r8"};
		instructions[0x31] = new LoadInstruction<RegisterSP,
												 Operand<uint16_t>>{"LD SP,d16"};
		instructions[0x32] = new LoadInstruction<Pointer<uint8_t, DecrementOnLoad<uint16_t, RegisterHL>>,
												 RegisterA>{"LD (HL-),A"};
		instructions[0x33] = new ALU::IncrementInstruction<uint16_t,
														   RegisterSP>{"INC SP"};
		instructions[0x34] = new ALU::IncrementInstruction<uint8_t,
														   Pointer<uint8_t, RegisterHL>>{"INC (HL)"};
		instructions[0x35] = new ALU::DecrementInstruction<uint8_t,
														   Pointer<uint8_t, RegisterHL>>{"DEC (HL)"};
		instructions[0x36] = new LoadInstruction<Pointer<uint8_t, RegisterHL>,
												 Operand<uint8_t>>{"LD (HL),d8"};
		instructions[0x37] = new SetCarryFlagInstruction<true>{"SCF"};
		instructions[0x38] = new JumpInstruction<JumpCondition::Carry,
												 JumpMode::SignedOffset,
												 Operand<uint8_t>>{"JR C,r8"};
		instructions[0x39] = new ALU::AddInstruction<uint16_t,
													 RegisterHL,
													 RegisterSP,
													 false>{"ADD HL,SP"};
		instructions[0x3A] = new LoadInstruction<RegisterA,
												 Pointer<uint8_t, DecrementOnLoad<uint16_t, RegisterHL>>>{"LD A,(HL-)"};
		instructions[0x3B] = new ALU::DecrementInstruction<uint16_t,
														   RegisterSP>{"DEC SP"};
		instructions[0x3C] = new ALU::IncrementInstruction<uint8_t,
														   RegisterA>{"INC A"};
		instructions[0x3D] = new ALU::DecrementInstruction<uint8_t,
														   RegisterA>{"DEC A"};
		instructions[0x3E] = new LoadInstruction<RegisterA,
												 Operand<uint8_t>>{"LD A,d8"};
		instructions[0x3F] = new ComplementCarryFlagInstruction{"CCF"};
	
		/* 
		   0x40
		*/
		// LD to B
		instructions[0x40] = new NoopInstruction{"LD B,B"};
		instructions[0x41] = new LoadInstruction<RegisterB,
												 RegisterC>{"LD B,C"};
		instructions[0x42] = new LoadInstruction<RegisterB,
												 RegisterD>{"LD B,D"};
		instructions[0x43] = new LoadInstruction<RegisterB,
												 RegisterE>{"LD B,E"};
		instructions[0x44] = new LoadInstruction<RegisterB,
												 RegisterH>{"LD B,H"};
		instructions[0x45] = new LoadInstruction<RegisterB,
												 RegisterL>{"LD B,L"};
		instructions[0x46] = new LoadInstruction<RegisterB,
												 Pointer<uint8_t, RegisterHL>>{"LD B,(HL)"};
		instructions[0x47] = new LoadInstruction<RegisterB,
												 RegisterA>{"LD B,A"};
		// LD to C
		instructions[0x48] = new LoadInstruction<RegisterC,
												 RegisterB>{"LD C,B"};
		instructions[0x49] = new NoopInstruction{"LD C,C"};
		instructions[0x4A] = new LoadInstruction<RegisterC,
												 RegisterD>{"LD C,D"};
		instructions[0x4B] = new LoadInstruction<RegisterC,
												 RegisterE>{"LD C,E"};
		instructions[0x4C] = new LoadInstruction<RegisterC,
												 RegisterH>{"LD C,H"};
		instructions[0x4D] = new LoadInstruction<RegisterC,
												 RegisterL>{"LD C,L"};
		instructions[0x4E] = new LoadInstruction<RegisterC,
												 Pointer<uint8_t, RegisterHL>>{"LD C,(HL)"};
		instructions[0x4F] = new LoadInstruction<RegisterC,
												 RegisterA>{"LD C,A"};

		/* 
		   0x50
		*/
		// LD to D
		instructions[0x50] = new LoadInstruction<RegisterD,
												 RegisterB>{"LD D,B"};
		instructions[0x51] = new LoadInstruction<RegisterD,
												 RegisterC>{"LD D,C"};
		instructions[0x52] = new NoopInstruction{"LD D,D"};
		instructions[0x53] = new LoadInstruction<RegisterD,
												 RegisterE>{"LD D,E"};
		instructions[0x54] = new LoadInstruction<RegisterD,
												 RegisterH>{"LD D,H"};
		instructions[0x55] = new LoadInstruction<RegisterD,
												 RegisterL>{"LD D,L"};
		instructions[0x56] = new LoadInstruction<RegisterD,
												 Pointer<uint8_t, RegisterHL>>{"LD D,(HL)"};
		instructions[0x57] = new LoadInstruction<RegisterD,
												 RegisterA>{"LD D,A"};

		// LD to E
		instructions[0x58] = new LoadInstruction<RegisterE,
												 RegisterB>{"LD E,B"};
		instructions[0x59] = new LoadInstruction<RegisterE,
												 RegisterC>{"LD E,C"};
		instructions[0x5A] = new LoadInstruction<RegisterE,
												 RegisterD>{"LD E,D"};
		instructions[0x5B] = new NoopInstruction{"LD E,E"};
		instructions[0x5C] = new LoadInstruction<RegisterE,
												 RegisterH>{"LD E,H"};
		instructions[0x5D] = new LoadInstruction<RegisterE,
												 RegisterL>{"LD E,L"};
		instructions[0x5E] = new LoadInstruction<RegisterE,
												 Pointer<uint8_t, RegisterHL>>{"LD E,(HL)"};
		instructions[0x5F] = new LoadInstruction<RegisterE,
												 RegisterA>{"LD E,A"};

		/* 
		   0x60
		*/
		// LD to H
		instructions[0x60] = new LoadInstruction<RegisterH,
												 RegisterB>{"LD H,B"};
		instructions[0x61] = new LoadInstruction<RegisterH,
												 RegisterC>{"LD H,C"};
		instructions[0x62] = new LoadInstruction<RegisterH,
												 RegisterD>{"LD H,D"};
		instructions[0x63] = new LoadInstruction<RegisterH,
												 RegisterE>{"LD H,E"};
		instructions[0x64] = new NoopInstruction{"LD H,H"};
		instructions[0x65] = new LoadInstruction<RegisterH,
												 RegisterL>{"LD H,L"};
		instructions[0x66] = new LoadInstruction<RegisterH,
												 Pointer<uint8_t, RegisterHL>>{"LD D,(HL)"};
		instructions[0x67] = new LoadInstruction<RegisterH,
												 RegisterA>{"LD H,A"};

		// LD to L
		instructions[0x68] = new LoadInstruction<RegisterL,
												 RegisterB>{"LD L,B"};
		instructions[0x69] = new LoadInstruction<RegisterL,
												 RegisterC>{"LD L,C"};
		instructions[0x6A] = new LoadInstruction<RegisterL,
												 RegisterD>{"LD L,D"};
		instructions[0x6B] = new LoadInstruction<RegisterL,
												 RegisterE>{"LD L,E"};
		instructions[0x6C] = new LoadInstruction<RegisterL,
												 RegisterH>{"LD L,H"};
		instructions[0x6D] = new NoopInstruction{"LD L,L"};
		instructions[0x6E] = new LoadInstruction<RegisterL,
												 Pointer<uint8_t, RegisterHL>>{"LD L,(HL)"};
		instructions[0x6F] = new LoadInstruction<RegisterL,
												 RegisterA>{"LD L,A"};

		/* 
		   0x70
		*/
		// LD to (HL), 0x76 is HALT
		instructions[0x70] = new LoadInstruction<Pointer<uint8_t, RegisterHL>,
												 RegisterB>{"LD (HL),B"};
		instructions[0x71] = new LoadInstruction<Pointer<uint8_t, RegisterHL>,
												 RegisterC>{"LD (HL),C"};
		instructions[0x72] = new LoadInstruction<Pointer<uint8_t, RegisterHL>,
												 RegisterD>{"LD (HL),D"};
		instructions[0x73] = new LoadInstruction<Pointer<uint8_t, RegisterHL>,
												 RegisterE>{"LD (HL),E"};
		instructions[0x74] = new LoadInstruction<Pointer<uint8_t, RegisterHL>,
												 RegisterH>{"LD (HL),H"};
		instructions[0x75] = new LoadInstruction<Pointer<uint8_t, RegisterHL>,
												 RegisterL>{"LD (HL),L"};
		instructions[0x76] = new HaltInstruction{"HALT"}; // TODO: Halting stops until an interrups happens (regardless of whether the interrupt master is true or not). implement this before using this instruction
		instructions[0x77] = new LoadInstruction<Pointer<uint8_t, RegisterHL>,
												 RegisterA>{"LD (HL),A"};
		// LD to A
		instructions[0x78] = new LoadInstruction<RegisterA,
												 RegisterB>{"LD A,B"};
		instructions[0x79] = new LoadInstruction<RegisterA,
												 RegisterC>{"LD A,C"};
		instructions[0x7A] = new LoadInstruction<RegisterA,
												 RegisterD>{"LD A,D"};
		instructions[0x7B] = new LoadInstruction<RegisterA,
												 RegisterE>{"LD A,E"};
		instructions[0x7C] = new LoadInstruction<RegisterA,
												 RegisterH>{"LD A,H"};
		instructions[0x7D] = new LoadInstruction<RegisterA,
												 RegisterL>{"LD A,L"};
		instructions[0x7E] = new LoadInstruction<RegisterA,
												 Pointer<uint8_t, RegisterHL>>{"LD A,(HL)"};
		instructions[0x7F] = new NoopInstruction{"LD A,A"};

		/* 
		   0x80
		*/
		// ADD to A (Add without Carry)
		instructions[0x80] = new ALU::AddInstruction<uint8_t,
													 RegisterA,
													 RegisterB,
													 false>{"ADD A,B"};
		instructions[0x81] = new ALU::AddInstruction<uint8_t,
													 RegisterA,
													 RegisterC,
													 false>{"ADD A,C"};
		instructions[0x82] = new ALU::AddInstruction<uint8_t,
													 RegisterA,
													 RegisterD,
													 false>{"ADD A,D"};
		instructions[0x83] = new ALU::AddInstruction<uint8_t,
													 RegisterA,
													 RegisterE,
													 false>{"ADD A,E"};
		instructions[0x84] = new ALU::AddInstruction<uint8_t,
													 RegisterA,
													 RegisterH,
													 false>{"ADD A,H"};
		instructions[0x85] = new ALU::AddInstruction<uint8_t,
													 RegisterA,
													 RegisterL,
													 false>{"ADD A,L"};
		instructions[0x86] = new ALU::AddInstruction<uint8_t,
													 RegisterA,
													 Pointer<uint8_t, RegisterHL>,
													 false>{"ADD A,(HL)"};
		instructions[0x87] = new ALU::AddInstruction<uint8_t,
													 RegisterA,
													 RegisterA,
													 false>{"ADD A,A"};
		// ADC to A (Add with Carry)
		instructions[0x88] = new ALU::AddInstruction<uint8_t,
													 RegisterA,
													 RegisterB,
													 true>{"ADC A,B"};
		instructions[0x89] = new ALU::AddInstruction<uint8_t,
													 RegisterA,
													 RegisterC,
													 true>{"ADC A,C"};
		instructions[0x8A] = new ALU::AddInstruction<uint8_t,
													 RegisterA,
													 RegisterD,
													 true>{"ADC A,D"};
		instructions[0x8B] = new ALU::AddInstruction<uint8_t,
													 RegisterA,
													 RegisterE,
													 true>{"ADC A,E"};
		instructions[0x8C] = new ALU::AddInstruction<uint8_t,
													 RegisterA,
													 RegisterH,
													 true>{"ADC A,H"};
		instructions[0x8D] = new ALU::AddInstruction<uint8_t,
													 RegisterA,
													 RegisterL,
													 true>{"ADC A,L"};
		instructions[0x8E] = new ALU::AddInstruction<uint8_t,
													 RegisterA,
													 Pointer<uint8_t, RegisterHL>,
													 true>{"ADC A,(HL)"};
		instructions[0x8F] = new ALU::AddInstruction<uint8_t,
													 RegisterA,
													 RegisterA,
													 true>{"ADC A,A"};
	
		/* 
		   0x90
		*/
		// SUB from A (Subtract without Carry)
		instructions[0x90] = new ALU::SubInstruction<RegisterA,
													 RegisterB,
													 false>{"SUB A,B"};
		instructions[0x91] = new ALU::SubInstruction<RegisterA,
													 RegisterC,
													 false>{"SUB A,C"};
		instructions[0x92] = new ALU::SubInstruction<RegisterA,
													 RegisterD,
													 false>{"SUB A,D"};
		instructions[0x93] = new ALU::SubInstruction<RegisterA,
													 RegisterE,
													 false>{"SUB A,E"};
		instructions[0x94] = new ALU::SubInstruction<RegisterA,
													 RegisterH,
													 false>{"SUB A,H"};
		instructions[0x95] = new ALU::SubInstruction<RegisterA,
													 RegisterL,
													 false>{"SUB A,L"};
		instructions[0x96] = new ALU::SubInstruction<RegisterA,
													 Pointer<uint8_t, RegisterHL>,
													 false>{"SUB A,(HL)"};
		instructions[0x97] = new ALU::SubInstruction<RegisterA,
													 RegisterA,
													 false>{"SUB A,A"};
		// SBC from A (Subtract with Carry)
		instructions[0x98] = new ALU::SubInstruction<RegisterA,
													 RegisterB,
													 true>{"SBC A,B"};
		instructions[0x99] = new ALU::SubInstruction<RegisterA,
													 RegisterC,
													 true>{"SBC A,C"};
		instructions[0x9A] = new ALU::SubInstruction<RegisterA,
													 RegisterD,
													 true>{"SBC A,D"};
		instructions[0x9B] = new ALU::SubInstruction<RegisterA,
													 RegisterE,
													 true>{"SBC A,E"};
		instructions[0x9C] = new ALU::SubInstruction<RegisterA,
													 RegisterH,
													 true>{"SBC A,H"};
		instructions[0x9D] = new ALU::SubInstruction<RegisterA,
													 RegisterL,
													 true>{"SBC A,L"};
		instructions[0x9E] = new ALU::SubInstruction<RegisterA,
													 Pointer<uint8_t, RegisterHL>,
													 true>{"SBC A,(HL)"};
		instructions[0x9F] = new ALU::SubInstruction<RegisterA,
													 RegisterA,
													 true>{"SBC A,A"};

		/* 
		   0xA0
		*/
		// AND with A
		instructions[0xA0] = new ALU::AndInstruction<RegisterA,
													 RegisterB>{"AND B"};
		instructions[0xA1] = new ALU::AndInstruction<RegisterA,
													 RegisterC>{"AND C"};
		instructions[0xA2] = new ALU::AndInstruction<RegisterA,
													 RegisterD>{"AND D"};
		instructions[0xA3] = new ALU::AndInstruction<RegisterA,
													 RegisterE>{"AND E"};
		instructions[0xA4] = new ALU::AndInstruction<RegisterA,
													 RegisterH>{"AND H"};
		instructions[0xA5] = new ALU::AndInstruction<RegisterA,
													 RegisterL>{"AND L"};
		instructions[0xA6] = new ALU::AndInstruction<RegisterA,
													 Pointer<uint8_t, RegisterHL>>{"AND (HL)"};
		instructions[0xA7] = new ALU::AndInstruction<RegisterA,
													 RegisterA>{"AND A"}; // TODO: This could be no-op?
		// XOR with A
		instructions[0xA8] = new ALU::XorInstruction<RegisterA,
													 RegisterB>{"XOR B"};
		instructions[0xA9] = new ALU::XorInstruction<RegisterA,
													 RegisterC>{"XOR C"};
		instructions[0xAA] = new ALU::XorInstruction<RegisterA,
													 RegisterD>{"XOR D"};
		instructions[0xAB] = new ALU::XorInstruction<RegisterA,
													 RegisterE>{"XOR E"};
		instructions[0xAC] = new ALU::XorInstruction<RegisterA,
													 RegisterH>{"XOR H"};
		instructions[0xAD] = new ALU::XorInstruction<RegisterA,
													 RegisterL>{"XOR L"};
		instructions[0xAE] = new ALU::XorInstruction<RegisterA,
													 Pointer<uint8_t, RegisterHL>>{"XOR (HL)"};
		instructions[0xAF] = new ALU::XorInstruction<RegisterA,
													 RegisterA>{"XOR A"};

		/* 
		   0xB0
		*/
		// OR with A
		instructions[0xB0] = new ALU::OrInstruction<RegisterA,
													RegisterB>{"OR B"};
		instructions[0xB1] = new ALU::OrInstruction<RegisterA,
													RegisterC>{"OR C"};
		instructions[0xB2] = new ALU::OrInstruction<RegisterA,
													RegisterD>{"OR D"};
		instructions[0xB3] = new ALU::OrInstruction<RegisterA,
													RegisterE>{"OR E"};
		instructions[0xB4] = new ALU::OrInstruction<RegisterA,
													RegisterH>{"OR H"};
		instructions[0xB5] = new ALU::OrInstruction<RegisterA,
													RegisterL>{"OR L"};
		instructions[0xB6] = new ALU::OrInstruction<RegisterA,
													Pointer<uint8_t, RegisterHL>>{"OR (HL)"};
		instructions[0xB7] = new ALU::OrInstruction<RegisterA,
													RegisterA>{"OR A"}; // TODO: This could be no-op?
		// CP with A (Compare)
		instructions[0xB8] = new ALU::CompareInstruction<RegisterA,
														 RegisterB>{"CP B"};
		instructions[0xB9] = new ALU::CompareInstruction<RegisterA,
														 RegisterC>{"CP C"};
		instructions[0xBA] = new ALU::CompareInstruction<RegisterA,
														 RegisterD>{"CP D"};
		instructions[0xBB] = new ALU::CompareInstruction<RegisterA,
														 RegisterE>{"CP E"};
		instructions[0xBC] = new ALU::CompareInstruction<RegisterA,
														 RegisterH>{"CP H"};
		instructions[0xBD] = new ALU::CompareInstruction<RegisterA,
														 RegisterL>{"CP L"};
		instructions[0xBE] = new ALU::CompareInstruction<RegisterA,
														 Pointer<uint8_t, RegisterHL>>{"CP (HL)"};
		instructions[0xBF] = new ALU::CompareInstruction<RegisterA,
														 RegisterA>{"CP A"};

		/*
		  0xC0
		*/
		instructions[0xC0] = new ReturnInstruction<JumpCondition::NotZero>{"RET NZ"};
		instructions[0xC1] = new PopStackInstruction<RegisterBC>{"POP BC"};
		instructions[0xC2] = new JumpInstruction<JumpCondition::NotZero,
												 JumpMode::AbsoluteValue,
												 Operand<uint16_t>>{"JP NZ,a16"};
		instructions[0xC3] = new JumpInstruction<JumpCondition::Always,
												 JumpMode::AbsoluteValue,
												 Operand<uint16_t>>{"JP a16"};
		instructions[0xC4] = new CallInstruction<Operand<uint16_t>, JumpCondition::NotZero>{"CALL NZ, a16"};
		instructions[0xC5] = new PushStackInstruction<RegisterBC>{"PUSH BC"};
		instructions[0xC6] = new ALU::AddInstruction<uint8_t,
													 RegisterA,
													 Operand<uint8_t>,
													 false>{"ADD A,d8"};
		instructions[0xC7] = new CallRoutineInstruction<0x00>{"RST 0"};
		instructions[0xC8] = new ReturnInstruction<JumpCondition::Zero>{"RET Z"};
		instructions[0xC9] = new ReturnInstruction<JumpCondition::Always>{"RET"};
		instructions[0xCA] = new JumpInstruction<JumpCondition::Zero,
												 JumpMode::AbsoluteValue,
												 Operand<uint16_t>>{"JP Z,a16"};
		instructions[0xCB] = nullptr; // This is done later
		instructions[0xCC] = new CallInstruction<Operand<uint16_t>, JumpCondition::Zero>{"CALL Z, a16"};
		instructions[0xCD] = new CallInstruction<Operand<uint16_t>>{"CALL NN"};
		instructions[0xCE] = new ALU::AddInstruction<uint8_t,
													 RegisterA,
													 Operand<uint8_t>,
													 true>{"ADC A,d8"};
		instructions[0xCF] = new CallRoutineInstruction<0x08>{"RST 8"};


		/*
		  0xD0
		*/
		instructions[0xD0] = new ReturnInstruction<JumpCondition::NotCarry>{"RET NC"};
		instructions[0xD1] = new PopStackInstruction<CPURegister<uint16_t, &CPU::Registers::de>>{"POP DE"};
		instructions[0xD2] = new JumpInstruction<JumpCondition::NotCarry,
												 JumpMode::AbsoluteValue,
												 Operand<uint16_t>>{"JP NC,a16"};
		instructions[0xD3] = nullptr;
		instructions[0xD4] = new CallInstruction<Operand<uint16_t>, JumpCondition::NotCarry>{"CALL NC, a16"};
		instructions[0xD5] = new PushStackInstruction<CPURegister<uint16_t, &CPU::Registers::de>>{"PUSH DE"};
		instructions[0xD6] = new ALU::SubInstruction<RegisterA,
													 Operand<uint8_t>,
													 false>{"SUB A,d8"};
		instructions[0xD7] = new CallRoutineInstruction<0x10>{"RST 10"};
		instructions[0xD8] = new ReturnInstruction<JumpCondition::Carry>{"RET C"};
		instructions[0xD9] = new ReturnInterruptInstruction{"RETI"};
		instructions[0xDA] = new JumpInstruction<JumpCondition::Carry,
												 JumpMode::AbsoluteValue,
												 Operand<uint16_t>>{"JP C,a16"};
		instructions[0xDB] = nullptr;
		instructions[0xDC] = new CallInstruction<Operand<uint16_t>, JumpCondition::Carry>{"CALL C, a16"};
		instructions[0xDD] = nullptr;
		instructions[0xDE] = new ALU::SubInstruction<RegisterA,
													 Operand<uint8_t>,
													 true>{"SBC A,d8"};
		instructions[0xDF] = new CallRoutineInstruction<0x18>{"RST 18"};


		/*
		  0xE0
		*/
		instructions[0xE0] = new LoadInstruction<PointerFromOffsetFF00<Operand<uint8_t>>,
												 RegisterA>{"LDH (n),A"};
		instructions[0xE1] = new PopStackInstruction<RegisterHL>{"POP HL"};
		instructions[0xE2] = new LoadInstruction<PointerFromOffsetFF00<RegisterC>,
												 RegisterA>{"LDH (C),A"};
		instructions[0xE3] = nullptr;
		instructions[0xE4] = nullptr;
		instructions[0xE5] = new PushStackInstruction<RegisterHL>{"PUSH HL"};
		instructions[0xE6] = new ALU::AndInstruction<RegisterA,
													 Operand<uint8_t>>{"AND d8"};
		instructions[0xE7] = new CallRoutineInstruction<0x20>{"RST 20"};
		instructions[0xE8] = new AddSigned8BitImmediateToSPInstruction{"ADD SP,r8"};
		instructions[0xE9] = new JumpInstruction<JumpCondition::Always,
												 JumpMode::AbsoluteValue,
												 RegisterHL>{"JP (HL)"}; // TODO: This doesn't seem right at all, but it works for Tetris. Coincidence?
		//WordPointer<RegisterHL>>{"JP (HL)"};
		instructions[0xEA] = new LoadInstruction<PointerFromOperand<uint8_t>,
												 RegisterA>{"LD (nn),A"};
		instructions[0xEB] = nullptr;
		instructions[0xEC] = nullptr;
		instructions[0xED] = nullptr;
		instructions[0xEE] = new ALU::XorInstruction<RegisterA,
													 Operand<uint8_t>>{"XOR n"};
		instructions[0xEF] = new CallRoutineInstruction<0x28>{"RST 28"};

		/*
		  0xF0
		*/
		instructions[0xF0] = new LoadInstruction<RegisterA,
												 PointerFromOffsetFF00<Operand<uint8_t>>>{"LDH A,(n)"};
		instructions[0xF1] = new PopStackInstruction<CPURegister<uint16_t, &CPU::Registers::af>>{"POP AF"};
		instructions[0xF2] = new LoadInstruction<RegisterA,
												 PointerFromOffsetFF00<RegisterC>>{"LDH A,(C)"};
		instructions[0xF3] = new SetInterruptsEnabledInstruction<false>{"DI"};
		instructions[0xF4] = nullptr;
		instructions[0xF5] = new PushStackInstruction<CPURegister<uint16_t, &CPU::Registers::af>>{"PUSH AF"};
		instructions[0xF6] = new ALU::OrInstruction<RegisterA,
													Operand<uint8_t>>{"OR d8"};
		instructions[0xF7] = new CallRoutineInstruction<0x30>{"RST 30"};
		instructions[0xF8] = new LDHLInstruction<RegisterHL,
												 RegisterSP,
												 Operand<uint8_t>>{"LD HL, SP + r8"};
		instructions[0xF9] = new LoadInstruction<RegisterSP,
												 RegisterHL>{"LD SP,HL"};
		instructions[0xFA] = new LoadInstruction<RegisterA,
												 PointerFromOperand<uint8_t>>{"LD A,(nn)"};
		instructions[0xFB] = new SetInterruptsEnabledInstruction<true>{"EI"};
		instructions[0xFC] = nullptr;
		instructions[0xFD] = nullptr;
		instructions[0xFE] = new ALU::CompareInstruction<RegisterA,
														 Operand<uint8_t>>{"CP n"};
		instructions[0xFF] = new CallRoutineInstruction<0x38>{"RST 38"};

		/*
		  0xCB
		*/
		populate_cb_instruction_block<CB::RLC, 0x00>("RLC");
		populate_cb_instruction_block<CB::RRC, 0x08>("RRC");
		populate_cb_instruction_block<CB::RL, 0x10>("RL");
		populate_cb_instruction_block<CB::RR, 0x18>("RR");

		populate_cb_instruction_block<CB::SLA, 0x20>("SLA");
		populate_cb_instruction_block<CB::SRA, 0x28>("SRA");
		populate_cb_instruction_block<CB::SWAP, 0x30>("SWAP");
		populate_cb_instruction_block<CB::SRL, 0x38>("SRL");

		populate_cb_instruction_block<CB::BIT, 0, 0x40>("BIT");
		populate_cb_instruction_block<CB::BIT, 1, 0x48>("BIT");
		populate_cb_instruction_block<CB::BIT, 2, 0x50>("BIT");
		populate_cb_instruction_block<CB::BIT, 3, 0x58>("BIT");
		populate_cb_instruction_block<CB::BIT, 4, 0x60>("BIT");
		populate_cb_instruction_block<CB::BIT, 5, 0x68>("BIT");
		populate_cb_instruction_block<CB::BIT, 6, 0x70>("BIT");
		populate_cb_instruction_block<CB::BIT, 7, 0x78>("BIT");

		populate_cb_instruction_block<CB::RES, 0, 0x80>("RES");
		populate_cb_instruction_block<CB::RES, 1, 0x88>("RES");
		populate_cb_instruction_block<CB::RES, 2, 0x90>("RES");
		populate_cb_instruction_block<CB::RES, 3, 0x98>("RES");
		populate_cb_instruction_block<CB::RES, 4, 0xA0>("RES");
		populate_cb_instruction_block<CB::RES, 5, 0xA8>("RES");
		populate_cb_instruction_block<CB::RES, 6, 0xB0>("RES");
		populate_cb_instruction_block<CB::RES, 7, 0xB8>("RES");

		populate_cb_instruction_block<CB::SET, 0, 0xC0>("SET");
		populate_cb_instruction_block<CB::SET, 1, 0xC8>("SET");
		populate_cb_instruction_block<CB::SET, 2, 0xD0>("SET");
		populate_cb_instruction_block<CB::SET, 3, 0xD8>("SET");
		populate_cb_instruction_block<CB::SET, 4, 0xE0>("SET");
		populate_cb_instruction_block<CB::SET, 5, 0xE8>("SET");
		populate_cb_instruction_block<CB::SET, 6, 0xF0>("SET");
		populate_cb_instruction_block<CB::SET, 7, 0xF8>("SET");

		assert(instructions[0xD3] == nullptr);
		assert(instructions[0xDB] == nullptr);
		assert(instructions[0xDD] == nullptr);
		assert(instructions[0xE3] == nullptr);
		assert(instructions[0xE4] == nullptr);
		assert(instructions[0xEB] == nullptr);
		assert(instructions[0xEC] == nullptr);
		assert(instructions[0xED] == nullptr);
		assert(instructions[0xF4] == nullptr);
		assert(instructions[0xFC] == nullptr);
		assert(instructions[0xFD] == nullptr);

		int current_normal_instructions = 1; // Count CB first, because it will be nullptr
		const int max_normal_instructions = 256 - 11; // 0-0xFF is 256, -11 undefined instructions
		for (auto instruction : instructions){
			if (instruction != nullptr)
				current_normal_instructions++;
		}
		int current_cb_instructions = 0;
		const int max_cb_instructions = 256;
		for (auto instruction : cb_instructions){
			if (instruction != nullptr)
				current_cb_instructions++;
		}
		fprintf(stdout, "%d/%d normal instructions defined, %d/%d CB instructions. Total Coverage: %3.2f%%\n", current_normal_instructions, max_normal_instructions, current_cb_instructions, max_cb_instructions, (current_normal_instructions + current_cb_instructions) * 100.0f/(max_normal_instructions + max_cb_instructions));
	}

	InstructionSet::~InstructionSet(){
		delete unknown_instruction;
		for (auto instruction : instructions){
			if (instruction != nullptr)
				delete instruction;
		}
		for (auto instruction : cb_instructions){
			if (instruction != nullptr)
				delete instruction;
		}
	}

	void InstructionSet::print_all(){
		for (int i = 0x00; i <= 0xFF; i++){
			Instruction* instruction = instructions[i];
			if (instruction == nullptr) instruction = unknown_instruction;
			fprintf(stdout, "0x%02x: %s\n", i, instruction->disassembly);
		}
	}
}
