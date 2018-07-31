#include "turnipemu/arm7tdmi/alu.h"
#include "turnipemu/arm7tdmi/instruction_category.h"

namespace TurnipEmu::ARM7TDMI::Thumb {
	class ALUImmediateInstruction : public ThumbInstructionCategory {
		using ThumbInstructionCategory::ThumbInstructionCategory;

		const std::array<const ALU::Operation, 4> operations = {{
				{
					"MOV",
					[](word arg1, word arg2, int carryFlag){
						return ALU::OperationOutput(arg2 & 0xFF);
					},
					ALU::OperationType::Logical
				},
				{
					"CMP", // SUB without result
					ALU::Sub<false, false>,
					ALU::OperationType::Arithmetic,
					false
				},
				{
					"ADD",
					ALU::Add<false>,
					ALU::OperationType::Arithmetic
				},
				{
					"SUB",
					ALU::Sub<false, false>,
					ALU::OperationType::Arithmetic
				}
			}};
		
		struct InstructionData {
			uint8_t opcode : 2;
			union {
				uint8_t operand1Register : 3;
				uint8_t destinationRegister : 3;
			};
			uint8_t immediateValue;

			InstructionData(halfword instruction){
				opcode = (instruction >> 11) & 0b11;
				operand1Register = (instruction >> 8) & 0b111;
				immediateValue = (instruction >> 0) & 0xFF;
			}
		};
			
	public:
		std::string disassembly(halfword instruction) const override {
			InstructionData data(instruction);

			const ALU::Operation& operation = operations[data.opcode];
			std::stringstream stream;
			stream << "ALU OP " << operation.mnemonic << "\n";
			stream << "Operand 1: Register " << (int)data.operand1Register << "\n";
			stream << "Operand 2: Immediate Value " << (int)data.immediateValue << "\n";
			stream << "Destination Register: " << (int)data.destinationRegister;
			return stream.str();
		}
		void execute(CPU& cpu, RegisterPointers registers, halfword instruction) const override {
			InstructionData data(instruction);

			const ALU::Operation& operation = operations[data.opcode];

			word arg1 = *registers.main[data.operand1Register];
			word arg2 = data.immediateValue;
			ALU::OperationOutput output = operation.execute(arg1, arg2, registers.cpsr->carry ? 1 : 0);
			if (operation.writeResult)
				*registers.main[data.destinationRegister] = output.result;
			output.applyToPSR(registers.cpsr);
		}
	};
	
	class ALULowRegistersInstruction : public ThumbInstructionCategory {
		using ThumbInstructionCategory::ThumbInstructionCategory;

		const std::array<const ALU::Operation, 16> operations = {{
				{
					"AND",
					[](word arg1, word arg2, int carryIn){
						return ALU::OperationOutput(arg1 & arg2);
					},
					ALU::OperationType::Logical
				},
				{
					"EOR", // XOR
					[](word arg1, word arg2, int carryIn){
						return ALU::OperationOutput(arg1 ^ arg2);
					},
					ALU::OperationType::Logical
				},
				{
					"LSL",
					[](word arg1, word arg2, int carryIn){
						return ALU::OperationOutput(arg1 << arg2);
					},
					ALU::OperationType::Logical
				},
				{
					"LSR",
					[](word arg1, word arg2, int carryIn){
						return ALU::OperationOutput(arg1 >> arg2);
					},
					ALU::OperationType::Logical
				},
				{
					"ASR", // Arithmetic Shift Right
					[](word arg1, word arg2, int carryIn){
						return ALU::OperationOutput(arg1 >> arg2);
					},
					ALU::OperationType::Arithmetic
				},
				{
					"ADC", // Add with Carry
					ALU::Add<true>,
					ALU::OperationType::Arithmetic
				},
				{
					"SBC", // Sub with Carry
					ALU::Sub<true, false>,
					ALU::OperationType::Arithmetic
				},
				{
					"ROR", // Rotate Right (no carry)
					[](word arg1, word arg2, int carryIn){
						return ALU::OperationOutput(
							arg1 >> arg2 |
							((arg1 << (32 - arg2)) & word(~0))
							);
					},
					ALU::OperationType::Logical
				},
				{
					"TST", // AND without result
					[](word arg1, word arg2, int carryIn){
						return ALU::OperationOutput(arg1 & arg2);
					},
					ALU::OperationType::Logical,
					false
				},
				{
					"NEG", // Negate
					[](word arg1, word arg2, int carryIn){
						// Use ALU::Sub to make sure the flags will be correct
						return ALU::Sub<false, false>(0, arg2, carryIn);
					},
					ALU::OperationType::Arithmetic
				},
				{
					"CMP", // SUB without result
					ALU::Sub<false, false>,
					ALU::OperationType::Arithmetic,
					false
				},
				{
					"CMN", // ADD without result
					ALU::Add<false>,
					ALU::OperationType::Arithmetic,
					false
				},
				{
					"ORR", // OR
					[](word arg1, word arg2, int carryIn){
						return ALU::OperationOutput(arg1 | arg2);
					},
					ALU::OperationType::Logical,
				},
				{
					"MUL", // Multiply
					[](word arg1, word arg2, int carryIn){
						// TODO: Flags
						throw std::runtime_error("Thumb MUL doesn't have the correct flags implemented");
						return ALU::OperationOutput(arg1 * arg2);
					},
					ALU::OperationType::Arithmetic
				},
				{
					"BIC", // A AND NOT B
					[](word arg1, word arg2, int carryIn){
						return ALU::OperationOutput(arg1 & ~arg2);
					},
					ALU::OperationType::Logical,
				},
				{
					"MVN", // NOT B
					[](word arg1, word arg2, int carryIn){
						return ALU::OperationOutput(~arg2);
					},
					ALU::OperationType::Logical
				},
			}};

		struct InstructionData {
			uint8_t opcode : 4;
			union {
				uint8_t destinationRegister : 3;
				uint8_t operand1Register : 3;
			};
			uint8_t operand2Register : 3;

			InstructionData(halfword instruction){
				opcode = (instruction >> 6) & 0xF;
				operand1Register = (instruction >> 0) & 0b111;
				operand2Register = (instruction >> 3) & 0b111;
			}
		};

	public:
		std::string disassembly(halfword instruction) const override {
			InstructionData data(instruction);

			const ALU::Operation& operation = operations[data.opcode];
			std::stringstream stream;
			stream << "ALU OP " << operation.mnemonic << "\n";
			stream << "Operand 1: Register " << (int)data.operand1Register << "\n";
			stream << "Operand 2: Register " << (int)data.operand2Register << "\n";
			stream << "Destination Register: " << (int)data.destinationRegister;
			return stream.str();
		}
		/*void execute(CPU& cpu, const RegisterPointers registers, halfword instruction) const override {
		  InstructionData data(instruction);
			
		  }*/
	};
}
