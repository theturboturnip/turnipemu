// Copyright Samuel Stark 2017

#include "gb/input.h"
#include "gb/cpu.h"

namespace GB{
	Input::Input(CPU& cpu) : cpu(cpu){}

	void Input::reset(){
		direction_horiz = 0;
		direction_vert = 0;
	
		buttons[static_cast<int>(Button::Start)] = false;
		buttons[static_cast<int>(Button::Select)] = false;
		buttons[static_cast<int>(Button::A)] = false;
		buttons[static_cast<int>(Button::B)] = false;

		set_value(1);
	}

	uint8_t Input::get_value(){
		if (current_value.allow_buttons){
		
			current_value.button_start = !buttons[static_cast<int>(Button::Start)];
			current_value.button_select = !buttons[static_cast<int>(Button::Select)];
			current_value.button_a = !buttons[static_cast<int>(Button::A)];
			current_value.button_b = !buttons[static_cast<int>(Button::B)];
		}
		if (current_value.allow_directions){
			current_value.direction_up &= (direction_vert != 1);
			current_value.direction_down &= (direction_vert != -1);
			current_value.direction_left &= (direction_horiz != -1);
			current_value.direction_right &= (direction_horiz != 1);
		}


		uint8_t byte_value=*reinterpret_cast<uint8_t*>(&current_value);
		if ((byte_value & 0xf) != 0xf){
			//fprintf(stdout, "Input Value: 0x%02x\n", byte_value);
		}
		return byte_value;
	}
	void Input::set_value(uint8_t new_byte){
		InputData new_data = *reinterpret_cast<InputData*>(&new_byte);

		current_value.allow_buttons = new_data.allow_buttons;
		current_value.allow_directions = new_data.allow_directions;
	}

	void Input::on_direction_down(Direction direction){
		if (current_value.allow_directions){
			cpu.interrupts.trigger(Interrupt::Joypad);
		}
	
		switch(direction){
		case Direction::Up:
			direction_vert = 1;
			break;
		case Direction::Down:
			direction_vert = -1;
			break;
		case Direction::Left:
			direction_horiz = -1;
			break;
		case Direction::Right:
			direction_horiz = 1;
			break;
		}
	}
	void Input::on_direction_up(Direction direction){
		switch(direction){
		case Direction::Up:
		case Direction::Down:
			direction_vert = 0;
			break;
		case Direction::Left:
		case Direction::Right:
			direction_horiz = 0;
			break;
		}
	}
	void Input::on_button_down(Button button){
		if (current_value.allow_buttons){
			cpu.interrupts.trigger(Interrupt::Joypad);
		}
		buttons[static_cast<int>(button)] = true;
	}
	void Input::on_button_up(Button button){
		buttons[static_cast<int>(button)] = false;
	}
}
