#include "turnipemu/arm7tdmi/cpu.h"

#include <sstream>

#include "turnipemu/utils.h"

#include "instructions_common.inl"
#include "instructions_alu.inl"
#include "instructions_data_transfer.inl"
#include "instructions_msr_mrs.inl"

namespace TurnipEmu::ARM7TDMI{
#define CONDITION(NAME, CODE) InstructionCategory::Condition{ {NAME[0], NAME[1]}, {#CODE}, [](ProgramStatusRegister status) { return CODE; } }
	const std::array<const InstructionCategory::Condition, 15> InstructionCategory::conditions = {
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
	
	InstructionCategory::Mask::Mask(std::initializer_list<Range> list){
		mask = 0;
		expectedValue = 0;
		for (const auto& range : list){
			range.updateMask(mask);
			range.updateExpectedValue(expectedValue);
		}
	}
	
	InstructionCategory::InstructionCategory(std::string name, Mask mask)
		: name(name), mask(mask)  {
		LogLine("INST", "Created InstructionCategory with name %s, mask 0x%08x, value 0x%08x", name.c_str(), mask.mask, mask.expectedValue);
	}

	const InstructionCategory::Condition& InstructionCategory::getCondition(word instructionWord) {
		return conditions[(instructionWord >> 28) & 0xF];
	}

	class BranchInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;
		
		struct InstructionData {
			int64_t offset;
			bool link;

			InstructionData(word instructionWord){
				offset = (int64_t)(int32_t)((instructionWord >> 0) & 0xFFFFFF) << 2;
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
				word pcForNextInstruction = *registers.main[15] + 4 - 8; // PC has been prefetched, the -8 undoes that
				*registers.main[14] = pcForNextInstruction;
			}
			word pcWithPrefetch = *registers.main[15];
			*registers.main[15] = pcWithPrefetch + data.offset;
			cpu.queuePipelineFlush();
		}
	};

	void CPU::setupInstructions(){
		instructions = std::vector<std::unique_ptr<InstructionCategory>>();
		
		instructions.push_back(std::make_unique<InstructionCategory>(
								   "Software Interrupt",
								   InstructionCategory::Mask{
									   {27, 24, 0b1111}
								   }));
		instructions.push_back(std::make_unique<InstructionCategory>(
								   "Undefined",
								   InstructionCategory::Mask{
									   {27, 25, 0b011},
									   {4, 1}
								   }));
		instructions.push_back(std::make_unique<InstructionCategory>(
								   "Branch and Exchange",
								   InstructionCategory::Mask{
									   {27, 4, 0b0'0001'0010'1111'1111'1111'0001}
								   }));
		instructions.push_back(std::make_unique<BranchInstruction>(
								   "Branch",
								   InstructionCategory::Mask{
									   {27, 25, 0b101}
								   }));
		instructions.push_back(std::make_unique<MRSInstruction>(
								   "MRS (Transfer PSR to Register)",
								   InstructionCategory::Mask{
									   {27, 26, 0b00},
									   {24, 23, 0b10},
									   {21, 16, 0b001111},
									   {11, 0, 0}
								   }));
		instructions.push_back(std::make_unique<MSRInstruction>(
								   "MSR (Transfer Register to PSR)",
								   InstructionCategory::Mask{
									   {27, 26, 0b00},
									   {24, 23, 0b10},
									   {21, 17, 0b10100},
									   {15, 12, 0b1111}
								   }));
		instructions.push_back(std::make_unique<InstructionCategory>(
								   "Multiply",
								   InstructionCategory::Mask{
									   {27, 23, 0b00000},
									   {7, 4, 0b1001}
								   }));
		instructions.push_back(std::make_unique<InstructionCategory>(
								   "Single Data Swap",
								   InstructionCategory::Mask{
									   {27, 23, 0b00010},
									   {21, 20, 0b00},
									   {11, 4, 0b0'0000'1001}
								   }));
		instructions.push_back(std::make_unique<InstructionCategory>(
								   "Halfword Data Transfer",
								   InstructionCategory::Mask{
									   {27, 25, 0b000},
									   {7, 1},
									   {4, 1}
								   }));
		instructions.push_back(std::make_unique<DataProcessingInstruction>(
								   "Data Processing",
								   InstructionCategory::Mask{
									   {27, 26, 0b00}
								   }));
		instructions.push_back(std::make_unique<SingleDataTransferInstruction>(
								   "Single Data Transfer",
								   InstructionCategory::Mask{
									   {27, 26, 0b01}
								   }));
		instructions.push_back(std::make_unique<InstructionCategory>(
								   "Block Data Transfer",
								   InstructionCategory::Mask{
									   {27, 26, 0b10}
								   }));
		instructions.push_back(std::make_unique<InstructionCategory>(
								   "Coprocessor Ops",
								   InstructionCategory::Mask{
									   {27, 26, 0b11}
								   }));
	}
	InstructionCategory* CPU::matchInstruction(word instructionWord){
		for (auto& instructionUniquePtr : instructions){
			if (instructionUniquePtr->mask.matches(instructionWord))
				return instructionUniquePtr.get();
		}
		return nullptr;
	}
}
