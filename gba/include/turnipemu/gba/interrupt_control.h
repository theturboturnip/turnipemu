#pragma once

#include "turnipemu/memory/controllers.h"
#include "turnipemu/types.h"

namespace TurnipEmu::GBA{
	class InterruptControl : public Memory::Controller {
	public:
		bool ownsAddress(uint32_t address) const override;
		bool allowRead(uint32_t address) const override;
		byte read(uint32_t address) const override;
		bool allowWrite(uint32_t address) const override;
		void write(uint32_t address, byte value) override;

		void tick(); // TODO: Implement
		
		void reset();
	protected:
#pragma pack(1)
		union InterruptRegister {
			byte data[2];
			struct {
				bool lcdVblank : 1;
				bool lcdHblank : 1;
				bool lcdVcountMatch : 1;
				bool timer0Overflow : 1;
				bool timer1Overflow : 1;
				bool timer2Overflow : 1;
				bool timer3Overflow : 1;
				bool serial : 1;
				bool dma0 : 1;
				bool dma1 : 1;
				bool dma2 : 1;
				bool dma3 : 1;
				bool keypad : 1;
				bool external : 1;
				bool dummy : 2;
			};
		};

		union InterruptMasterEnable {
			byte data[4];
			struct {
				bool enabled : 1;
				uint32_t dummy : 31;
			};
		} interruptsMasterEnable;
#pragma pack()

		InterruptRegister enabledInterrupts;
		InterruptRegister flaggedInterrupts;
	};
}
