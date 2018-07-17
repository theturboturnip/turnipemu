#include "memory.h"
#include "emulator.h"
#include "log.h"

namespace TurnipEmu{
	MemoryMap::MemoryMap(Emulator& emulator) : emulator(emulator){
	}
	
	void MemoryMap::registerMemoryController(MemoryController* memoryController){
		memoryControllers.push_back(memoryController);
	}

	MemoryController* MemoryMap::controllerForAddress(uint32_t address) const {
		for (auto* controller : memoryControllers){
			if (controller->ownsAddress(address)) return controller;
		}
		LogLine(logTag, "Invalid Memory Access at address 0x%08x, stopping", address);
		emulator.stopped = true;
		return nullptr;
	}
}
