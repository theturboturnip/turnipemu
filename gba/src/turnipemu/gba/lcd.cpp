#include "turnipemu/gba/lcd.h"

namespace TurnipEmu::GBA{
	bool LCDEngine::ownsAddress(uint32_t address) const {
		return (0x0'0400'0000 <= address && address < 0x0'0400'0060) ||
			(0x0'0500'0000 <= address && address < 0x0'0500'0400) ||
			(0x0'0600'0000 <= address && address < 0x0'0601'8000) ||
			(0x0'0700'0000 <= address && address < 0x0'0700'0400);
	}
	bool LCDEngine::allowRead(uint32_t address) const {
		return true; // TODO: This probably isn't right
	}
	byte LCDEngine::read(uint32_t address) const {
		if (0x0'0400'0000 <= address && address < 0x0'0400'0060)
			return io.data[address - 0x0'0400'0000];
		else if (0x0'0500'0000 <= address && address < 0x0'0500'0400)
			return paletteRam.data[address - 0x0'0500'0000];
		else if (0x0'0600'0000 <= address && address < 0x0'0601'8000)
			return vram.data[address - 0x0'0600'0000];
		else if (0x0'0700'0000 <= address && address < 0x0'0700'0400)
			return objectAttributes.data[address - 0x0'0700'0000];
		assert(false);
		return 0x0;
	}
	bool LCDEngine::allowWrite(uint32_t address) const {
		return true; // TODO: This DEFINITELY isn't right
	}
	void LCDEngine::write(uint32_t address, byte value) {
		if (address == 0x0'0400'0004){
			value = (value & statusFirstByteWriteMask) | (io.status.data[0] & ~statusFirstByteWriteMask);
			assert(ownedAddressToByte(address) == &io.status.data[0]);
		}
		*ownedAddressToByte(address) = value;
	}
	byte* LCDEngine::ownedAddressToByte(uint32_t address){
		if (0x0'0400'0000 <= address && address < 0x0'0400'0060)
			return &io.data[address - 0x0'0400'0000];
		else if (0x0'0500'0000 <= address && address < 0x0'0500'0400)
			return &paletteRam.data[address - 0x0'0500'0000];
		else if (0x0'0600'0000 <= address && address < 0x0'0601'8000)
			return &vram.data[address - 0x0'0600'0000];
		else if (0x0'0700'0000 <= address && address < 0x0'0700'0400)
			return &objectAttributes.data[address - 0x0'0700'0000];
		assert(false);
		return nullptr;
	}
}
