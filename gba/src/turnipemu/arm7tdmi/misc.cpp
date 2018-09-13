#include "turnipemu/arm7tdmi/misc.h"

#include "turnipemu/arm7tdmi/cpu.h"
#include "turnipemu/arm7tdmi/instruction_category.h"

namespace TurnipEmu::ARM7TDMI::Instructions::MultipleLoadStore {
	std::ostream& operator << (std::ostream& stream, const InstructionData& data){
		stream << "Block ";
		if (data.genericInfo.transferMode == DataTransferInfo::TransferMode::Load){
			stream << "Load\n"; 
		}else{
			stream << "Store\n";
		}
		stream << "Registers:";
		for (int i = 0; i < 16; i++){
			if (data.registerList[i]){
				stream << " R" << i;
			}
		}
		stream << "\nAddress Register: " << (int)data.genericInfo.addressRegister;
		stream << "\nDirection: " << ((data.genericInfo.offsetSign == 1) ? "Increment" : "Decrement") << " from register";
		stream << "\nWriteback: " << std::boolalpha << data.genericInfo.writeback;
		stream << "\nIndexing: " << ((data.genericInfo.indexMode == DataTransferInfo::IndexMode::PreIndex) ? "Pre" : "Post");

		return stream;
	}
	
	void Execute(InstructionData data, CPU& cpu, InstructionRegisterInterface registers){
		const bool preindex = data.genericInfo.indexMode == DataTransferInfo::IndexMode::PreIndex;
			const bool load = data.genericInfo.transferMode == DataTransferInfo::TransferMode::Load;
			
			// As stated by the docs:
			// The registers must be read from lowest to highest (excluding the address)
			// The lowest register must be written to the lowest point
			word baseAddress = registers.get(data.genericInfo.addressRegister);
			word address = baseAddress;
			if (data.genericInfo.offsetSign < 0){
				address -= sizeof(word) * data.registerList.count();
				if (preindex) address -= sizeof(word);
			}

			for (int i = 0; i < 16; i++){
				if (!data.registerList[i]) continue;
				
				if (preindex) address += sizeof(word);

				if (load)
					registers.set(i, cpu.memoryMap.read<word>(address).value());
				else
					cpu.memoryMap.write<word>(address, registers.get(i));
					
				if (!preindex) address += sizeof(word);
			}

			if (data.genericInfo.writeback){
				registers.set(data.genericInfo.addressRegister, baseAddress + data.genericInfo.offsetSign * sizeof(word) * data.registerList.count());
			}
	}
}
