#include "turnipemu/gba/sound.h"

namespace TurnipEmu::GBA{
	SoundEngine::SoundEngine()
		: Memory::RangeController(0x0'0400'0060, 0x0'0400'00B0) {
	}
	
	const std::vector<uint8_t>& SoundEngine::generateSamples(size_t count){
		throw std::runtime_error("Sound has not been implemented!");
	}
	
	bool SoundEngine::allowRead(uint32_t address) const {
		// TODO: FIFO shouldn't be readable
		return true;
	}
	byte SoundEngine::read(uint32_t address) const {
		const uint32_t deltaAddress = (address - 0x0'0400'0060);

		if (deltaAddress < 0x20){
			const uint32_t channelIndex = deltaAddress / 8; // This could end up being > 4, in which case the
			const uint32_t byteIndex = deltaAddress % 8;
			switch(channelIndex){
			case 0:
				return channel1.data[byteIndex];
			case 1:
				return channel2.data[byteIndex];
			case 2:
				return channel3.data[byteIndex];
			case 3:
				return channel4.data[byteIndex];
			}
		} else if (deltaAddress < 0x2B) {
			return control.data[deltaAddress - 0x20];
		}

		// TODO: Read from wave RAM
		return 0x0;
	}
	bool SoundEngine::allowWrite(uint32_t address) const {
		return true;
	}
	void SoundEngine::write(uint32_t address, byte value) {
		const uint32_t deltaAddress = (address - 0x0'0400'0060);

		if (deltaAddress < 0x20){
			const uint32_t channelIndex = deltaAddress / 8; // This could end up being > 4, in which case the
			const uint32_t byteIndex = deltaAddress % 8;
			switch(channelIndex){
			case 0:
				channel1.data[byteIndex] = value;
				break;
			case 1:
				channel2.data[byteIndex] = value;
				break;
			case 2:
				channel3.data[byteIndex] = value;
				break;
			case 3:
				channel4.data[byteIndex] = value;
				break;
			}
		} else if (deltaAddress < 0x2B) {
			control.data[deltaAddress - 0x20] = value;
		}

		// TODO: Write to wave RAM/FIFO
	}
}
