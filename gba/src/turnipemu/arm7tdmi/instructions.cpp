#include "arm7tdmi.h"
#include "turnipemu/utils.h"

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
					assert(registerValue.shiftRegister != 15);
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

			if (useImmediate){
				return word(immediateValue.baseImmediateValue >> immediateValue.rotation) |
					word(immediateValue.baseImmediateValue << (32 - immediateValue.rotation));
			}else{
				word baseValue = *registers.main[registerValue.baseRegister];
				word baseShiftAmount = registerValue.shiftedByRegister ? *registers.main[registerValue.shiftRegister] :
					registerValue.shiftAmount;
				uint16_t finalShiftAmount = baseShiftAmount & 0b11111;

				switch(registerValue.shiftType){
				case ALUOperand2::RegisterShiftType::LogicalShiftLeft:
					// If baseShiftAmount is a multiple of 32 then the carry should be set => don't use the final amount for the if statement
					if (baseShiftAmount != 0) setCarryIfPossible((baseValue >> (32 - finalShiftAmount)) & 1);
					return word(baseValue << finalShiftAmount);
				case ALUOperand2::RegisterShiftType::LogicalShiftRight:
					if (finalShiftAmount == 0) finalShiftAmount = 32; 
					setCarryIfPossible((baseValue >> (finalShiftAmount - 1)) & 1);
					return word(baseValue >> finalShiftAmount);
				case ALUOperand2::RegisterShiftType::ArithmeticShiftRight:
					if (finalShiftAmount == 0) finalShiftAmount = 32;
					setCarryIfPossible((baseValue >> (finalShiftAmount - 1)) & 1);
					return word( ((int32_t) baseValue) >> finalShiftAmount);
				case ALUOperand2::RegisterShiftType::RotateRight:
					if (finalShiftAmount == 0){
						// RRX, rotate right by 1 using the carry flag
						setCarryIfPossible(baseValue & 1);
						return word((registers.cpsr->carry << 31) | (baseValue >> 1));
					}
					// ROR, normal rotate right
					setCarryIfPossible((baseValue >> (finalShiftAmount - 1)) & 1);
					return word(baseValue >> finalShiftAmount) |
						word(baseValue << (32 - finalShiftAmount));
				default:
					assert(false);
				}
			}
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

	class DataProcessingInstruction : public Instruction{
		using Instruction::Instruction;

		struct OperationOutput {
			word result;
			bool carry;
			bool overflow;

			constexpr OperationOutput(word result) : result(result), carry(false), overflow(false){}
			constexpr OperationOutput(word result, bool carry, bool overflow) : result(result), carry(carry), overflow(overflow) {}
		};
		struct OperationType {
			constexpr static bool Logical = true;
			constexpr static bool Arithmetic = false;
		};
		struct Operation {
			using OperationFunction = std::function<OperationOutput(word arg1, word arg2, int carryIn)>;
			char mnemonic[3];
			OperationFunction execute;
			bool logical;
			bool writeResult;

			Operation(const char* mnemonicFromStr, OperationFunction execute, bool logical)
				: Operation(mnemonicFromStr, execute, logical, true){}
			Operation(const char* mnemonicFromStr, OperationFunction execute, bool logical, bool writeResult)
				: execute(execute), logical(logical), writeResult(writeResult) {
				mnemonic[0] = mnemonicFromStr[0];
				mnemonic[1] = mnemonicFromStr[1];
				mnemonic[2] = mnemonicFromStr[2];
			}
		};
		
		template<bool WithCarry>
		static OperationOutput Add(word arg1, word arg2, int carryIn){
			uint64_t ulongResult = (uint64_t)arg1 + (uint64_t)arg2 + (WithCarry ? carryIn : 0);
			int64_t slongResult = TURNIPEMU_UINT32_TO_SINT64(arg1) + TURNIPEMU_UINT32_TO_SINT64(arg2) + (WithCarry ? carryIn : 0);
			return OperationOutput(
				word(ulongResult),
				(ulongResult >> 32) & 1,
				(slongResult > (1 << 31)) | (slongResult < -(1 << 31))
				);
		}
		template<bool WithCarry, bool Reverse>
		static OperationOutput Sub(word arg1, word arg2, int carryIn){
			if (!WithCarry) carryIn = 1;
			if (Reverse)
				return Add<true>(arg2, ~arg1, carryIn);
			else
				return Add<true>(arg1, ~arg2, carryIn);
		}
		
		const std::array<const Operation, 16> operations = {{
			{
				"AND",
				[](word arg1, word arg2, int carryIn){
					return OperationOutput(arg1 & arg2);
				},
				OperationType::Logical
			},
			{
				"EOR", // XOR
				[](word arg1, word arg2, int carryIn){
					return OperationOutput(arg1 ^ arg2);
				},
				OperationType::Logical
			},
			{
				"SUB",
				Sub<false, false>,
				OperationType::Arithmetic
			},
			{
				"RSB",
				Sub<false, true>,
				OperationType::Arithmetic
			},
			{
				"ADD",
				Add<false>,
				OperationType::Arithmetic
			},
			{
				"ADC",
				Add<true>,
				OperationType::Arithmetic
			},
			{
				"SBC",
				Sub<true, false>,
				OperationType::Arithmetic
			},
			{
				"RSC",
				Sub<true, true>,
				OperationType::Arithmetic
			},
			{
				"TST", // AND without result
				[](word arg1, word arg2, int carryIn){
					return OperationOutput(arg1 & arg2);
				},
				OperationType::Logical,
				false
			},
			{
				"TEQ", // XOR without result
				[](word arg1, word arg2, int carryIn){
					return OperationOutput(arg1 ^ arg2);
				},
				OperationType::Logical,
				false
			},
			{
				"CMP", // SUB without result
				Sub<false, false>,
				OperationType::Arithmetic,
				false
			},
			{
				"CMN", // ADD without result
				Add<false>,
				OperationType::Arithmetic,
				false
			},
			{
				"ORR", // OR
				[](word arg1, word arg2, int carryIn){
					return OperationOutput(arg1 | arg2);
				},
				OperationType::Logical,
			},
			{
				"MOV",
				[](word arg1, word arg2, int carryIn){
					return OperationOutput(arg2);
				},
				OperationType::Logical
			},
			{
				"BIC",
				[](word arg1, word arg2, int carryIn){
					return OperationOutput(arg1 & ~arg2);
				},
				OperationType::Logical,
			},
			{
				"MVN",
				[](word arg1, word arg2, int carryIn){
					return OperationOutput(~arg2);
				},
				OperationType::Logical
			},
			}};
		
		struct InstructionData {
			uint8_t opcode : 4;
			bool setFlags;
			uint8_t operand1Register : 4;
			uint8_t destinationRegister : 4;
			ALUOperand2 operand2;
			bool restoreSPSR;

			InstructionData(word instructionWord) : operand2(instructionWord){
				opcode = (instructionWord >> 21) & 0xF;
				setFlags = (instructionWord >> 20) & 1;
				operand1Register = (instructionWord >> 16) & 0xF;
				destinationRegister = (instructionWord >> 12) & 0xF;

				restoreSPSR = !setFlags && (opcode == 0b1001); // SPSR restoration via TEQP
				if (destinationRegister == 15 && setFlags){
					restoreSPSR = true;
					setFlags = false;
				}
			}
		};

		std::string disassembly(word instructionWord) override {
			InstructionData data(instructionWord);
			if (data.restoreSPSR){
				return "Restores SPSR, ignored in User mode";
			}
			
			const Operation& operation = operations[data.opcode];
			std::stringstream stream;
			stream << "ALU OP " << operation.mnemonic << "\n";
			stream << "Operand 1 Register: " << (int)data.operand1Register << "\n";
			stream << "Operand 2: ";
			if (data.operand2.useImmediate){
				stream << "Immediate Value " << (int)data.operand2.immediateValue.baseImmediateValue << " rotated by " << (int)data.operand2.immediateValue.rotation;
			}else{
				stream << "Register " << (int)data.operand2.registerValue.baseRegister;
				switch(data.operand2.registerValue.shiftType){
				case ALUOperand2::RegisterShiftType::LogicalShiftLeft:
					stream << " logical shift left";
					break;
				case ALUOperand2::RegisterShiftType::LogicalShiftRight:
					stream << " logical shift right";
					break;
				case ALUOperand2::RegisterShiftType::ArithmeticShiftRight:
					stream << " arithmetic shift right";
					break;
				case ALUOperand2::RegisterShiftType::RotateRight:
					stream << " rotate right";
					break;
				}
				stream << " by ";
				if (data.operand2.registerValue.shiftedByRegister){
					stream << "Register " << (int)data.operand2.registerValue.shiftRegister;
				}else{
					stream << (int)data.operand2.registerValue.shiftAmount;
				}
			}
			stream << "\nDestination Register: " << (int)data.destinationRegister;
			return stream.str();
		}
		void execute(CPU& cpu, RegisterPointers currentRegisters, word instructionWord) override {
			InstructionData data(instructionWord);
			if (data.restoreSPSR){
				throw std::runtime_error("Implement SPSR restoration!");
			}
			
			const Operation& operation = operations[data.opcode];

			uint8_t pcOffset = 0;
			if (data.operand1Register == 15 || (!data.operand2.useImmediate && data.operand2.registerValue.baseRegister == 15)){
				if (!data.operand2.useImmediate && data.operand2.registerValue.shiftedByRegister) 
					pcOffset = 12;
				else
					pcOffset = 8;
				*currentRegisters.main[data.destinationRegister] += pcOffset;
			}
			
			word arg1 = *currentRegisters.main[data.operand1Register];
			word arg2 = data.operand2.calculateValue(currentRegisters, data.setFlags);
			OperationOutput output = operation.execute(arg1, arg2, currentRegisters.cpsr->carry ? 1 : 0);
			if (operation.writeResult)
				*currentRegisters.main[data.destinationRegister] = output.result;
			if (data.setFlags){
				currentRegisters.cpsr->overflow = output.overflow;
				currentRegisters.cpsr->carry = output.carry;
				currentRegisters.cpsr->zero = (output.result == 0);
				currentRegisters.cpsr->negative = (output.result >> 31) & 1;
			}

			if (data.destinationRegister == 15)
				*currentRegisters.main[data.destinationRegister] -= pcOffset;
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
