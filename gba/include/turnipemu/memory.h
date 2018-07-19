#pragma once

#include "types.h"

#include <assert.h>
#include <vector>
#include <optional>

namespace TurnipEmu{
	class Emulator;

    // An interface for reading and writing bytes from memory. 
	class MemoryController{
	public:
		virtual bool ownsAddress(uint32_t address) const = 0;
	
		virtual bool allowRead(uint32_t address) const = 0;
		virtual byte read(uint32_t address) const = 0;
		virtual bool allowWrite(uint32_t address) const {
			return false;
		}
		virtual void write(uint32_t address, byte value){}

		virtual uint8_t cyclesForRead(uint32_t address, uint8_t byteWidth) const {
			switch(byteWidth){
			case 1:
			case 2:
			case 4:
				return 1;
			default:
				assert(false);
				return 0; // 0 is for an unsupported read width
			}
		}
		virtual uint8_t cyclesForWrite(uint32_t address, uint8_t byteWidth) const {
			switch(byteWidth){
			case 1:
			case 2:
			case 4:
				return 1;
			default:
				assert(false);
				return 0; // 0 is for an unsupported read width
			}
		}
	};
	class MemoryRangeController : public MemoryController{
	public:
		MemoryRangeController(uint32_t startAddress, uint32_t endAddress) :
			startAddress(startAddress), endAddress(endAddress) {}
		
		bool ownsAddress(uint32_t address) const override {
			return (startAddress <= address) && (address < endAddress);
		}
	protected:
		// If these are const then any MemoryRangeController subclass can't be copied
		uint32_t startAddress = 0;
		uint32_t endAddress = 0;
	};
	
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
