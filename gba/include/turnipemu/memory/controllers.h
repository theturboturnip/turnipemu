#pragma once

#include <assert.h>
#include <memory>

#include "turnipemu/types.h"

namespace TurnipEmu::Memory{
    // An interface for reading and writing bytes from memory. 
	class Controller {
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

	class RangeController : public Controller {
	public:
		RangeController(uint32_t startAddress, uint32_t endAddress) :
			startAddress(startAddress), endAddress(endAddress) {}
		
		bool ownsAddress(uint32_t address) const override {
			return (startAddress <= address) && (address < endAddress);
		}
	protected:
		// If these are const then any MemoryRangeController subclass can't be copied
		uint32_t startAddress = 0;
		uint32_t endAddress = 0;
	};

	template<typename StorageClass>
	class StaticDataRangeController : public RangeController {
	public:
		static_assert(std::is_same<byte, typename StorageClass::value_type>::value);
		
		StaticDataRangeController(StorageClass data, uint32_t startAddress) : RangeController(startAddress, startAddress + data.size()), data(std::move(data)){
		}

		bool allowRead(uint32_t address) const override {
			return true;
		}
		byte read(uint32_t address) const override {
			return data[address - startAddress];
		}
	protected:
		StorageClass data;
	};

	// This is for storing parts of memory that have no purpose.
	// Reads will return a constant value. Writes are always allowed, but have no effect.
	class NoopController : public Controller {
	public:
		NoopController(byte valueOnRead = 0xFF)
			: valueOnRead(valueOnRead) {
		}
		
		bool allowRead(uint32_t address) const override {
			return true;
		}
		byte read(uint32_t address) const override {
			// TODO: Warn the user on a read from Noop'd memory
			return valueOnRead;
		}
		bool allowWrite(uint32_t address) const override {
			return true;
		}
		void write(uint32_t address, byte value) override {}
	private:
		uint32_t valueOnRead;
	};
}
