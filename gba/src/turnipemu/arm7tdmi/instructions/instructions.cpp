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
		armInstructions.push_back(std::make_unique<ARMInstructionCategory>(
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
