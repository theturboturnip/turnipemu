#pragma once

#include "turnipemu/types.h"
#include "turnipemu/arm7tdmi/registers.h"

#include <functional>

namespace TurnipEmu::ARM7TDMI::ALU {
	struct OperationOutput {
		word result;
		bool carry;
		bool overflow;

		constexpr OperationOutput(word result) : result(result), carry(false), overflow(false){}
		constexpr OperationOutput(word result, bool carry, bool overflow) : result(result), carry(carry), overflow(overflow) {}

		inline void applyToPSR(ProgramStatusRegister& psr){
			psr.overflow = overflow;
			psr.carry = carry;
			psr.zero = (result == 0);
			psr.negative = (result >> 31) & 1;
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

	// Arithmetic Ops
	const extern ALU::Operation ADD;
	const extern ALU::Operation ADC;
	const extern ALU::Operation SUB;
	const extern ALU::Operation SBC;
	const extern ALU::Operation RSB;
	const extern ALU::Operation RSC;

	// Logical Ops
	const extern ALU::Operation AND;
	const extern ALU::Operation EOR; // XOR
	const extern ALU::Operation ORR; // OR
	const extern ALU::Operation MOV;
	const extern ALU::Operation BIC; // A AND NOT B
	const extern ALU::Operation MVN; // NOT B

	// Test Ops (only sets flags)
	const extern ALU::Operation TST; // AND without result
	const extern ALU::Operation TEQ; // XOR without result
	const extern ALU::Operation CMP; // SUB without result
	const extern ALU::Operation CMN; // ADD without result

	namespace Thumb {
		// Logical Shifts: A shifted by B
		const extern ALU::Operation LSL; // Logical Shift Left
		const extern ALU::Operation LSR; // Logical Shift Right
		const extern ALU::Operation ASR; // Arithmetic Shift Right (sign extended)
		const extern ALU::Operation ROR; // Rotate Right

		// Misc. Ops
		const extern ALU::Operation MUL; // Multiply
		const extern ALU::Operation NEG; // Negate
	}
}
