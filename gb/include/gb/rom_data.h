// Copyright Samuel Stark 2017

#pragma once

#include <cstdint>
#include <unordered_map>

namespace GB::RomData{
	constexpr static uint16_t ROM_OFFSET_NAME = 0x134;
	constexpr static uint16_t ROM_OFFSET_TYPE = 0x147;
	
	constexpr static uint16_t ROM_OFFSET_ROM_SIZE = 0x148;
	const static std::unordered_map<uint8_t, uint16_t> ROM_SIZE_TO_BANK_COUNT{
		{0, 2},
		{1, 4},
		{2, 8},
		{3, 16},
		{4, 32},
		{5, 64},
		{6, 128},
		{0x52, 72},
		{0x53, 80},
		{0x54, 96},
	};
	constexpr static uint16_t ROM_BANK_SIZE = 0x4000;
	
	constexpr static uint16_t ROM_OFFSET_RAM_SIZE = 0x149;
	const static std::unordered_map<uint8_t, uint8_t> RAM_SIZE_TO_BANK_COUNT{
		{0, 0},
		{1, 1},
		{2, 1},
		{3, 4},
		{4, 16}
	};
	
	enum RomType{
		ROM_PLAIN = 0x00,
		ROM_MBC1 = 0x01,
		ROM_MBC1_RAM = 0x02,
		ROM_MBC1_RAM_BATT = 0x03,
		ROM_MBC2 = 0x05,
		ROM_MBC2_BATTERY = 0x06,
		ROM_RAM = 0x08,
		ROM_RAM_BATTERY = 0x09,
		ROM_MMM01 = 0x0B,
		ROM_MMM01_SRAM = 0x0C,
		ROM_MMM01_SRAM_BATT = 0x0D,
		ROM_MBC3_TIMER_BATT = 0x0F,
		ROM_MBC3_TIMER_RAM_BATT = 0x10,
		ROM_MBC3 = 0x11,
		ROM_MBC3_RAM = 0x12,
		ROM_MBC3_RAM_BATT = 0x13,
		ROM_MBC5 = 0x19,
		ROM_MBC5_RAM = 0x1A,
		ROM_MBC5_RAM_BATT = 0x1B,
		ROM_MBC5_RUMBLE = 0x1C,
		ROM_MBC5_RUMBLE_SRAM = 0x1D,
		ROM_MBC5_RUMBLE_SRAM_BATT = 0x1E,
		ROM_POCKET_CAMERA = 0x1F,
		ROM_BANDAI_TAMA5 = 0xFD,
		ROM_HUDSON_HUC3 = 0xFE,
		ROM_HUDSON_HUC1 = 0xFF,
	};
	const static std::unordered_map<RomType, char const*, std::hash<uint8_t>> RomType_strings{
		{RomType::ROM_PLAIN, "ROM_PLAIN"},
		{RomType::ROM_MBC1, "ROM_MBC1 (ROM only)"},
		{RomType::ROM_MBC1_RAM, "ROM_MBC1 (ROM + RAM)"},
		{RomType::ROM_MBC3_RAM_BATT, "ROM_MBC3"}
		   
			// TODO: Define strings for other types
	};
}
