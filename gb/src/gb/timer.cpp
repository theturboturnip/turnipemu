#include "gb/timer.h"
#include "gb/cpu.h"

namespace GB{
	void Timer::step(){		
		divider_predivider += cpu.clock_cycles_this_step;
		while(divider_predivider > 4){
			divider++;
			divider_predivider -= 4;
		}

		if (!control.enabled) return;

		predivider += cpu.clock_cycles_this_step;
		int predivider_modulo;
		constexpr int modulo_for_16x = 8; // TODO: Is this 8 or 16?
		switch(control.speed){
		case Timer::Speed::Quarter:
			predivider_modulo = modulo_for_16x * (16 / 0.25);
			break;
		case Timer::Speed::x1:
			predivider_modulo = modulo_for_16x * (16 / 1);
			break;
		case Timer::Speed::x4:
			predivider_modulo = modulo_for_16x * (16 / 4);
			break;
		case Timer::Speed::x16:
			predivider_modulo = modulo_for_16x * (16 / 16);
			break;
		}
		while(predivider > predivider_modulo){
			if (counter == 0xFF){
				cpu.interrupts.trigger(Interrupt::Timer);
			}
			counter++;
			predivider -= predivider_modulo;
		}
	}

	void Timer::reset_divider(){
		divider = 0;
	}
	void Timer::write_counter(uint8_t new_value){
		counter = new_value;
	}
	void Timer::write_modulo(uint8_t new_value){
		modulo = new_value;
	}
	void Timer::write_control(uint8_t new_value){
		//fprintf(stdout, "Writing 0x%02x to Timer Control\n", new_value);
		control.enabled = new_value & (0b100);
		if (!control.enabled) predivider = 0; // TODO: This might be wrong?
		//fprintf(stdout, "Enabled: %d\n", control.enabled);
		control.speed = static_cast<Timer::Speed>(new_value & 0b11);
		//fprintf(stdout, "Speed: %d\n", static_cast<int>(control.speed));
	}

	void Timer::reset(){
		// TODO: Are there documented defaults for these?
		divider = 0;
		counter = 0;
		modulo = 0;
		predivider = 0;
		divider_predivider = 0;
		control.enabled = false;
		control.speed = Timer::Speed::x1;
	}
}
