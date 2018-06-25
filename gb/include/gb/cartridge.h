
#pragma once

#include "gb/rom_data.h"
#include "gb/mbc.h"

#include <memory>

namespace GB{

	class MMU;
	
	class Cartridge{
		friend class MMU;
	public:
		Cartridge(std::vector<uint8_t> data);
		void reset();

		std::unique_ptr<MBC> mbc;
	};
}
