namespace TurnipEmu::ARM7TDMI {
	// Good Test Instruction: 0x0328f301 (MSR if EQ, from SPSR, flags only, immediate 1 rotated by 6
	class MSRInstruction : public Instruction {
		using Instruction::Instruction;
		
		struct InstructionData {
			bool writeToSpecialPSR;
			bool writeFlagBitsOnly;

			ALUOperand2 operand;
			
			InstructionData(word instructionWord) : operand(instructionWord){
				writeToSpecialPSR = (instructionWord >> 22) & 1;
				writeFlagBitsOnly = ((instructionWord >> 16) & 1) == 0;

				if (!writeFlagBitsOnly){
					assert(operand.useImmediate == false);
				}
			}
		};
		std::string disassembly(word instructionWord) override {
			InstructionData data(instructionWord);
			std::stringstream stream;
			stream << "Transfer ";
			if (data.operand.useImmediate){
				stream << "Immediate Value " << (int)data.operand.immediateValue.baseImmediateValue << " rotated right by " << (int)data.operand.immediateValue.rotation;
			}else{
				data.operand.registerValue.writeDescription(stream);
			}
			stream << " into ";
			if (data.writeToSpecialPSR){
				stream << "SPSR for current mode";
			}else{
				stream << "CPSR";
			}
			if (data.writeFlagBitsOnly){
				stream << " [flag bits only]";
			}
			return stream.str();
		}
		void execute(CPU& cpu, const RegisterPointers registers, word instructionWord) override {
			InstructionData data(instructionWord);

			word* targetRegister = data.writeToSpecialPSR ? &registers.spsr->value : &registers.cpsr->value;
			word writeValue = data.operand.calculateValue(registers, true);
			if (data.writeFlagBitsOnly){
				word flagMask = (0xF << 28);
				writeValue = (writeValue & flagMask) | (*targetRegister & ~flagMask);
			}
			*targetRegister = writeValue;
		}
	};
	class MRSInstruction : public Instruction {
		using Instruction::Instruction;
		
		struct InstructionData {
			bool loadFromSpecialPSR;
			uint8_t destinationRegister : 4;

			InstructionData(word instructionWord){
				loadFromSpecialPSR = (instructionWord >> 22) & 1;
				destinationRegister = (instructionWord >> 12) & 0xF;
			}
		};
		std::string disassembly(word instructionWord) override {
			InstructionData data(instructionWord);
			std::stringstream stream;
			stream << "Transfer ";
			if (data.loadFromSpecialPSR){
				stream << "SPSR for current mode";
			}else{
				stream << "CPSR";
			}
			stream << " into Register " << (int)data.destinationRegister;
			return stream.str();
		}
		void execute(CPU& cpu, const RegisterPointers registers, word instructionWord) override {
			InstructionData data(instructionWord);
			if (data.loadFromSpecialPSR){
				*registers.main[data.destinationRegister] = registers.spsr->value;
			}else{
				*registers.main[data.destinationRegister] = registers.cpsr->value;
			}
		}
	};
	
}
