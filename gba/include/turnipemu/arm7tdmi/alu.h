#pragma once

#include "turnipemu/types.h"
#include "turnipemu/arm7tdmi/registers.h"

namespace TurnipEmu::ARM7TDMI::ALU {
	struct OperationOutput {
		word result;
		bool carry;
		bool overflow;

		constexpr OperationOutput(word result) : result(result), carry(false), overflow(false){}
		constexpr OperationOutput(word result, bool carry, bool overflow) : result(result), carry(carry), overflow(overflow) {}

		inline void applyToPSR(ProgramStatusRegister* psr){
			psr->overflow = overflow;
			psr->carry = carry;
			psr->zero = (result == 0);
			psr->negative = (result >> 31) & 1;
		}
	};
	struct OperationType {
		constexpr static bool Logical = true;
		constexpr static bool Arithmetic = false;
	};
	struct Operation {
		using OperationFunction = std::function<OperationOutput(word arg1, word arg2, int carryIn)>;
		char mnemonic[4];
		OperationFunction execute;
		bool logical;
		bool writeResult;

		Operation(const char* mnemonicFromStr, OperationFunction execute, bool logical)
			: Operation(mnemonicFromStr, execute, logical, true){}
		Operation(const char* mnemonicFromStr, OperationFunction execute, bool logical, bool writeResult)
			: execute(execute), logical(logical), writeResult(writeResult) {
			mnemonic[0] = mnemonicFromStr[0];
			mnemonic[1] = mnemonicFromStr[1];
			mnemonic[2] = mnemonicFromStr[2];
			mnemonic[3] = '\0';
		}
	};
		
	template<bool WithCarry>
	static OperationOutput Add(word arg1, word arg2, int carryIn){
		uint64_t ulongResult = (uint64_t)arg1 + (uint64_t)arg2 + (WithCarry ? carryIn : 0);
		int64_t slongResult = TURNIPEMU_UINT32_TO_SINT64(arg1) + TURNIPEMU_UINT32_TO_SINT64(arg2) + (WithCarry ? carryIn : 0);
		OperationOutput output(
			word(ulongResult),
			(ulongResult >> 32) & 1,
			(slongResult > std::numeric_limits<int32_t>::max()) || (slongResult < std::numeric_limits<int32_t>::lowest())
			);
		return output;
	}
	template<bool WithCarry, bool Reverse>
	static OperationOutput Sub(word arg1, word arg2, int carryIn){
		if (!WithCarry) carryIn = 1;
		if (Reverse)
			return Add<true>(arg2, ~arg1, carryIn);
		else
			return Add<true>(arg1, ~arg2, carryIn);
	}
}
