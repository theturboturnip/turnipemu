#include "turnipemu/gba/sys_control.h"
#include "turnipemu/utils.h"

#include <cstring>

namespace TurnipEmu::GBA{  
	bool SystemControl::ownsAddress(uint32_t address) const {
		int ioDelta = address - 0x0'0400'0000;
		if (ioDelta < 0 || ioDelta > 0x0'00FF'FFFF) return false;
		return (0x0204 <= ioDelta && ioDelta < 0x0208) ||
			(0x0300 <= ioDelta && ioDelta < 0x0302) ||
			(ioDelta == 0x0410) || // Used in the BIOS, has no purpose(?)
			(ioDelta % 0x0800 < 4); // This byte is mirrored across the area in increments of 64K
	}
	bool SystemControl::allowRead(uint32_t address) const {
		return true;
	}
	byte SystemControl::read(uint32_t address) const {
		int ioDelta = address - 0x0'0400'0000;
		if (0x0204 <= ioDelta && ioDelta < 0x0208){
			return waitstateControl.data[ioDelta - 0x0204];
		}else if (ioDelta == 0x0300){
			return postBootFlag.data;
		}else if (ioDelta == 0x0301){
			return haltControl.data;
		}else if (ioDelta == 0x0410){
			return 0x0;
		}else if (ioDelta % 0x0800 < 4){
			return internalMemControl.data[ioDelta % 0x0800];
		}
		assert(false);
		return 0x0;
	}
	bool SystemControl::allowWrite(uint32_t address) const {
		return true;
	}
	void SystemControl::write(uint32_t address, byte value) {
		int ioDelta = address - 0x0'0400'0000;
		if (0x0204 <= ioDelta && ioDelta < 0x0208){
			waitstateControl.data[ioDelta - 0x0204] = value;
		}else if (ioDelta == 0x0300){
			postBootFlag.data = value;
		}else if (ioDelta == 0x0301){
			haltControl.data = value;
			throw std::runtime_error(Utils::streamFormat("Halt register was written to, should be implemented!"));
		}else if (ioDelta == 0x0410){
			return;
		}else if (ioDelta % 0x0800 < 4){
			internalMemControl.data[ioDelta % 0x0800] = value;
		}else{
			assert(false);
		}
	}

	void SystemControl::reset(){
		memset(&waitstateControl, 0, sizeof(waitstateControl));
		
		// Little-endian version of 0x0D00'0020
		internalMemControl.data[0] = 0x20;
		internalMemControl.data[1] = 0x00;
		internalMemControl.data[2] = 0x00;
		internalMemControl.data[3] = 0xD0;
		
		postBootFlag.data = 0;
		haltControl.data = 0;
	}
}
