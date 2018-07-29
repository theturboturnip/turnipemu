#pragma once

namespace TurnipEmu::ARM7TDMI {
	enum class Mode : uint8_t {
		User       = 0b10000,
		FIQ        = 0b10001, // TODO: What is this?
		IRQ        = 0b10010, // TODO: What is this?
		Supervisor = 0b10011,
		Abort      = 0b10111,
		Undefined  = 0b11011,
		System     = 0b11111,
	};
	inline std::string ModeString(Mode m){
		switch(m){
		case Mode::User: return "User";
		case Mode::FIQ: return "FIQ";
		case Mode::IRQ: return "IRQ";
		case Mode::Supervisor: return "Supervisor";
		case Mode::Abort: return "Abort";
		case Mode::Undefined: return "Undefined";
		case Mode::System: return "System";
		}
		return "Invalid";
	}
}
