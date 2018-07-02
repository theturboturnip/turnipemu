#pragma once

#include "types.h"
#include "memory.h"

namespace TurnipEmu {
	class ARM7DTMI;

	namespace GBA{
		class DMAEngine : public MemoryRangeController {
		public:
			void execute(ARM7DTMI& cpu);
			bool canExecute();

			// TODO: Implement read/write and associated rules
			// Only the controls are readable, all areas are writeable
			//using MemoryRangeController::startAddress = 0x40000B0;
			//using MemoryRangeController::endAddress = 0x40000DF;

		protected:
			struct DMAChannelState {
				// TODO: Internal DMA State
			};
			DMAChannelState channels[4];
		};
	}
}
