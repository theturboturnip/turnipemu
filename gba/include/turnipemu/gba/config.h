#pragma once

namespace TurnipEmu::GBA {
	namespace StaticConfig {
		constexpr bool DebugRomData = true;
	}
	struct RuntimeConfig {
		bool RunBIOS = true;
	};
}
