#include "turnipemu/gba/dma.h"

namespace TurnipEmu::GBA{
	DMAEngine::DMAEngine()
		: Memory::RangeController(0x0'0400'00B0,  0x0'0400'00E0) {
	}
	
	void DMAEngine::execute(Memory::Map& memoryMap){
		throw std::runtime_error("DMA has not been implemented!");
	}
	bool DMAEngine::canExecute(){
		// TODO: Should this check if certain DMAs have actually been requested?
		// Or should that be left to execute()?
		return channels[0].enabled() || channels[1].enabled() || channels[2].enabled() || channels[3].enabled();
	}
	
	bool DMAEngine::allowRead(uint32_t address) const {
		const uint32_t deltaAddress = (address - 0x0'0400'00B0);
		const uint32_t byteIndex = deltaAddress % 12;
		return byteIndex == 10 || byteIndex == 11;
	}
	byte DMAEngine::read(uint32_t address) const {
		const uint32_t deltaAddress = (address - 0x0'0400'00B0);
		const uint32_t channelIndex = deltaAddress / 12;
		const uint32_t byteIndex = deltaAddress % 12;
		assert(channelIndex < 4);
		return channels[channelIndex].externalState.data[byteIndex];
	}
	bool DMAEngine::allowWrite(uint32_t address) const {
		return true;
	}
	void DMAEngine::write(uint32_t address, byte value) {
		const uint32_t deltaAddress = (address - 0x0'0400'00B0);
		const uint32_t channelIndex = deltaAddress / 12;
		const uint32_t byteIndex = deltaAddress % 12;
		assert(channelIndex < 4);
		channels[channelIndex].externalState.data[byteIndex] = value;
	}
}
