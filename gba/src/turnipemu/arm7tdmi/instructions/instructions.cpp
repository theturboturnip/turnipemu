#include "turnipemu/arm7tdmi/cpu.h"

#include <sstream>

#include "turnipemu/utils.h"

#include "common.inl"
#include "arm/all.inl"
//#include "thumb/all.inl"

namespace TurnipEmu::ARM7TDMI{
	bool CPU::instructionsAreSetup = false;
	std::vector<std::unique_ptr<const ARM::InstructionCategory>> CPU::armInstructions;
	std::vector<std::unique_ptr<const Thumb::InstructionCategory>> CPU::thumbInstructions;
	
	void CPU::setupInstructions(){
		if (instructionsAreSetup) return;
		
		armInstructions = std::vector<std::unique_ptr<const ARM::InstructionCategory>>();
		armInstructions.push_back(std::make_unique<ARM::InstructionCategory>(
								   "Software Interrupt",
								   Mask<word>{
									   {27, 24, 0b1111}
								   }));
		armInstructions.push_back(std::make_unique<ARM::InstructionCategory>(
								   "Undefined",
								   Mask<word>{
									   {27, 25, 0b011},
									   {4, 1}
								   }));
		armInstructions.push_back(std::make_unique<ARM::BranchExchangeInstruction>(
								   "Branch and Exchange",
								   Mask<word>{
									   {27, 4, 0b0'0001'0010'1111'1111'1111'0001}
								   }));
		armInstructions.push_back(std::make_unique<ARM::BranchInstruction>(
								   "Branch",
								   Mask<word>{
									   {27, 25, 0b101}
								   }));
		armInstructions.push_back(std::make_unique<ARM::MRSInstruction>(
								   "MRS (Transfer PSR to Register)",
								   Mask<word>{
									   {27, 26, 0b00},
									   {24, 23, 0b10},
									   {21, 16, 0b001111},
									   {11, 0, 0}
								   }));
		armInstructions.push_back(std::make_unique<ARM::MSRInstruction>(
								   "MSR (Transfer Register to PSR)",
								   Mask<word>{
									   {27, 26, 0b00},
									   {24, 23, 0b10},
									   {21, 17, 0b10100},
									   {15, 12, 0b1111}
								   }));
		armInstructions.push_back(std::make_unique<ARM::InstructionCategory>(
								   "Multiply",
								   Mask<word>{
									   {27, 23, 0b00000},
									   {7, 4, 0b1001}
								   }));
		armInstructions.push_back(std::make_unique<ARM::InstructionCategory>(
								   "Single Data Swap",
								   Mask<word>{
									   {27, 23, 0b00010},
									   {21, 20, 0b00},
									   {11, 4, 0b0'0000'1001}
								   }));
		armInstructions.push_back(std::make_unique<ARM::InstructionCategory>(
								   "Halfword Data Transfer",
								   Mask<word>{
									   {27, 25, 0b000},
									   {7, 1},
									   {4, 1}
								   }));
		armInstructions.push_back(std::make_unique<ARM::DataProcessingInstruction>(
								   "Data Processing",
								   Mask<word>{
									   {27, 26, 0b00}
								   }));
		armInstructions.push_back(std::make_unique<ARM::SingleDataTransferInstruction>(
								   "Single Data Transfer",
								   Mask<word>{
									   {27, 26, 0b01}
								   }));
		armInstructions.push_back(std::make_unique<ARM::InstructionCategory>(
								   "Block Data Transfer",
								   Mask<word>{
									   {27, 26, 0b10}
								   }));
		armInstructions.push_back(std::make_unique<ARM::InstructionCategory>(
								   "Coprocessor Ops",
								   Mask<word>{
									   {27, 26, 0b11}
								   }));

		thumbInstructions = std::vector<std::unique_ptr<const Thumb::InstructionCategory>>();
		/*
		thumbInstructions.push_back(std::make_unique<Thumb::ALUAddSubInstruction>(
										"F2: Add and Subtract",
										Mask<halfword>{
											{ 15, 11, 0b00011 }
										}));
		thumbInstructions.push_back(std::make_unique<Thumb::ALUMoveShiftedRegisterInstruction>(
										"F1: Move Shifted Register",
										Mask<halfword>{
											{ 15, 13, 0b000 }
										}));
		thumbInstructions.push_back(std::make_unique<Thumb::ALUImmediateInstruction>(
										"F3: ALU Operation with Immediate",
										Mask<halfword>{
											{ 15, 13, 0b001 }
										}));
		thumbInstructions.push_back(std::make_unique<Thumb::ALULowRegistersInstruction>(
										"F4: ALU Operation with Low Registers",
										Mask<halfword>{
											{ 15, 10, 0b010000 }
										}));
		// This is a specialization of ALU Op with High Registers (format 5). It doesn't exist in the spec as a separate format.
		// TODO: This creates a gap with undefined behaviour when bit 7 is high.
		thumbInstructions.push_back(std::make_unique<Thumb::BranchExchangeInstruction>(
										"F5.5: Branch and eXchange", 
										Mask<halfword>{
											{ 15,  7, 0b010001110 }
										}));	
		thumbInstructions.push_back(std::make_unique<Thumb::InstructionCategory>(
										"F5: ALU Operation with High Registers", 
										Mask<halfword>{
											{ 15, 10, 0b010001 }
										}));
		thumbInstructions.push_back(std::make_unique<Thumb::LoadPCRelativeInstruction>(
										"F6: PC Relative Load",
										Mask<halfword>{
											{ 15, 11, 0b01001 }
										}));
		thumbInstructions.push_back(std::make_unique<Thumb::LoadStoreRegisterOffsetInstruction>(
										"F7+8: Load/Store with Register Offset",
										Mask<halfword>{
											{ 15, 12, 0b0101 },
										}));
		thumbInstructions.push_back(std::make_unique<Thumb::InstructionCategory>(
										"F9: Load/Store with Immediate offset",
										Mask<halfword>{
											{ 15, 13, 0b011 }
										}));
		thumbInstructions.push_back(std::make_unique<Thumb::LoadStoreHalfwordInstruction>(
										"F10: Load/Store Halfword",
										Mask<halfword>{
											{ 15, 12, 0b1000 }
										}));
		thumbInstructions.push_back(std::make_unique<Thumb::LoadStoreSPRelativeInstruction>(
										"F11: Load/Store relative to Stack Pointer",
										Mask<halfword>{
											{ 15, 12, 0b1001 }
										}));
		thumbInstructions.push_back(std::make_unique<Thumb::InstructionCategory>(
										"F12: Load Address",
										Mask<halfword>{
											{ 15, 12, 0b1010 }
										}));
		thumbInstructions.push_back(std::make_unique<Thumb::OffsetStackPointerInstruction>(
										"F13: Add Offset to Stack Pointer",
										Mask<halfword>{
											{ 15,  8, 0b10110000 }
										}));
		thumbInstructions.push_back(std::make_unique<Thumb::PushPopInstruction>(
										"F14: Push/Pop Registers",
										Mask<halfword>{
											{ 15, 12, 0b1011 },
											{ 10, 9, 0b10 }
										}));
		thumbInstructions.push_back(std::make_unique<Thumb::InstructionCategory>(
										"F15: Multiple Load/Store",
										Mask<halfword>{
											{ 15, 12, 0b1100 }
										}));
		thumbInstructions.push_back(std::make_unique<Thumb::ConditionalBranchInstruction>(
										"F16: Conditional Branch",
										Mask<halfword>{
											{ 15, 12, 0b1101 }
										}));
		thumbInstructions.push_back(std::make_unique<Thumb::InstructionCategory>(
										"F17: Software Interrupt",
										Mask<halfword>{
											{ 15,  8, 0b11011111 }
										}));
		thumbInstructions.push_back(std::make_unique<Thumb::InstructionCategory>(
										"F18: Unconditional Branch",
										Mask<halfword>{
											{ 15, 11, 0b11100 }
										}));
		thumbInstructions.push_back(std::make_unique<Thumb::LongBranchLinkInstruction>(
										"F19: Long Branch w/ Link",
										Mask<halfword>{
											{ 15, 12, 0b1111 }
										}));
		*/

		instructionsAreSetup = true;
	}
	const ARM::InstructionCategory* CPU::matchArmInstruction(word instruction){
		for (auto& instructionUniquePtr : armInstructions){
			if (instructionUniquePtr->mask.matches(instruction))
				return instructionUniquePtr.get();
		}
		return nullptr;
	}
	const Thumb::InstructionCategory* CPU::matchThumbInstruction(halfword instruction){
		for (auto& instructionUniquePtr : thumbInstructions){
			if (instructionUniquePtr->mask.matches(instruction))
				return instructionUniquePtr.get();
		}
		return nullptr;
	}
}
