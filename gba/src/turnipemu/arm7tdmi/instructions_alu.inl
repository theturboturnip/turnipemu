namespace TurnipEmu::ARM7TDMI {
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
			char mnemonic[4];
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
				mnemonic[3] = '\0';
			}
		};
		
		template<bool WithCarry>
		static OperationOutput Add(word arg1, word arg2, int carryIn){
			uint64_t ulongResult = (uint64_t)arg1 + (uint64_t)arg2 + (WithCarry ? carryIn : 0);
			int64_t slongResult = TURNIPEMU_UINT32_TO_SINT64(arg1) + TURNIPEMU_UINT32_TO_SINT64(arg2) + (WithCarry ? carryIn : 0);
			OperationOutput output(
				word(ulongResult),
				(ulongResult >> 32) & 1,
				(slongResult > std::numeric_limits<int32_t>::max()) || (slongResult < std::numeric_limits<int32_t>::lowest())
				);
			return output;
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
				data.operand2.registerValue.writeDescription(stream);
			}
			stream << "\nDestination Register: " << (int)data.destinationRegister;
			return stream.str();
		}
		void execute(CPU& cpu, RegisterPointers currentRegisters, word instructionWord) override {
			InstructionData data(instructionWord);
			if (data.restoreSPSR){
				if (currentRegisters.cpsr->mode == Mode::User) return;
				throw std::runtime_error("Implement SPSR restoration!");
			}
			
			const Operation& operation = operations[data.opcode];

			uint8_t pcOffset = 0;
			if (data.operand1Register == 15 || (!data.operand2.useImmediate && data.operand2.registerValue.baseRegister == 15)){
				if (!data.operand2.useImmediate && data.operand2.registerValue.shiftedByRegister) 
					pcOffset = 4;
				else
					pcOffset = 0;
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
}
