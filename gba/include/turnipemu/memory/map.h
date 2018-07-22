#pragma once

#include "types.h"
#include "controllers.h"

#include <assert.h>
#include <vector>
#include <optional>

namespace TurnipEmu{
	class Emulator;
	
	class MemoryMap {
		// Read/Write Pseudocode
		// 1. Check if there's a DMA transfer either TO or FROM this address. Disallow if so. (TODO: Allow writes when transferring from?)
		// 2. Check if the address is within an AddressableMemoryRange. Disallow if not.
		// 3. Check if the MemoryController for the AddressableMemoryRange allows the current op. Disallow if not.
		// 4. Do the op.
		
	public:
		MemoryMap(Emulator& emulator);
		void registerMemoryController(MemoryController* memoryController);

		void beginTick();
		void endTick();
		
		// Instantiated for byte, halfword, word
		template<typename ReadType>
		std::optional<ReadType> read(uint32_t address, bool accessByEmulator = true) const;
		template<typename WriteType>
		void write(uint32_t address, WriteType value, bool accessByEmulator = true) const;
	protected:
		MemoryController* controllerForAddress(uint32_t address, bool accessByEmulator) const;

		std::vector<MemoryController*> memoryControllers;
		// TODO: std::unordered_map<uint32_t, MemoryController*> could be used as a cache? Use FIFO to limit total space taken up?

		Emulator& emulator;
		
		// TODO: Remember to make this happen still, just without using a ref here
		// ARM7TDMI& cpu; // Memory reads take cycles, and affect the CPU registers.
		uint8_t memoryAccessCyclesThisTick;
		
		const char* const logTag = "MEM";
	};
}
