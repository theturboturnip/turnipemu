#pragma once

#include "types.h"

#include <assert.h>
#include <vector>

namespace GBA{
	class GBA;
}
class ARM7TDMI;

// An interface for reading and writing bytes from memory. 
class MemoryController{
public:
	virtual bool ownsAddress(uint32_t address) const = 0;
	
	virtual bool allowRead(uint32_t address) const = 0;
	virtual GBA::byte read(uint32_t address) const = 0;
	virtual bool allowWrite(uint32_t address) const {
		return false;
	}
	virtual void write(uint32_t address, GBA::byte value){}

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
	const uint32_t startAddress = 0;
	const uint32_t endAddress = 0;

	bool ownsAddress(uint32_t address) const override {
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
	MemoryMap(GBA::GBA&); // Setup the MemoryControllers based on the current GBA implementation.

	// Instantiated for byte, halfword, word
	template<typename ReadType>
	ReadType read(uint32_t address) const;
	template<typename WriteType>
	void write(uint32_t address, WriteType value) const;
protected:
	MemoryController* controllerForAddress(uint32_t address) const; 
		
	const std::vector<MemoryController*> memoryControllers;
	// TODO: std::unordered_map<uint32_t, MemoryController*> could be used as a cache? Use FIFO to limit total space taken up?

	ARM7TDMI& cpu; // Memory reads take cycles, and affect the CPU registers.
};

