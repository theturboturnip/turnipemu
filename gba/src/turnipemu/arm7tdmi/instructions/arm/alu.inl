#include "turnipemu/arm7tdmi/alu.h"

namespace TurnipEmu::ARM7TDMI::Instructions::ARM {
	using namespace TurnipEmu::ARM7TDMI::ALU;
	
	class DataProcessingInstruction : public InstructionCategory{
		using InstructionCategory::InstructionCategory;
		
		constexpr static std::array<const Operation*, 16> operations = {{
				&ALU::AND,
				&ALU::EOR,
				&ALU::SUB,
				&ALU::RSB,
				&ALU::ADD,
				&ALU::ADC,
				&ALU::SBC,
				&ALU::RSC,
				&ALU::TST,
				&ALU::TEQ,
				&ALU::CMP,
				&ALU::CMN,
				&ALU::ORR,
				&ALU::MOV,
				&ALU::BIC,
				&ALU::MVN,
			}};
		
		struct InstructionData {
			ALU::Request request;

			InstructionData(word instructionWord){
				request.op = operations[(instructionWord >> 21) & 0xF];
				request.setFlags = (instructionWord >> 20) & 1;
				
				request.operand1.type = RequestInput::ValueType::Register;
				request.operand1.registerIndex = (instructionWord >> 16) & 0xF;

				request.operand2.type = (instructionWord >> 25) & 1 ?
					RequestInput::ValueType::Immediate : RequestInput::ValueType::Register;
				if (request.operand2.type == RequestInput::ValueType::Immediate){
					request.operand2.immediateValue = instructionWord & 0xFF;
					request.operand2.immediateRotation = ((instructionWord >> 8) & 0xF) * 2;
				}else{
					request.operand2.registerIndex = instructionWord & 0xF;
					request.operand2.shift.enabled = true;
					request.operand2.shift.shiftType = static_cast<RequestInput::ShiftType>((instructionWord >> 5) & 0b11);
					request.operand2.shift.amountType = (instructionWord >> 4) & 1 ?
						RequestInput::ValueType::Register : RequestInput::ValueType::Immediate;
					if (request.operand2.shift.amountType == RequestInput::ValueType::Register){
						request.operand2.shift.amountRegisterIndex = (instructionWord >> 8) & 0xF;
						assert(request.operand2.shift.amountRegisterIndex != 15);
					}else{
						request.operand2.shift.amountImmediateValue = (instructionWord >> 7) & 0b11111;
					}
				}

				request.destinationRegister = (instructionWord >> 12) & 0xF;

				// TODO: SPSR Restoration
			}
		};

		std::string disassembly(word instructionWord) const override {
			InstructionData data(instructionWord);
			//if (data.restoreSPSR){
			//	return "Restores SPSR, ignored in User mode";
			//}
			
			std::stringstream stream;
			stream << data.request;
			return stream.str();
		}
		void execute(CPU& cpu, InstructionRegisterInterface currentRegisters, word instructionWord) const override {
			InstructionData data(instructionWord);
			//if (data.restoreSPSR){
			//	if (currentRegisters.cpsr().mode == Mode::User) return;
			//	throw std::runtime_error("Implement SPSR restoration!");
			//}
			
			data.request.Evaluate(currentRegisters);
		}
	};
}
