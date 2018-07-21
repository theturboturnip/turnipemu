#include "arm7tdmi.h"

#include <sstream>

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

	struct DataTransferInfo {
		enum class IndexMode {
			PreIndex,
			PostIndex
		};
		enum class TransferMode {
			Load,
			Store
		};
		IndexMode indexMode;
		int offsetSign; // +1/-1
		bool writeback;
		TransferMode transferMode;
		uint8_t baseRegister : 4;

		DataTransferInfo(word instructionWord){
			indexMode = ((instructionWord >> 24) & 1) ? IndexMode::PreIndex : IndexMode::PostIndex;
			offsetSign = ((instructionWord >> 23) & 1) ? +1 : -1;
			writeback = ((instructionWord >> 21) & 1);
			transferMode = ((instructionWord >> 20) & 1) ? TransferMode::Load : TransferMode::Store;
			baseRegister = (instructionWord >> 16);
		}
	};
	struct ALUOperand2 {
		enum class RegisterShiftType {
			LogicalShiftLeft = 0b00,
			LogicalShiftRight = 0b01,
			ArithmeticShiftRight = 0b10,
			RotateRight = 0b11
		};
		
		bool useImmediate;
		union {
			struct {
				uint8_t baseImmediateValue;
				uint8_t rotation;
			} immediateValue;
			struct {
				uint8_t baseRegister : 4;
				RegisterShiftType shiftType : 2;
				bool shiftedByRegister;
				union {
					uint8_t shiftRegister : 4;
					uint8_t shiftAmount : 5;
				};
			} registerValue;
		};

		// Evaluating this can update flags in the registers, we want to make sure that only happens once
		bool hasChangedFlags = false;

		ALUOperand2(word instructionWord){
			useImmediate = (instructionWord >> 25) & 1;
			if (useImmediate){
				immediateValue.baseImmediateValue = instructionWord & 0xFF;
				immediateValue.rotation = ((instructionWord >> 8) & 0xF) * 2;
			}else{
				registerValue.baseRegister = instructionWord & 0xF;
				registerValue.shiftType = static_cast<RegisterShiftType>((instructionWord >> 5) & 0b11);
				registerValue.shiftedByRegister = (instructionWord >> 4) & 1;
				if (registerValue.shiftedByRegister){
					registerValue.shiftRegister = (instructionWord >> 8) & 0xF;
				}else{
					registerValue.shiftAmount = (instructionWord >> 7) & 0b11111;
				}
			}
		}
		word calculateValue(const RegisterPointers registers, bool allowChangeFlags){
			auto setCarryIfPossible = [&](bool set) {
				if (!allowChangeFlags) return;

				assert(!hasChangedFlags);
				registers.cpsr->carry = set;
				hasChangedFlags = true;
			};

			// TODO: Calculate Value
		}
	};

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
		instructions.push_back(std::make_unique<Instruction>("Data Processing", InstructionMask{
					{27, 26, 0b00}
				}));
		instructions.push_back(std::make_unique<Instruction>("Single Data Transfer", InstructionMask{
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
