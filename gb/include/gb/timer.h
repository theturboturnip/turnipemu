
#pragma once

#include <cstdint>

namespace GB{

	class CPU;
	
	class Timer{
	public:
		Timer(CPU& cpu) : cpu(cpu) {}
		
		void step();
		void reset();
		
		void reset_divider();
		void write_counter(uint8_t new_value);
		void write_modulo(uint8_t new_value);
		void write_control(uint8_t new_value);

		inline uint8_t read_divider(){
			return divider;
		}
		inline uint8_t read_counter(){
			return counter;
		}
		inline uint8_t read_modulo(){
			return modulo;
		}
		inline uint8_t read_control(){
			return (control.enabled << 2) | (static_cast<uint8_t>(control.speed));
		}

	protected:
		uint8_t divider;
		uint8_t counter;
		uint8_t modulo;

		int predivider;
		int divider_predivider;

		enum class Speed {
			Quarter = 0,
			x16 = 1,
			x4 = 2,
			x1 = 3
		};
		struct{
			bool enabled;
			Speed speed;
		} control;

		CPU& cpu;
	};
	
}
