#pragma once

#include <cstdint>

namespace GB{
	class CPU;

	union InputData{
		struct {
			// All 0 for pressed
			bool direction_right : 1;
			bool direction_left : 1;
			bool direction_up : 1;
			bool direction_down : 1;

			bool allow_buttons : 1;
			bool allow_directions : 1;
			uint8_t dummy : 2; // Bits 6/7
		};
		struct {
			// All false for pressed
			bool button_a : 1;
			bool button_b : 1;
			bool button_select : 1;
			bool button_start : 1;

			uint8_t dummy_2 : 4; // Bits 4/5/6/7 (4/5 are handled in the other struct)
		};
	};
	static_assert(sizeof(InputData) == 1, "InputData must be convertible to byte");

	class Input{
	public:
		enum class Direction{
			Up,
			Down,
			Left,
			Right
		};
		enum class Button{
			Start = 0,
			Select = 1,
			A = 2,
			B = 3
		};
		
		Input(CPU& cpu);

		uint8_t get_value();
		void set_value(uint8_t from_byte);

		void on_direction_down(Direction);
		void on_direction_up(Direction);
		void on_button_down(Button);
		void on_button_up(Button);

		void reset();
	protected:
		CPU& cpu;
	
		int direction_horiz = 0;
		int direction_vert = 0;
		bool buttons[4];
	
		InputData current_value;
	};
}
