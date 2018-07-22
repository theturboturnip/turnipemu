#pragma once

#include "types.h"

#include <assert.h>
#include <memory>

namespace TurnipEmu{
    // An interface for reading and writing bytes from memory. 
	class MemoryController {
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

	class RangeMemoryController : public MemoryController {
	public:
		RangeMemoryController(uint32_t startAddress, uint32_t endAddress) :
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
	class StaticDataRangeMemoryController : public RangeMemoryController {
	public:
		static_assert(std::is_same<byte, typename StorageClass::value_type>::value);
		
		StaticDataRangeMemoryController(StorageClass data, uint32_t startAddress) : RangeMemoryController(startAddress, startAddress + data.size()), data(std::move(data)){
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
}
