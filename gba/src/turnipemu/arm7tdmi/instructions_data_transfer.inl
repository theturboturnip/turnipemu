namespace TurnipEmu::ARM7TDMI {
	class SingleDataTransferInstruction : public Instruction {
		using Instruction::Instruction;

		struct TransferSize {
			constexpr static bool Byte = true;
			constexpr static bool Word = false;
		};
		struct InstructionData {
			DataTransferInfo transferInfo;
			uint8_t addressRegister : 4;
			
			bool useImmediateOffset;
			union {
				int immediateValue;
				ShiftedRegister registerValue;
			} offset;

			bool transferSize;

			InstructionData(word instructionWord) : transferInfo(instructionWord) {
				addressRegister = (instructionWord >> 12) & 0xF;
				useImmediateOffset = (instructionWord >> 25) & 1;
				if (useImmediateOffset){
					offset.immediateValue = (instructionWord & 0xFFF) * (transferInfo.offsetSign);
				}else{
					offset.registerValue = ShiftedRegister(instructionWord);
					assert(!offset.registerValue.shiftedByRegister);
				}

				transferSize = (instructionWord >> 22) & 1;
			}
		};

		std::string disassembly(word instructionWord) override {
			InstructionData data(instructionWord);
			
			std::stringstream stream;
			if (data.transferSize == TransferSize::Byte){
				stream << "Byte ";
			}else{
				stream << "Word ";
			}
			if (data.transferInfo.transferMode == DataTransferInfo::TransferMode::Load){
				stream << "Load\n"; 
			}else{
				stream << "Store\n";
			}
			stream << "Address Register: " << (int)data.addressRegister << "\n";
			stream << "Data Register: " << (int)data.transferInfo.baseRegister << "\n";
			stream << "Offset: ";
			if (data.useImmediateOffset){
				stream << "Immediate Value " << (int)data.offset.immediateValue;
			}else{
				data.offset.registerValue.writeDescription(stream);
			}
			stream << "\nWriteback: " << std::boolalpha << data.transferInfo.writeback;
			stream << "\nIndexing: " << ((data.transferInfo.indexMode == DataTransferInfo::IndexMode::PreIndex) ? "Pre" : "Post");
			return stream.str();
		}
	};
}
