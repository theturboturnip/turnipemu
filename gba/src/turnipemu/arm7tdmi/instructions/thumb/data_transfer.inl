#include "turnipemu/arm7tdmi/instruction_category.h"
#include "turnipemu/utils.h"
#include "turnipemu/log.h"

namespace TurnipEmu::ARM7TDMI::Instructions::Thumb {
	class LoadStoreHalfwordInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;

		enum class TransferMode : bool {
			Load = true,
			Store = false
		};
		struct InstructionData {
			TransferMode transferMode;
			
			uint8_t immediateValue : 5;

			union {
				uint8_t sourceRegister : 3;
				uint8_t destinationRegister : 3;
			};
			uint8_t baseRegister : 3;
			
			InstructionData(halfword instruction){
				transferMode = static_cast<TransferMode>((instruction >> 11) & 1);

				immediateValue = (instruction >> 6) & 0b11111;
				sourceRegister = (instruction >> 0) & 0b111;
				baseRegister = (instruction >> 3) & 0b111;
			}
		};

	public:
		std::string disassembly(halfword instruction) const override {
			InstructionData data(instruction);

			std::stringstream os;
			bool loading = (data.transferMode == TransferMode::Load);
			os << (loading ? "Load halfword from " : "Store halfword to ");
			os << "Register " << (int)data.baseRegister << " + " << (int)data.immediateValue;
			os << (loading ? " to " : " from ");
			os << "Register " << (int)data.destinationRegister;
			return os.str();
		}
		void execute(CPU& cpu, InstructionRegisterInterface registers, halfword instruction) const override {
			InstructionData data(instruction);

			word address = registers.get(data.baseRegister) + data.immediateValue;
			if (data.transferMode == TransferMode::Load){
				registers.set(data.destinationRegister, cpu.memoryMap.read<halfword>(address).value());
			}else{
				cpu.memoryMap.write<halfword>(address, registers.get(data.sourceRegister));
			}
		}
	};

	class LoadPCRelativeInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;

		struct InstructionData {
			uint16_t immediateValue : 10;
			uint8_t destinationRegister : 3;
			
			InstructionData(halfword instruction){
				immediateValue = (instruction & 0xFF) << 2;
				destinationRegister = (instruction >> 8) & 0b111;
			}
		};

	public:
		std::string disassembly(halfword instruction) const override {
			InstructionData data(instruction);

			return Utils::streamFormat(
				"Load from PC + ",
				(int)data.immediateValue,
				" into Register ",
				(int)data.destinationRegister,
				". The PC is forced to be word-aligned by zeroing bit 0."
				);
		}
		void execute(CPU& cpu, InstructionRegisterInterface registers, halfword instruction) const override {
			InstructionData data(instruction);

			word address = registers.get(registers.PC);
			address = address & (word(~0) << 2); // Set bits 0 and 1 to 0, to make sure it's a multiple of 4
			address += data.immediateValue;

			if (auto dataOptional = cpu.memoryMap.read<word>(address))
				registers.set(data.destinationRegister, dataOptional.value());
		}
	};

	class LoadStoreSPRelativeInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;

		enum class TransferMode : bool {
			Load = true,
			Store = false
		};			
		struct InstructionData {
			uint16_t immediateValue : 10;

			union {
				uint8_t sourceRegister : 3;
				uint8_t destinationRegister : 3;
			};
			
			TransferMode transferMode;
			
			InstructionData(halfword instruction){
				immediateValue = (instruction & 0xFF) << 2;
				destinationRegister = (instruction >> 8) & 0b111;

				transferMode = static_cast<TransferMode>((instruction >> 11) & 1);
			}
		};

	public:
		std::string disassembly(halfword instruction) const override {
			InstructionData data(instruction);

			std::stringstream os;
			bool loading = (data.transferMode == TransferMode::Load);
			os << (loading ? "Load from " : "Store to ");
			os << "SP + " << Utils::HexFormat(data.immediateValue);
			os << (loading ? " to " : " from ");
			os << "Register " << (int)data.destinationRegister;
			return os.str();
		}
		void execute(CPU& cpu, InstructionRegisterInterface registers, halfword instruction) const override {
			InstructionData data(instruction);

			word address = registers.get(registers.SP) + data.immediateValue;
			if (data.transferMode == TransferMode::Load){
				registers.set(data.destinationRegister, cpu.memoryMap.read<word>(address).value());
			}else{
				cpu.memoryMap.write<word>(address, registers.get(data.sourceRegister));
			}
		}
	};

	class LoadStoreRegisterOffsetInstruction : public InstructionCategory {
		using InstructionCategory::InstructionCategory;

		enum class TransferMode : bool {
			Load = true,
			Store = false
		};
		enum class TransferSize : uint8_t {
			Byte = 1,
			HalfWord = 2,
			Word = 4
		};
		struct InstructionData {
			TransferMode transferMode;
			TransferSize transferSize;
			bool signExtended;

			uint8_t baseRegister : 3;
			uint8_t offsetRegister : 3;
			union {
				uint8_t destinationRegister : 3;
				uint8_t sourceRegister : 3;
			};
				
			InstructionData(halfword instruction){
				baseRegister = (instruction >> 3) & 0b111;
				offsetRegister = (instruction >> 6) & 0b111;
				destinationRegister = (instruction >> 0) & 0b111;

				if ((instruction >> 9) & 1){
					// Instruction is Format 8
					switch ((instruction >> 10) & 0b11){
					case 0b00:
						transferMode = TransferMode::Store;
						transferSize = TransferSize::HalfWord;
						signExtended = false;
						break;
					case 0b01:
						transferMode = TransferMode::Load;
						transferSize = TransferSize::HalfWord;
						signExtended = false;
						break;
					case 0b10:
						transferMode = TransferMode::Load;
						transferSize = TransferSize::Byte;
						signExtended = true;
						break;
					case 0b11:
						transferMode = TransferMode::Load;
						transferSize = TransferSize::HalfWord;
						signExtended = true;
						break;
					default:
						assert(false); // Something went *REALLY* wrong here
					}
				}else{
					// Instruction is Format 7
					transferMode = static_cast<TransferMode>(((instruction >> 11) & 1) == 1);
					transferSize = ((instruction >> 10) & 1) ? TransferSize::Byte : TransferSize::Word;
					signExtended = false;
				}
			}
		};
		
	public:
		std::string disassembly(halfword instruction) const override {
			InstructionData data(instruction);

			std::stringstream os;
			bool loading = (data.transferMode == TransferMode::Load);
			os << (loading ? "Load " : "Store ");
			os << (data.signExtended ? "sign-extended " : "");
			switch (data.transferSize) {
			case TransferSize::Byte:
				os << "byte";
				break;
			case TransferSize::HalfWord:
				os << "halfword";
				break;
			case TransferSize::Word:
				os << "word";
				break;
			}
			os << (loading ? " from " : " to ");
			os << "address at [Register " << (int)data.baseRegister << " + Register " << (int)data.offsetRegister << "]";
			os << (loading ? " to " : " from ");
			os << "Register " << (int)data.destinationRegister;

			return os.str();
		}
		void execute(CPU& cpu, InstructionRegisterInterface registers, halfword instruction) const override {
			InstructionData data(instruction);

			word address = registers.get(data.baseRegister) + registers.get(data.offsetRegister);
			if (data.transferMode == TransferMode::Load){
				word loadedValue = 0;
				switch (data.transferSize) {
				case TransferSize::Byte:
					loadedValue = cpu.memoryMap.read<byte>(address).value();
					if (data.signExtended) loadedValue = loadedValue | (-((loadedValue >> 7) & 1));
					break;
				case TransferSize::HalfWord:
					loadedValue = cpu.memoryMap.read<halfword>(address).value();
					if (data.signExtended) loadedValue = loadedValue | (-((loadedValue >> 15) & 1));
					break;
				case TransferSize::Word:
					loadedValue = cpu.memoryMap.read<word>(address).value();
					assert(!data.signExtended);
					break;
				}

				registers.set(data.destinationRegister, loadedValue);
			}else{
				assert(!data.signExtended);

				word value = registers.get(data.sourceRegister);
				switch (data.transferSize) {
				case TransferSize::Byte:
					cpu.memoryMap.write<byte>(address, value);
					break;
				case TransferSize::HalfWord:
					cpu.memoryMap.write<halfword>(address, value);
					break;
				case TransferSize::Word:
					cpu.memoryMap.write<word>(address, value);
					break;
				}
			}
		}
	};
}
