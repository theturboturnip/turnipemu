#include "turnipemu/arm7tdmi/alu.h"
#include "turnipemu/arm7tdmi/instruction_category.h"

namespace TurnipEmu::ARM7TDMI::Instructions::Thumb {
	class ALUMoveShiftedRegisterInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;

		constexpr static std::array<const ALU::Operation*, 4> operations = {{
				&ALU::Thumb::LSL,
				&ALU::Thumb::LSR,
				&ALU::Thumb::ASR,
				nullptr
			}};
		
		struct InstructionData {
			ALU::Request request;
			
			InstructionData(halfword instruction){
				request.op = operations[(instruction >> 11) & 0b11];
				assert(request.op);

				request.operand1.SetRegisterIndex((instruction >> 3) & 0b111);
				request.operand2.SetImmediateValue((instruction >> 6) & 0b11111);
				request.destinationRegister = (instruction >> 0) & 0b111;
			}
		};
			
	public:
		std::string disassembly(word instruction) const override {
			InstructionData data(instruction);
			
			std::stringstream stream;
			stream << data.request;
			return stream.str();
		}
		void execute(CPU& cpu, InstructionRegisterInterface registers, word instruction) const override {
			InstructionData data(instruction);
			data.request.Evaluate(registers);
		}
	};
	
	class ALUAddSubInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;

		constexpr static std::array<const ALU::Operation*, 2> operations = {{
				&ALU::ADD,
				&ALU::SUB
			}};
		
		struct InstructionData {
			ALU::Request request;

			InstructionData(halfword instruction){
				request.op = operations[(instruction >> 9) & 0b1];
				
				request.operand1.SetRegisterIndex((instruction >> 3) & 0b111);

				if ((instruction >> 10) & 1){
					request.operand2.SetImmediateValue((instruction >> 6) & 0b111);
				}else{
					request.operand2.SetRegisterIndex((instruction >> 6) & 0b111);
				}
				
				request.destinationRegister = (instruction >> 0) & 0b111;
			}
		};
			
	public:
		std::string disassembly(word instruction) const override {
			InstructionData data(instruction);
			
			std::stringstream stream;
			stream << data.request;
			return stream.str();
		}
		void execute(CPU& cpu, InstructionRegisterInterface registers, word instruction) const override {
			InstructionData data(instruction);
			data.request.Evaluate(registers);
		}
	};

	class ALUImmediateInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;

		constexpr static std::array<const ALU::Operation*, 4> operations = {{
				&ALU::MOV,
				&ALU::CMP,
				&ALU::ADD,
				&ALU::SUB,
			}};
		
		struct InstructionData {
			ALU::Request request;

			InstructionData(halfword instruction){
				request.op = operations[(instruction >> 11) & 0b11];
				
				request.destinationRegister = (instruction >> 8) & 0b111;
				request.operand1.SetRegisterIndex(request.destinationRegister);
				request.operand2.SetImmediateValue((instruction >> 0) & 0xFF);
			}
		};
			
	public:
		std::string disassembly(word instruction) const override {
			InstructionData data(instruction);
			
			std::stringstream stream;
			stream << data.request;
			return stream.str();
		}
		void execute(CPU& cpu, InstructionRegisterInterface registers, word instruction) const override {
			InstructionData data(instruction);
			data.request.Evaluate(registers);
		}
	};
	
	class ALULowRegistersInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;

		constexpr static std::array<const ALU::Operation*, 16> operations = {{
				&ALU::AND,
				&ALU::EOR,
				&ALU::Thumb::LSL,
				&ALU::Thumb::LSR,
				&ALU::Thumb::ASR,
				&ALU::ADC,
				&ALU::SBC,
				&ALU::Thumb::ROR,
				&ALU::TST,
				&ALU::Thumb::NEG,
				&ALU::CMP,
				&ALU::CMN,
				&ALU::ORR,
				nullptr,
				&ALU::BIC,
				&ALU::MVN,
			}};

		struct InstructionData {
			ALU::Request request;
			ALU::MultiplyRequest multiplyRequest;
			bool isNormalRequest;
				
			InstructionData(halfword instruction){
				request.op = operations[(instruction >> 6) & 0xF];
				
				if (request.op == nullptr){
					isNormalRequest = false;

					multiplyRequest.destinationRegister1 = (instruction >> 0) & 0b111;
					multiplyRequest.operand1.SetRegisterIndex(multiplyRequest.destinationRegister1);
					multiplyRequest.operand2.SetRegisterIndex((instruction >> 3) & 0b111);

					multiplyRequest.isSigned = false;
					multiplyRequest.accumulate = false;
					multiplyRequest.isLong = false;
				}else{
					isNormalRequest = true;
					
					request.destinationRegister = (instruction >> 0) & 0b111;
					request.operand1.SetRegisterIndex(request.destinationRegister);
					request.operand2.SetRegisterIndex((instruction >> 3) & 0b111);
				}
			}
		};

	public:
		std::string disassembly(word instruction) const override {
			InstructionData data(instruction);
			
			std::stringstream stream;
			if (data.isNormalRequest)
				stream << data.request;
			else
				stream << data.multiplyRequest;
			return stream.str();
		}
		void execute(CPU& cpu, InstructionRegisterInterface registers, word instruction) const override {
			InstructionData data(instruction);
			if (data.isNormalRequest)
				data.request.Evaluate(registers);
			else
				data.multiplyRequest.Evaluate(registers);
		}
	};

	class ALUHighRegistersInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;

		constexpr static std::array<const ALU::Operation*, 3> operations = {{
				&ALU::ADD,
				&ALU::CMP,
				&ALU::MOV,
			}};

		struct InstructionData {
			ALU::Request request;

			InstructionData(halfword instruction){
				uint8_t opcode = (instruction >> 8) & 0b11;
				request.op = operations[opcode];
				request.setFlags = (opcode == 0b01); // Only CMP sets the flags in this mode

				request.destinationRegister = (instruction >> 0) & 0b111;
				request.destinationRegister |= ((instruction >> 7) & 1) << 3;
				
				request.operand1.SetRegisterIndex(request.destinationRegister);

				request.operand2.SetRegisterIndex( ((instruction >> 3) & 0b111) |
												   (((instruction >> 6) & 1) << 3) );
			}
		};

	public:
		std::string disassembly(word instruction) const override {
			InstructionData data(instruction);
			
			std::stringstream stream;
			stream << data.request;
			return stream.str();
		}
		void execute(CPU& cpu, InstructionRegisterInterface registers, word instruction) const override {
			InstructionData data(instruction);
			data.request.Evaluate(registers);
		}
	};

	
	class ALULoadAddresswithOffset : public InstructionCategory {
	using InstructionCategory::InstructionCategory;

		struct InstructionData {
			ALU::Request request;

			InstructionData(halfword instruction){
				request.op = &ALU::ADD;
				request.setFlags = false;
				request.operand1.SetRegisterIndex(((instruction >> 11) & 1) ? 13 : 15);
				request.operand2.SetImmediateValue((instruction & 0xFF) << 2);
				request.destinationRegister = (instruction >> 8) & 0b111;
			}
		};
		
	public:
		std::string disassembly(word instruction) const override {
			InstructionData data(instruction);
			
			std::stringstream stream;
			stream << data.request;
			return stream.str();
		}
		void execute(CPU& cpu, InstructionRegisterInterface registers, word instruction) const override {
			InstructionData data(instruction);
			data.request.Evaluate(registers);
		}
	};
}
