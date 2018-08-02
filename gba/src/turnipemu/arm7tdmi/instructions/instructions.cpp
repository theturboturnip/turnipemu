#include "turnipemu/arm7tdmi/cpu.h"

#include <sstream>

#include "turnipemu/utils.h"

#include "common.inl"
#include "arm/all.inl"
#include "thumb/all.inl"

namespace TurnipEmu::ARM7TDMI{
	bool CPU::instructionsAreSetup = false;
	std::vector<std::unique_ptr<const ARMInstructionCategory>> CPU::armInstructions;
	std::vector<std::unique_ptr<const ThumbInstructionCategory>> CPU::thumbInstructions;
	
	void CPU::setupInstructions(){
		if (instructionsAreSetup) return;
		
		armInstructions = std::vector<std::unique_ptr<const ARMInstructionCategory>>();
		armInstructions.push_back(std::make_unique<ARMInstructionCategory>(
								   "Software Interrupt",
								   Mask<word>{
									   {27, 24, 0b1111}
								   }));
		armInstructions.push_back(std::make_unique<ARMInstructionCategory>(
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
		armInstructions.push_back(std::make_unique<ARMInstructionCategory>(
								   "Multiply",
								   Mask<word>{
									   {27, 23, 0b00000},
									   {7, 4, 0b1001}
								   }));
		armInstructions.push_back(std::make_unique<ARMInstructionCategory>(
								   "Single Data Swap",
								   Mask<word>{
									   {27, 23, 0b00010},
									   {21, 20, 0b00},
									   {11, 4, 0b0'0000'1001}
								   }));
		armInstructions.push_back(std::make_unique<ARMInstructionCategory>(
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
		armInstructions.push_back(std::make_unique<ARMInstructionCategory>(
								   "Block Data Transfer",
								   Mask<word>{
									   {27, 26, 0b10}
								   }));
		armInstructions.push_back(std::make_unique<ARMInstructionCategory>(
								   "Coprocessor Ops",
								   Mask<word>{
									   {27, 26, 0b11}
								   }));

		thumbInstructions = std::vector<std::unique_ptr<const ThumbInstructionCategory>>();
		thumbInstructions.push_back(std::make_unique<ThumbInstructionCategory>(
										"Add and Subtract",
										Mask<halfword>{
											{ 15, 10, 0b000111 }
										}));
		thumbInstructions.push_back(std::make_unique<ThumbInstructionCategory>(
										"Move Shifted Register",
										Mask<halfword>{
											{ 15, 13, 0b000 }
										}));
		thumbInstructions.push_back(std::make_unique<Thumb::ALUImmediateInstruction>(
										"ALU Operation with Immediate",
										Mask<halfword>{
											{ 15, 13, 0b001 }
										}));
		thumbInstructions.push_back(std::make_unique<Thumb::ALULowRegistersInstruction>(
										"ALU Operation with Low Registers",
										Mask<halfword>{
											{ 15, 10, 0b010000 }
										}));
		// This is a specialization of ALU Op with High Registers (format 5). It doesn't exist in the spec as a separate format.
		// TODO: This creates a gap with undefined behaviour when bit 7 is high.
		thumbInstructions.push_back(std::make_unique<ThumbInstructionCategory>(
										"Branch and eXchange", 
										Mask<halfword>{
											{ 15,  7, 0b010001110 }
										}));	
		thumbInstructions.push_back(std::make_unique<ThumbInstructionCategory>(
										"ALU Operation with High Registers", 
										Mask<halfword>{
											{ 15, 10, 0b010001 }
										}));
		thumbInstructions.push_back(std::make_unique<ThumbInstructionCategory>(
										"PC Relative Load",
										Mask<halfword>{
											{ 15, 11, 0b01001 }
										}));
		thumbInstructions.push_back(std::make_unique<ThumbInstructionCategory>(
										"Load/Store with Relative Offset",
										Mask<halfword>{
											{ 15, 12, 0b0101 },
											{ 9, 0 }
										}));
		// TODO: V. similar to previous, merge?
		thumbInstructions.push_back(std::make_unique<ThumbInstructionCategory>(
										"Load/Store sign-extended with Relative Offset",
										Mask<halfword>{
											{ 15, 12, 0b0101 },
											{ 9, 1 }
										}));
		thumbInstructions.push_back(std::make_unique<ThumbInstructionCategory>(
										"Load/Store with Immediate offset",
										Mask<halfword>{
											{ 15, 13, 0b011 }
										}));
		thumbInstructions.push_back(std::make_unique<ThumbInstructionCategory>(
										"Load/Store Halfword",
										Mask<halfword>{
											{ 15, 12, 0b1000 }
										}));
		thumbInstructions.push_back(std::make_unique<ThumbInstructionCategory>(
										"Load/Store relative to Stack Pointer",
										Mask<halfword>{
											{ 15, 12, 0b1001 }
										}));
		thumbInstructions.push_back(std::make_unique<ThumbInstructionCategory>(
										"Load Address",
										Mask<halfword>{
											{ 15, 12, 0b1010 }
										}));
		thumbInstructions.push_back(std::make_unique<ThumbInstructionCategory>(
										"Add Offset to Stack Pointer",
										Mask<halfword>{
											{ 15,  8, 0b10110000 }
										}));
		thumbInstructions.push_back(std::make_unique<ThumbInstructionCategory>(
										"Push/Pop Registers",
										Mask<halfword>{
											{ 15, 12, 0b1011 },
											{ 10, 9, 0b10 }
										}));
		thumbInstructions.push_back(std::make_unique<ThumbInstructionCategory>(
										"Multiple Load/Store",
										Mask<halfword>{
											{ 15, 12, 0b1100 }
										}));
		// TODO: Override this specific category to include conditions
		thumbInstructions.push_back(std::make_unique<ThumbInstructionCategory>(
										"Conditional Branch",
										Mask<halfword>{
											{ 15, 12, 0b1101 }
										}));
		thumbInstructions.push_back(std::make_unique<ThumbInstructionCategory>(
										"Software Interrupt",
										Mask<halfword>{
											{ 15,  8, 0b11011111 }
										}));
		thumbInstructions.push_back(std::make_unique<ThumbInstructionCategory>(
										"Unconditional Branch",
										Mask<halfword>{
											{ 15, 11, 0b11100 }
										}));
		thumbInstructions.push_back(std::make_unique<ThumbInstructionCategory>(
										"Long Branch w/ Link",
										Mask<halfword>{
											{ 15, 12, 0b1111 }
										}));

		instructionsAreSetup = true;
	}
	const ARMInstructionCategory* CPU::matchArmInstruction(word instruction){
		for (auto& instructionUniquePtr : armInstructions){
			if (instructionUniquePtr->mask.matches(instruction))
				return instructionUniquePtr.get();
		}
		return nullptr;
	}
	const ThumbInstructionCategory* CPU::matchThumbInstruction(halfword instruction){
		for (auto& instructionUniquePtr : thumbInstructions){
			if (instructionUniquePtr->mask.matches(instruction))
				return instructionUniquePtr.get();
		}
		return nullptr;
	}
}
