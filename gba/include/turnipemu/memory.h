#pragma once

#include "types.h"

#include <vector>

namespace GBA{
	class GBA;
}

// An interface for reading and writing bytes from memory. 
class MemoryController{
public:
	virtual bool allowRead(uint32_t address) = 0;
	virtual byte read(uint32_t address) = 0;
	virtual bool allowWrite(uint32_t address) = 0;
	virtual bool write(uint32_t address, byte value) = 0;

	virtual uint8_t cyclesForRead(uint8_t byteWidth){
		switch(width){
		case 1:
		case 2:
		case 4:
			return 1;
		default:
			assert(false);
			return 0; // 0 is for an unsupported read width
		}
	}
	virtual uint8_t cyclesForWrite(uint8_t byteWidth){
		switch(width){
		case 1:
		case 2:
		case 4:
			return 1;
		default:
			assert(false);
			return 0; // 0 is for an unsupported read width
		}
	}
}
	struct AddressableMemoryRange{
		const uint32_t startAddress;
		const uint32_t endAddress;
		const MemoryController* controller = nullptr;

		// Other implementations can override this, if they only control individual addresses across a large range
		virtual bool ownsAddress(uint32_t address) const{
			return (startAddress <= address) && (address < endAddress);
		}
	};
	
class MemoryMap{
	// Read/Write Pseudocode
	// 1. Check if there's a DMA transfer either TO or FROM this address. Disallow if so. (TODO: Allow writes when transferring from?)
	// 2. Check if the address is within an AddressableMemoryRange. Disallow if not.
	// 3. Check if the MemoryController for the AddressableMemoryRange allows the current op. Disallow if not.
	// 4. Do the op.
		
public:
	MemoryMap(GBA::GBA&); // Setup the AddressableMemoryRanges based on the current GBA implementation.

	// Instantiated for byte, halfword, word
	template<typename ReadType>
	ReadType read(uint32_t address) const;
	template<typename WriteType>
	void write(uint32_t address, WriteType value) const;
protected:
	MemoryController* controllerForAddress(uint32_t address) const; 
		
	const std::vector<AddressableMemoryRange> addressableMemoryRanges;
	// TODO: std::unordered_map<uint32_t, MemoryController*> could be used as a cache? Use FIFO to limit total space taken up?

	ARM7TDMI& cpu; // Memory reads take cycles, and affect the CPU registers.
};

