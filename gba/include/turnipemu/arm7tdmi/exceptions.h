#pragma once

#include "turnipemu/arm7tdmi/modes.h"

namespace TurnipEmu::ARM7TDMI {
	// Integer value = priority, lower is better.
	// e.x. a FIQ exception is handled BEFORE an UndefinedInstruction exception
	enum class Exception {
		Reset = 0,
		DataAbort = 1,
		FIQ = 2,
		IRQ = 3,
		PrefetchAbort = 4,
		UndefinedInstruction = 5,
		SoftwareInterrupt = 6
	};
	struct ExceptionData {
		std::string name;
		word handlerAddress;
		Mode modeOnEntry;
	};
	const static std::array<ExceptionData, 7> exceptionsData  = {{
		{ "Reset",                 0x00, Mode::Supervisor },
		{ "Data Abort",            0x10, Mode::Abort      },
		{ "FIQ",                   0x1C, Mode::FIQ        },
		{ "IRQ",                   0x18, Mode::IRQ        },
		{ "Prefetch Abort",        0x0C, Mode::Abort      },
		{ "Undefined Instruction", 0x04, Mode::Undefined  },
		{ "Software Interrupt",    0x08, Mode::Supervisor },
		}};
}
