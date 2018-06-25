// Copyright Samuel Stark 2017

#pragma once

#include <cstdint>
#include <vector>
#include <assert.h>

namespace GB{
	class CPU;

	namespace Instructions{
		class Instruction{
		public:	
			const char* const disassembly;
			// Returns cycles taken
			virtual uint8_t execute(CPU& cpu);

			virtual ~Instruction(){}
			Instruction(const char* const disassembly) : disassembly(disassembly){}
		};
	}

}
