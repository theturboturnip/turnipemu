#include "turnipemu/memory/map.h"

#include "turnipemu/emulator.h"
#include "turnipemu/log.h"

namespace TurnipEmu::Memory{
	Map::Map(Emulator& emulator) : emulator(emulator){
	}
	
	void Map::registerMemoryController(Controller* memoryController){
		memoryControllers.push_back(memoryController);
	}

	Controller* Map::controllerForAddress(uint32_t address, bool accessByEmulator) const {
		for (auto* controller : memoryControllers){
			if (controller->ownsAddress(address)) return controller;
		}
		if (accessByEmulator){
			LogLine(logTag, "Invalid Memory Access at address 0x%08x, stopping", address);
			emulator.stopped = true;
		}
		return nullptr;
	}

	template<typename ReadType>
	std::optional<ReadType> Map::read(uint32_t address, bool accessByEmulator) const {
		assert(address % sizeof(ReadType) == 0); // Byte reads MUST be on byte-boundaries, halfwords on 2-byte boundaries, words on 4-byte boundaries.
		
		auto* controller = controllerForAddress(address, accessByEmulator);
		if (controller){
			ReadType value = 0;
			for (int i = 0; i < sizeof(ReadType); i++){
				if (controller->allowRead(address + i))
					value = value | controller->read(address + i) << (8 * i);
				else{
					LogLine(logTag, "Memory Read disallowed by controller");
					emulator.stopped = true;
				}
			}
			return {value};
		}else{
			return {};
		}
	}
	template std::optional<byte> Map::read<byte>(uint32_t, bool) const;
	template std::optional<halfword> Map::read<halfword>(uint32_t, bool) const;
	template std::optional<word> Map::read<word>(uint32_t, bool) const;

	template<typename WriteType>
	void Map::write(uint32_t address, WriteType value, bool accessByEmulator) const {
		assert(address % sizeof(WriteType) == 0); // Byte writes MUST be on byte-boundaries, halfwords on 2-byte boundaries, words on 4-byte boundaries.
		
		auto* controller = controllerForAddress(address, accessByEmulator);
		if (controller){
			for (int i = 0; i < sizeof(WriteType); i++){
				if (controller->allowWrite(address + i))
					controller->write(address + i, (value >> (8 * i)) & 0xFF);
				else{
					LogLine(logTag, "Memory Write disallowed by controller");
					emulator.stopped = true;
				}
			}
		}
	}
	template void Map::write<byte>(uint32_t, byte, bool) const;
	template void Map::write<halfword>(uint32_t, halfword, bool) const;
	template void Map::write<word>(uint32_t, word, bool) const;
}
