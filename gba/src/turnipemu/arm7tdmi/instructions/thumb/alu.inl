#include "turnipemu/arm7tdmi/alu.h"
#include "turnipemu/arm7tdmi/instruction_category.h"

namespace TurnipEmu::ARM7TDMI::Instructions::Thumb {
	class ALUMoveShiftedRegisterInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;

		const std::array<const ALU::Operation, 3> operations = {{
				ALU::Thumb::LSL,
				ALU::Thumb::LSR,
				ALU::Thumb::ASR
			}};
		
		struct InstructionData {
			uint8_t opcode : 2;
			
			uint8_t operand1Register : 3;
			uint8_t operand2Immediate : 5;
			uint8_t destinationRegister : 3;


			InstructionData(halfword instruction){
				opcode = (instruction >> 11) & 0b11;
				assert(opcode < 3);
				
				operand1Register = (instruction >> 3) & 0b111;
				operand2Immediate = (instruction >> 6) & 0b11111;
				destinationRegister = (instruction >> 0) & 0b111;
			}
		};
			
	public:
		std::string disassembly(halfword instruction) const override {
			InstructionData data(instruction);
			
			const ALU::Operation& operation = operations[data.opcode];
			std::stringstream stream;
			stream << "ALU OP " << operation.mnemonic << "\n";
			stream << "Operand 1: Register " << (int)data.operand1Register << "\n";
			stream << "Operand 2: Immediate Value " << (int)data.operand2Immediate << "\n";
			stream << "Destination Register: " << (int)data.destinationRegister;
			return stream.str();
		}
		void execute(CPU& cpu, InstructionRegisterInterface registers, halfword instruction) const override {
			InstructionData data(instruction);

			const ALU::Operation& operation = operations[data.opcode];

			word arg1 = registers.get(data.operand1Register);
			word arg2 = data.operand2Immediate;
			ALU::OperationOutput output = operation.execute(arg1, arg2, registers.cpsr().carry ? 1 : 0);
			if (operation.writeResult)
				registers.set(data.destinationRegister, output.result);
			output.applyToPSR(registers.cpsr());
		}
	};
	
	class ALUAddSubInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;

		const std::array<const ALU::Operation, 2> operations = {{
				ALU::ADD,
				ALU::SUB
			}};
		
		struct InstructionData {
			uint8_t opcode : 1;
			uint8_t operand1Register : 3;
			uint8_t destinationRegister : 3;

			bool useImmediate;
			union {
				uint8_t immediateValue : 3;
				uint8_t operand2Register : 3;
			};

			InstructionData(halfword instruction){
				opcode = (instruction >> 9) & 0b1;
				operand1Register = (instruction >> 3) & 0b111;
				destinationRegister = (instruction >> 0) & 0b111;

				useImmediate = (instruction >> 10) & 1;
				immediateValue = (instruction >> 6) & 0b111; // Will set operand2Register
			}
		};
			
	public:
		std::string disassembly(halfword instruction) const override {
			InstructionData data(instruction);

			const ALU::Operation& operation = operations[data.opcode];
			std::stringstream stream;
			stream << "ALU OP " << operation.mnemonic << "\n";
			stream << "Operand 1: Register " << (int)data.operand1Register << "\n";
			if (data.useImmediate)
				stream << "Operand 2: Immediate Value " << (int)data.immediateValue << "\n";
			else
				stream << "Operand 2: Register " << (int)data.operand2Register << "\n";
			stream << "Destination Register: " << (int)data.destinationRegister;
			return stream.str();
		}
		void execute(CPU& cpu, InstructionRegisterInterface registers, halfword instruction) const override {
			InstructionData data(instruction);

			const ALU::Operation& operation = operations[data.opcode];

			word arg1 = registers.get(data.operand1Register);
			word arg2;
			if (data.useImmediate)
				arg2 = data.immediateValue;
			else
				arg2 = registers.get(data.operand2Register);
			ALU::OperationOutput output = operation.execute(arg1, arg2, registers.cpsr().carry ? 1 : 0);
			if (operation.writeResult)
				registers.set(data.destinationRegister, output.result);
			output.applyToPSR(registers.cpsr());
		}
	};

	class ALUImmediateInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;

		const std::array<const ALU::Operation, 4> operations = {{
				ALU::MOV,
				ALU::CMP,
				ALU::ADD,
				ALU::SUB,
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
		void execute(CPU& cpu, InstructionRegisterInterface registers, halfword instruction) const override {
			InstructionData data(instruction);

			const ALU::Operation& operation = operations[data.opcode];

			word arg1 = registers.get(data.operand1Register);
			byte arg2 = data.immediateValue;
			ALU::OperationOutput output = operation.execute(arg1, arg2, registers.cpsr().carry ? 1 : 0);
			if (operation.writeResult)
				registers.set(data.destinationRegister, output.result);
			output.applyToPSR(registers.cpsr());
		}
	};
	
	class ALULowRegistersInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;

		const std::array<const ALU::Operation, 16> operations = {{
				ALU::AND,
				ALU::EOR,
				ALU::Thumb::LSL,
				ALU::Thumb::LSR,
				ALU::Thumb::ASR,
				ALU::ADC,
				ALU::SBC,
				ALU::Thumb::ROR,
				ALU::TST,
				ALU::Thumb::NEG,
				ALU::CMP,
				ALU::CMN,
				ALU::ORR,
				ALU::Thumb::MUL,
				ALU::BIC,
				ALU::MVN,
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
		void execute(CPU& cpu, InstructionRegisterInterface registers, halfword instruction) const override {
			InstructionData data(instruction);
		  
			const ALU::Operation& operation = operations[data.opcode];
		  
			word arg1 = registers.get(data.operand1Register);
			word arg2 = registers.get(data.operand2Register);
			ALU::OperationOutput output = operation.execute(arg1, arg2, registers.cpsr().carry ? 1 : 0);
			if (operation.writeResult)
				registers.set(data.destinationRegister, output.result);
			output.applyToPSR(registers.cpsr());
		}
	};

	class ALUHighRegistersInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;

		const std::array<const ALU::Operation, 3> operations = {{
				ALU::ADD,
				ALU::CMP,
				ALU::MOV,
			}};

		struct InstructionData {
			uint8_t opcode : 2;
			union {
				uint8_t destinationRegister : 4;
				uint8_t operand1Register : 4;
			};
			uint8_t operand2Register : 4;

			InstructionData(halfword instruction){
				opcode = (instruction >> 8) & 0b11;
				
				operand1Register = (instruction >> 0) & 0b111;
				operand1Register |= ((instruction >> 7) & 1) << 3;

				operand2Register = (instruction >> 3) & 0b111;
				operand2Register |= ((instruction >> 6) & 1) << 3;
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
		void execute(CPU& cpu, InstructionRegisterInterface registers, halfword instruction) const override {
			InstructionData data(instruction);
		  
			const ALU::Operation& operation = operations[data.opcode];
		  
			word arg1 = registers.get(data.operand1Register);
			word arg2 = registers.get(data.operand2Register);
			ALU::OperationOutput output = operation.execute(arg1, arg2, registers.cpsr().carry ? 1 : 0);
			if (operation.writeResult)
				registers.set(data.destinationRegister, output.result);
			if (data.opcode == 0b01)
				output.applyToPSR(registers.cpsr()); // Only CMP sets the flags in this mode
		}
	};
}
