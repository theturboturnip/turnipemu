namespace TurnipEmu::ARM7TDMI {
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
};
