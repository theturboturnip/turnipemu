namespace TurnipEmu::ARM7TDMI {
	class SingleDataTransferInstruction : public ARMInstructionCategory {
		using ARMInstructionCategory::ARMInstructionCategory;

		struct TransferSize {
			constexpr static bool Byte = true;
			constexpr static bool Word = false;
		};
		// The official ARM7TDMI manual says an immediate value is always used.
		// However, the older data sheet says a shifted register can be used.
		struct InstructionData : public DataTransferInfo{
			uint8_t dataRegister : 4;
			
			bool useImmediateOffset;
			union {
				word immediateValue;
				ShiftedRegister registerValue;
			} offset;

			bool transferSize;

			InstructionData(word instructionWord) : DataTransferInfo(instructionWord) {
				dataRegister = (instructionWord >> 12) & 0xF;
				useImmediateOffset = ((instructionWord >> 25) & 1) == 0;
				if (useImmediateOffset){
					offset.immediateValue = (instructionWord & 0xFFF);
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
			if (data.transferMode == DataTransferInfo::TransferMode::Load){
				stream << "Load\n"; 
			}else{
				stream << "Store\n";
			}
			stream << "Address Register: " << (int)data.addressRegister << "\n";
			stream << "Data Register: " << (int)data.dataRegister << "\n";
			stream << "Offset: ";
			if (data.useImmediateOffset){
				stream << "Immediate Value " << (int)data.offset.immediateValue;
			}else{
				data.offset.registerValue.writeDescription(stream);
			}
			stream << " * " << data.offsetSign;
			stream << "\nWriteback: " << std::boolalpha << data.writeback;
			stream << "\nIndexing: " << ((data.indexMode == DataTransferInfo::IndexMode::PreIndex) ? "Pre" : "Post");
			return stream.str();
		}
		void execute(CPU& cpu, const RegisterPointers registers, word instructionWord) override {
			InstructionData data(instructionWord);
			
			int finalOffset = (data.useImmediateOffset ? data.offset.immediateValue : data.offset.registerValue.calculateValue(registers, true)) * data.offsetSign;
			word address = (*registers.main[data.addressRegister]) + finalOffset;
			if (data.transferMode == DataTransferInfo::TransferMode::Load){
				if (data.transferSize == TransferSize::Byte){
					auto optionalByte = cpu.memoryMap.read<byte>(address);
					if (optionalByte)
						*registers.main[data.dataRegister] = optionalByte.value();
				}else{
					auto optionalWord = cpu.memoryMap.read<word>(address);
					if (optionalWord)
						*registers.main[data.dataRegister] = optionalWord.value();
				}
			}else{
				if (data.transferSize == TransferSize::Byte){
					byte value = (*registers.main[data.dataRegister]) & 0xFF;
					if (!cpu.memoryMap.write<byte>(address, value)){
						// Memory Exception?
					}
				}else{
					word value = *registers.main[data.dataRegister];
					if (!cpu.memoryMap.write<word>(address, value)){
						// Memory Exception?
					}
				}
			}
			// On the original ARM7, if post-indexing and writeback is used then the transfer is performed in user mode. However on the GBA the mode doesn't matter, so we ignore it here.
			if (data.writeback && (data.indexMode == DataTransferInfo::IndexMode::PreIndex)){
				*registers.main[data.addressRegister] = address;
			}
		}
	};
}
