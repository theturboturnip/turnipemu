#pragma once

#include <cstdint>
#include <sstream>
#include <iomanip>
#include <bitset>

namespace TurnipEmu::Utils{
	
	template<typename Arg>
	inline void streamFormat(std::stringstream& stream, Arg a);
	
	// This will call itself with one less argument each time
	template<typename Arg, typename ...Args>
	inline void streamFormat(std::stringstream& stream, Arg a, Args... args){
		stream << a;
		streamFormat(stream, args...);
	}
	
	// This will be called once there's only one argument left
	template<typename Arg>
	inline void streamFormat(std::stringstream& stream, Arg a){
		stream << a;
	}

	// This will start the chain
	template<typename ...Args>
	inline std::string streamFormat(Args... args){
		std::stringstream stream;
		streamFormat(stream, args...);
		return stream.str();
	}
	// If it's called with nothing, this will be used instead
	inline std::string streamFormat(){
		return "";
	}

	template<typename NumberType>
	struct HexFormat{
		HexFormat(NumberType number) : number(number) {}
		
		friend std::ostream& operator << (std::ostream &os, const HexFormat<NumberType>& format){
			os << "0x" << std::setfill('0') << std::setw(sizeof(NumberType) * 2) << std::hex << (int)format.number << std::dec; // Put the std::dec in to make sure new numbers use decimal formatting
			return os;
		}
	private:
		NumberType number;
	};
	template<typename NumberType>
	struct BinaryFormat{
		BinaryFormat(NumberType number) : number(number) {}
		
		friend std::ostream& operator << (std::ostream &os, const BinaryFormat<NumberType>& format){
			os << "0b" << std::bitset<sizeof(NumberType) * 8>(format.number);
			return os;
		}
	private:
		NumberType number;
	};

	template<typename InputNumberType, typename TargetNumberType, int StartingBit>
	TargetNumberType SignExtend(InputNumberType number){
		TargetNumberType result;
		if ((number >> StartingBit) & 1){
			result = number | ((unsigned long long)(~0) << StartingBit);
		}else{
			result = number;
		}
		return result;
	}
}

#define TURNIPEMU_UINT32_TO_SINT64(VALUE) (int64_t)(int32_t)(VALUE)

#define TURNIPEMU_SPLIT_HALFWORD_OR_LESS(NAME, SIZE)	\
	uint8_t NAME##_low : 8;								\
	uint8_t NAME##_high : (SIZE - 8);					\
	inline uint16_t NAME##(){							\
		return (NAME##_high << 8) | NAME##_low;			\
	}

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
