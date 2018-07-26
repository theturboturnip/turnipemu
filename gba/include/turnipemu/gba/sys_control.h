#pragma once

#include "turnipemu/memory/controllers.h"
#include "turnipemu/types.h"

namespace TurnipEmu::GBA{
	class SystemControl : public Memory::Controller {
	public:		
		bool ownsAddress(uint32_t address) const override;
		bool allowRead(uint32_t address) const override;
		byte read(uint32_t address) const override;
		bool allowWrite(uint32_t address) const override;
		void write(uint32_t address, byte value) override;
		
		void reset();
	protected:
#pragma pack(0)
		// Access cycle data through public functions, not 
		union WaitstateControl {
			byte data[4];
			struct {
				uint8_t sramWait : 2; // 0..3 = 4,3,2,8 cycles
				uint8_t ws0FirstAccess : 2; // 0..3 = 4,3,2,8 cycles
				uint8_t ws0SecondAccess : 1; // 0 = 2, 1 = 1 cycles
				uint8_t ws1FirstAccess : 2; // 0..3 = 4,3,2,8 cycles
				uint8_t ws1SecondAccess : 1; // 0 = 4, 1 = 1 cycles
				
				uint8_t ws2FirstAccess : 2; // 0..3 = 4,3,2,8 cycles
				uint8_t ws2SecondAccess : 1; // 0 = 8, 1 = 1 cycles
				uint8_t phiTerminalOutput : 2; // 0..3 = disable, 4.19MHz, 8.38MHz, 16.78MHz
				bool dummy : 1;
				bool enablePrefetch : 1;
				bool gamePakIsCGB : 1; // Always false for our purposes

				uint16_t dummy1;
			};
		} waitstateControl;


		union InternalMemControl {
			byte data[4];
			struct {
				bool disableAllWRAM : 1; // (when off: empty/prefetch) <= what does this mean?
				uint8_t dummy : 4;
				bool enable256WRAM : 1; // (when off, the 256k WRAM mirrors the 32K WRAM)
				uint32_t dummy1 : 18;
				uint8_t wram256WaitState : 4; // wait timing for 256K WRAM, 8/16 bit access is 2 + (15 - N) cycles, 32 bit is equal to 2x16 bit
				uint8_t dummy2 : 4;
			};
		} internalMemControl;

		union PostBootFlag {
			byte data;
			struct {
				bool notFirstBoot : 1;
				uint8_t unused : 7;
			};
		} postBootFlag;

		// GBA powers down on write to this register
		union HaltControl {
			byte data;
			struct {
				uint8_t unused : 7;
				uint8_t powerDownMode : 1; // 0 = HALT (Soft stop, sound + video continue, CPU paused until interrupt), 1 = STOP (Hard stop, sound + video are stopped, CPU paused as before) 
			};
		} haltControl;
#pragma pack()
	};
}
