#include "turnipemu/arm7tdmi/alu.h"

namespace TurnipEmu::ARM7TDMI::Instructions::ARM {
	using namespace TurnipEmu::ARM7TDMI::ALU;
	
	class DataProcessingInstruction : public InstructionCategory{
		using InstructionCategory::InstructionCategory;
		
		const std::array<const Operation, 16> operations = {{
				ALU::AND,
				ALU::EOR,
				ALU::SUB,
				ALU::RSB,
				ALU::ADD,
				ALU::ADC,
				ALU::SBC,
				ALU::RSC,
				ALU::TST,
				ALU::TEQ,
				ALU::CMP,
				ALU::CMN,
				ALU::ORR,
				ALU::MOV,
				ALU::BIC,
				ALU::MVN,
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

		std::string disassembly(word instructionWord) const override {
			InstructionData data(instructionWord);
			if (data.restoreSPSR){
				return "Restores SPSR, ignored in User mode";
			}
			
			const Operation& operation = operations[data.opcode];
			std::stringstream stream;
			stream << "ALU OP " << operation.mnemonic << "\n";
			stream << "Operand 1: Register " << (int)data.operand1Register << "\n";
			stream << "Operand 2: ";
			if (data.operand2.useImmediate){
				stream << "Immediate Value " << (int)data.operand2.immediateValue.baseImmediateValue << " rotated by " << (int)data.operand2.immediateValue.rotation;
			}else{
				data.operand2.registerValue.writeDescription(stream);
			}
			stream << "\nDestination Register: " << (int)data.destinationRegister;
			return stream.str();
		}
		void execute(CPU& cpu, InstructionRegisterInterface currentRegisters, word instructionWord) const override {
			InstructionData data(instructionWord);
			if (data.restoreSPSR){
				if (currentRegisters.cpsr().mode == Mode::User) return;
				throw std::runtime_error("Implement SPSR restoration!");
			}
			
			const Operation& operation = operations[data.opcode];
			
			word arg1 = currentRegisters.get(data.operand1Register);
			word arg2 = data.operand2.calculateValue(currentRegisters, data.setFlags);
			OperationOutput output = operation.execute(arg1, arg2, currentRegisters.cpsr().carry ? 1 : 0);
			if (operation.writeResult)
				currentRegisters.set(data.destinationRegister, output.result);
			if (data.setFlags){ // TODO: Does it matter whether the instruction is arithmetic or logical?
				output.applyToPSR(currentRegisters.cpsr());
			}
		}
	};
}
