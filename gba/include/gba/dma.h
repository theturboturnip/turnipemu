#pragma once

#include "types.h"
#include "memory.h"

class ARM7DTMI;

namespace GBA{
	class DMAEngine : public MemoryController, public AddressableMemoryRange{
	public:
		void execute(ARM7DTMI& cpu);
		bool canExecute();

		// TODO: Implement read/write and associated rules
		// Only the controls are readable, all areas are writeable

		using AddressableMemoryRange::startAddress = 0x40000B0;
		using AddressableMemoryRange::endAddress = 0x40000DF;

	protected:
		struct DMAChannelState {
			// TODO: Internal DMA State
		};
		DMAChannelState[4] channels;
	};
}
