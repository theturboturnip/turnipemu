namespace TurnipEmu::ARM7TDMI {
	struct DataTransferInfo {
		struct IndexMode {
			constexpr static bool PreIndex = true;
			constexpr static bool PostIndex = false;
		};
		struct TransferMode {
			constexpr static bool Load = true;
			constexpr static bool Store = false;
		};
		bool indexMode;
		int offsetSign; // +1/-1
		bool writeback;
		bool transferMode;
		uint8_t baseRegister : 4;

		DataTransferInfo(word instructionWord){
			indexMode = ((instructionWord >> 24) & 1);
			offsetSign = ((instructionWord >> 23) & 1) ? +1 : -1;
			writeback = ((instructionWord >> 21) & 1);
			transferMode = ((instructionWord >> 20) & 1);
			baseRegister = (instructionWord >> 16);
		}
	};
	struct ShiftedRegister {
		enum class RegisterShiftType {
			LogicalShiftLeft = 0b00,
			LogicalShiftRight = 0b01,
			ArithmeticShiftRight = 0b10,
			RotateRight = 0b11
		};
		
		uint8_t baseRegister : 4;
		RegisterShiftType shiftType : 2;
		bool shiftedByRegister;
		union {
			uint8_t shiftRegister : 4;
			uint8_t shiftAmount : 5;
		};

		// Evaluating this can update flags in the registers, we want to make sure that only happens once
		bool hasChangedFlags;
		
		ShiftedRegister() = default;
		ShiftedRegister(word instructionWord) : hasChangedFlags(false){
			baseRegister = instructionWord & 0xF;
			shiftType = static_cast<RegisterShiftType>((instructionWord >> 5) & 0b11);
			shiftedByRegister = (instructionWord >> 4) & 1;
			if (shiftedByRegister){
				shiftRegister = (instructionWord >> 8) & 0xF;
				assert(shiftRegister != 15);
			}else{
				shiftAmount = (instructionWord >> 7) & 0b11111;
			}
		}

		word calculateValue(const RegisterPointers registers, bool allowChangeFlags){
			auto setCarryIfPossible = [&](bool set) {
				if (!allowChangeFlags) return;

				assert(!hasChangedFlags);
				registers.cpsr->carry = set;
				hasChangedFlags = true;
			};
			
			word baseValue = *registers.main[baseRegister];
			word baseShiftAmount = shiftedByRegister ? *registers.main[shiftRegister] :
				shiftAmount;
			uint16_t finalShiftAmount = baseShiftAmount & 0b11111;

			switch(shiftType){
			case ShiftedRegister::RegisterShiftType::LogicalShiftLeft:
				// If baseShiftAmount is a multiple of 32 then the carry should be set => don't use the final amount for the if statement
				if (baseShiftAmount != 0) setCarryIfPossible((baseValue >> (32 - finalShiftAmount)) & 1);
				return word(baseValue << finalShiftAmount);
			case ShiftedRegister::RegisterShiftType::LogicalShiftRight:
				if (finalShiftAmount == 0) finalShiftAmount = 32; 
				setCarryIfPossible((baseValue >> (finalShiftAmount - 1)) & 1);
				return word(baseValue >> finalShiftAmount);
			case ShiftedRegister::RegisterShiftType::ArithmeticShiftRight:
				if (finalShiftAmount == 0) finalShiftAmount = 32;
				setCarryIfPossible((baseValue >> (finalShiftAmount - 1)) & 1);
				return word( ((int32_t) baseValue) >> finalShiftAmount);
			case ShiftedRegister::RegisterShiftType::RotateRight:
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
		
		void writeDescription(std::stringstream& stream){
			stream << "Register " << (int)baseRegister;
			switch(shiftType){
			case RegisterShiftType::LogicalShiftLeft:
				stream << " logical shift left";
				break;
			case RegisterShiftType::LogicalShiftRight:
				stream << " logical shift right";
				break;
			case RegisterShiftType::ArithmeticShiftRight:
				stream << " arithmetic shift right";
				break;
			case RegisterShiftType::RotateRight:
				stream << " rotate right";
				break;
			}
			stream << " by ";
			if (shiftedByRegister){
				stream << "Register " << (int)shiftRegister;
			}else{
				stream << (int)shiftAmount;
			}
		}
	};
	// TODO: No longer common
	struct ALUOperand2 {
		bool useImmediate;
		union {
			struct {
				uint8_t baseImmediateValue;
				uint8_t rotation;
			} immediateValue;
			ShiftedRegister registerValue;
		};

		ALUOperand2(word instructionWord){
			useImmediate = (instructionWord >> 25) & 1;
			if (useImmediate){
				immediateValue.baseImmediateValue = instructionWord & 0xFF;
				immediateValue.rotation = ((instructionWord >> 8) & 0xF) * 2;
			}else{
				registerValue = ShiftedRegister(instructionWord);
			}
		}
		word calculateValue(const RegisterPointers registers, bool allowChangeFlags){
			if (useImmediate){
				return word(immediateValue.baseImmediateValue >> immediateValue.rotation) |
					word(immediateValue.baseImmediateValue << (32 - immediateValue.rotation));
			}else{
				return registerValue.calculateValue(registers, allowChangeFlags);
			}
		}
	};
};
