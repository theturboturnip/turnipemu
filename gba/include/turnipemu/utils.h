#pragma once

#include <cstdint>

#pragma pack(0)
#if TURNIPEMU_BIG_ENDIAN
#define TURNIPEMU_WORD_BLOCK(BITS_FROM_0, BITS_FROM_8, BITS_FROM_16, BITS_FROM_24) \
	BITS_FROM_0;														\
	BITS_FROM_8;														\
	BITS_FROM_16;														\
	BITS_FROM_24;
#define TURNIPEMU_WORD_BLOCK_FROM_HALFWORD(HALFWORD_FROM_0, HALFWORD_FROM_16) \
	HALFWORD_FROM_0;													\
	HALFWORD_FROM_16;											
#define TURNIPEMU_HALFWORD_BLOCK(BITS_FROM_0, BITS_FROM_8) \
	BITS_FROM_0;										   \
	BITS_FROM_8;

namespace TurnipEmu::Test{
    // Test the macros
	struct TestWordFromBytes {
		TURNIPEMU_WORD_BLOCK(uint8_t first, uint8_t second, uint8_t third, uint8_t fourth)
	};
	static_assert(sizeof(TestWordFromBytes) == 4, "Result of TURNIPEMU_WORD_BLOCK has wrong size");
	static_assert(offsetof(TestWordFromBytes, first) == 0, "TURNIPEMU_WORD_BLOCK put something at wrong offset");
	static_assert(offsetof(TestWordFromBytes, second) == 1, "TURNIPEMU_WORD_BLOCK put something at wrong offset");
	static_assert(offsetof(TestWordFromBytes, third) == 2, "TURNIPEMU_WORD_BLOCK put something at wrong offset");
	static_assert(offsetof(TestWordFromBytes, fourth) == 3, "TURNIPEMU_WORD_BLOCK put something at wrong offset");

	struct TestHalfWord {
		TURNIPEMU_HALFWORD_BLOCK(uint8_t first, uint8_t second)
	};
	static_assert(sizeof(TestHalfWord) == 2, "Result of TURNIPEMU_HALFWORD_BLOCK has wrong size");
	static_assert(offsetof(TestHalfWord, first) == 0, "TURNIPEMU_HALFWORD_BLOCK put something at wrong offset");
	static_assert(offsetof(TestHalfWord, second) == 1, "TURNIPEMU_HALFWORD_BLOCK put something at wrong offset");
	
	struct TestWordFromHalfWord {
		TURNIPEMU_WORD_BLOCK_FROM_HALFWORD(TestHalfWord first, TestHalfWord second)
	};
	static_assert(sizeof(TestWordFromHalfWord) == 4, "Result of TURNIPEMU_WORD_BLOCK_FROM_HALFWORD has wrong size");
	static_assert(offsetof(TestWordFromHalfWord, first) == 0, "TURNIPEMU_WORD_BLOCK_FROM_HALFWORD put something at wrong offset");
	static_assert(offsetof(TestWordFromHalfWord, second) == 2, "TURNIPEMU_WORD_BLOCK_FROM_HALFWORD put something at wrong offset");

	static_assert(offsetof(TestWordFromHalfWord, first.first) == 0, "TURNIPEMU_WORD_BLOCK_FROM_HALFWORD put something at wrong offset");
	static_assert(offsetof(TestWordFromHalfWord, first.second) == 1, "TURNIPEMU_WORD_BLOCK_FROM_HALFWORD put something at wrong offset");
	static_assert(offsetof(TestWordFromHalfWord, second.first) == 2, "TURNIPEMU_WORD_BLOCK_FROM_HALFWORD put something at wrong offset");
	static_assert(offsetof(TestWordFromHalfWord, second.second) == 3, "TURNIPEMU_WORD_BLOCK_FROM_HALFWORD put something at wrong offset");
}
#else
#define TURNIPEMU_WORD_BLOCK(BITS_FROM_0, BITS_FROM_8, BITS_FROM_16, BITS_FROM_24) \
	BITS_FROM_24;														\
	BITS_FROM_16;														\
	BITS_FROM_8;														\
	BITS_FROM_0;
#define TURNIPEMU_WORD_BLOCK_FROM_HALFWORD(HALFWORD_FROM_0, HALFWORD_FROM_16) \
	HALFWORD_FROM_16;													\
	HALFWORD_FROM_0;											
#define TURNIPEMU_HALFWORD_BLOCK(BITS_FROM_0, BITS_FROM_8) \
	BITS_FROM_8;										   \
	BITS_FROM_0;

namespace TurnipEmu::Test{
    // Test the macros
	struct TestWordFromBytes {
		TURNIPEMU_WORD_BLOCK(uint8_t first, uint8_t second, uint8_t third, uint8_t fourth)
	};
	static_assert(sizeof(TestWordFromBytes) == 4, "Result of TURNIPEMU_WORD_BLOCK has wrong size");
	static_assert(offsetof(TestWordFromBytes, first) == 3, "TURNIPEMU_WORD_BLOCK put something at wrong offset");
	static_assert(offsetof(TestWordFromBytes, second) == 2, "TURNIPEMU_WORD_BLOCK put something at wrong offset");
	static_assert(offsetof(TestWordFromBytes, third) == 1, "TURNIPEMU_WORD_BLOCK put something at wrong offset");
	static_assert(offsetof(TestWordFromBytes, fourth) == 0, "TURNIPEMU_WORD_BLOCK put something at wrong offset");

	struct TestHalfWord {
		TURNIPEMU_HALFWORD_BLOCK(uint8_t first, uint8_t second)
	};
	static_assert(sizeof(TestHalfWord) == 2, "Result of TURNIPEMU_HALFWORD_BLOCK has wrong size");
	static_assert(offsetof(TestHalfWord, first) == 1, "TURNIPEMU_HALFWORD_BLOCK put something at wrong offset");
	static_assert(offsetof(TestHalfWord, second) == 0, "TURNIPEMU_HALFWORD_BLOCK put something at wrong offset");
	
	struct TestWordFromHalfWord {
		TURNIPEMU_WORD_BLOCK_FROM_HALFWORD(TestHalfWord first, TestHalfWord second)
	};
	static_assert(sizeof(TestWordFromHalfWord) == 4, "Result of TURNIPEMU_WORD_BLOCK_FROM_HALFWORD has wrong size");
	static_assert(offsetof(TestWordFromHalfWord, first) == 2, "TURNIPEMU_WORD_BLOCK_FROM_HALFWORD put something at wrong offset");
	static_assert(offsetof(TestWordFromHalfWord, second) == 0, "TURNIPEMU_WORD_BLOCK_FROM_HALFWORD put something at wrong offset");

	static_assert(offsetof(TestWordFromHalfWord, first.first) == 3, "TURNIPEMU_WORD_BLOCK_FROM_HALFWORD put something at wrong offset");
	static_assert(offsetof(TestWordFromHalfWord, first.second) == 2, "TURNIPEMU_WORD_BLOCK_FROM_HALFWORD put something at wrong offset");
	static_assert(offsetof(TestWordFromHalfWord, second.first) == 1, "TURNIPEMU_WORD_BLOCK_FROM_HALFWORD put something at wrong offset");
	static_assert(offsetof(TestWordFromHalfWord, second.second) == 0, "TURNIPEMU_WORD_BLOCK_FROM_HALFWORD put something at wrong offset");
}
#endif
#pragma pack()
