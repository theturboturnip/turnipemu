#pragma once

#include <bitset>
#include <ostream>

#include "turnipemu/types.h"

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
		uint8_t addressRegister : 4;

		DataTransferInfo(word instructionWord){
			indexMode = ((instructionWord >> 24) & 1);
			offsetSign = ((instructionWord >> 23) & 1) ? +1 : -1;
			writeback = ((instructionWord >> 21) & 1);
			transferMode = ((instructionWord >> 20) & 1);
			addressRegister = (instructionWord >> 16) & 0xF;
		}
	};

	class CPU;
	class InstructionRegisterInterface;
}

namespace TurnipEmu::ARM7TDMI::Instructions::MultipleLoadStore {
	struct InstructionData {
		DataTransferInfo genericInfo;
		std::bitset<16> registerList;

		InstructionData()
			: genericInfo(0), registerList(0) {}

		friend std::ostream& operator << (std::ostream&, const InstructionData&);
	};
	void Execute(InstructionData data, CPU& cpu, InstructionRegisterInterface registers);
}
