#pragma once

#include "turnipemu/types.h"
#include "turnipemu/memory/controllers.h"

namespace TurnipEmu::GBA{
	class InterruptControl;
	
	class TimerEngine : public Memory::RangeController {
	public:
		TimerEngine();

		void execute(InterruptControl&);
		bool canExecute();
		
		bool allowRead(uint32_t address) const override;
		byte read(uint32_t address) const override;
		bool allowWrite(uint32_t address) const override;
		void write(uint32_t address, byte value) override;

	private:
#pragma pack(1)
		struct TimerData {
			union {
				byte data[2];
				struct {
					uint8_t prescalerSelection : 2; // Bit 0-1, 0=Normal Freq. 1=Freq./64 2=Freq./256 3= Freq./1024
					bool countUp : 1; // Bit 2, 0=Normal Behaviour, 1=Only increment once the previous one overflows. Disabled for Timer 0
					uint8_t dummy : 3; // Bits 3-5
					bool irqOnOverflow : 1; // Bit 6
					bool enable : 1; // Bit 7
					uint8_t dummy1 : 8; // Bits 8-15
				};
			} control;
			halfword counter = 0;
			halfword reload; // The counter is reset to this value 1) on overflow 2) when control.enable is set to true

			inline bool enabled(){
				return control.enable;
			}
			
			TimerData() {
				control.enable = false;
			}
		};

		std::array<TimerData, 4> timers;
#pragma pack()
	};
}
