#include "turnipemu/utils.h"

std::ostream& TurnipEmu::Utils::hexAddressAlpha(std::ostream& os){
	os << std::setfill('0') << std::setw(8) << std::hex;
	return os;
}
