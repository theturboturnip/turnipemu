#include "arm7tdmi.h"
#include "turnipemu/utils.h"

#include <sstream>

#include "instructions_common.inl"
#include "instructions_alu.inl"
#include "instructions_data_transfer.inl"

namespace TurnipEmu::ARM7TDMI{
#define CONDITION(NAME, CODE) Instruction::Condition{ {NAME[0], NAME[1]}, {#CODE}, [](ProgramStatusRegister status) { return CODE; } }
	const std::array<const Instruction::Condition, 15> Instruction::conditions = {
		CONDITION("EQ", status.zero),
		CONDITION("NE", !status.zero),
		CONDITION("CS", status.carry),
		CONDITION("CC", !status.carry),
		CONDITION("MI", status.negative),
		CONDITION("PL", !status.negative),
		CONDITION("VS", status.overflow),
		CONDITION("VC", !status.overflow),
		CONDITION("HI", status.carry && !status.zero),
		CONDITION("LS", !status.carry || status.zero),
		CONDITION("GE", status.negative == status.overflow),
		CONDITION("LT", status.negative != status.overflow),
		CONDITION("GT", !status.zero && (status.negative == status.overflow)),
		CONDITION("LE", status.zero || (status.negative != status.overflow)),
		CONDITION("AL", true),
	};
#undef CONDITION
	
	InstructionMask::InstructionMask(std::initializer_list<MaskRange> list){
		mask = 0;
		expectedValue = 0;
		for (const auto& range : list){
			range.updateMask(mask);
			range.updateExpectedValue(expectedValue);
		}
	}
	
	Instruction::Instruction(std::string category, InstructionMask mask)
		: category(category), mask(mask)  {
		LogLine("INST", "Created Instruction with category %s, mask 0x%08x, value 0x%08x", category.c_str(), mask.mask, mask.expectedValue);
	}

	const Instruction::Condition& ARM7TDMI::Instruction::getCondition(word instructionWord) {
		return conditions[(instructionWord >> 28) & 0xF];
	}

	class BranchInstruction : public Instruction {
		using Instruction::Instruction;
		
		struct InstructionData {
			int32_t offset;
			bool link;

			InstructionData(word instructionWord){
				offset = ((instructionWord >> 0) & 0xFFFFFF) << 2;
				link = (instructionWord >> 24) & 1;
			}
		};
		std::string disassembly(word instructionWord) override {
			InstructionData data(instructionWord);
			std::stringstream stream;
			stream << "Branch by " << data.offset << ", link: " << std::boolalpha << data.link;
			return stream.str();
		}
		void execute(CPU& cpu, const RegisterPointers registers, word instructionWord) override {
			InstructionData data(instructionWord);
			if (data.link){
				word pcForNextInstruction = *registers.main[15] + 4;
				*registers.main[14] = pcForNextInstruction;
			}
			word pcWithPrefetch = *registers.main[15] + 8;
			*registers.main[15] = pcWithPrefetch + data.offset;
		}
	};

	void CPU::setupInstructions(){
		instructions = std::vector<std::unique_ptr<Instruction>>();
		instructions.push_back(std::make_unique<Instruction>("Software Interrupt", InstructionMask{
					{27, 24, 0b1111}
				}));
		instructions.push_back(std::make_unique<Instruction>("Undefined", InstructionMask{
					{27, 25, 0b011},
					{4, 1}
				}));
		instructions.push_back(std::make_unique<Instruction>("Branch and Exchange", InstructionMask{
					{27, 4, 0b0'0001'0010'1111'1111'1111'0001}
				}));
		instructions.push_back(std::make_unique<BranchInstruction>("Branch", InstructionMask{
					{27, 25, 0b101}
				}));
		instructions.push_back(std::make_unique<Instruction>("Multiply", InstructionMask{
					{27, 23, 0b00000},
					{7, 4, 0b1001}
				}));
		instructions.push_back(std::make_unique<Instruction>("Single Data Swap", InstructionMask{
					{27, 23, 0b00010},
					{21, 20, 0b00},
					{11, 4, 0b0'0000'1001}
				}));
		instructions.push_back(std::make_unique<Instruction>("Halfword Data Transfer", InstructionMask{
					{27, 25, 0b000},
					{7, 1},
					{4, 1}
				}));
		instructions.push_back(std::make_unique<DataProcessingInstruction>("Data Processing", InstructionMask{
					{27, 26, 0b00}
				}));
		instructions.push_back(std::make_unique<SingleDataTransferInstruction>("Single Data Transfer", InstructionMask{
					{27, 26, 0b01}
				}));
		instructions.push_back(std::make_unique<Instruction>("Block Data Transfer", InstructionMask{
					{27, 26, 0b10}
				}));
		instructions.push_back(std::make_unique<Instruction>("Coprocessor Ops", InstructionMask{
					{27, 26, 0b11}
				}));
	}
	Instruction* CPU::matchInstruction(word instructionWord){
		for (auto& instructionUniquePtr : instructions){
			if (instructionUniquePtr->mask.matches(instructionWord))
				return instructionUniquePtr.get();
		}
		return nullptr;
	}
}
