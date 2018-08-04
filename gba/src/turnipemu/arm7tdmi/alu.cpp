#include "turnipemu/arm7tdmi/alu.h"

#include "turnipemu/utils.h"

namespace TurnipEmu::ARM7TDMI::ALU {
	template<bool WithCarry>
	OperationOutput Add(word arg1, word arg2, int carryIn){
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
	OperationOutput Sub(word arg1, word arg2, int carryIn){
		if (!WithCarry) carryIn = 1;
		if (Reverse)
			return Add<true>(arg2, ~arg1, carryIn);
		else
			return Add<true>(arg1, ~arg2, carryIn);
	}

	// Arithmetic Ops
	const ALU::Operation ADD = {
		"ADD",
		ALU::Add<false>,
		ALU::OperationType::Arithmetic
	};
	const ALU::Operation ADC = {
		"ADC",
		ALU::Add<true>,
		ALU::OperationType::Arithmetic
	};
	const ALU::Operation SUB = {
		"SUB",
		ALU::Sub<false, false>,
		ALU::OperationType::Arithmetic
	};
	const ALU::Operation SBC = {
		"SBC",
		ALU::Sub<true, false>,
		ALU::OperationType::Arithmetic
	};
	const ALU::Operation RSB = {
		"RSB",
		ALU::Sub<false, true>,
		ALU::OperationType::Arithmetic
	};
	const ALU::Operation RSC = {
		"RSC",
		ALU::Sub<true, true>,
		ALU::OperationType::Arithmetic
	};

	// Logical Ops
	const ALU::Operation AND = {
		"AND",
		[](word arg1, word arg2, int carryIn){
			return OperationOutput(arg1 & arg2);
		},
		OperationType::Logical
	};
	const ALU::Operation EOR = {
		"EOR", // XOR
		[](word arg1, word arg2, int carryIn){
			return OperationOutput(arg1 ^ arg2);
		},
		OperationType::Logical
	};
	const ALU::Operation ORR = {
		"ORR", // OR
		[](word arg1, word arg2, int carryIn){
			return OperationOutput(arg1 | arg2);
		},
		OperationType::Logical,
	};
	const ALU::Operation MOV = {
		"MOV",
		[](word arg1, word arg2, int carryIn){
			return OperationOutput(arg2);
		},
		OperationType::Logical
	};
	const ALU::Operation BIC = {
		"BIC", // A AND NOT B
		[](word arg1, word arg2, int carryIn){
			return OperationOutput(arg1 & ~arg2);
		},
		OperationType::Logical,
	};
	const ALU::Operation MVN = {
		"MVN", // MOV NOT
		[](word arg1, word arg2, int carryIn){
			return OperationOutput(~arg2);
		},
		OperationType::Logical
	};

	// Test Ops (only sets flags)
	const ALU::Operation TST = {
		"TST", // AND without result
		[](word arg1, word arg2, int carryIn){
			return OperationOutput(arg1 & arg2);
		},
		OperationType::Logical,
		false
	};
	const ALU::Operation TEQ = {
		"TEQ", // XOR without result
		[](word arg1, word arg2, int carryIn){
			return OperationOutput(arg1 ^ arg2);
		},
		OperationType::Logical,
		false
	};
	const ALU::Operation CMP = {
		"CMP", // SUB without result
		Sub<false, false>,
		OperationType::Arithmetic,
		false
	};
	const ALU::Operation CMN = {
		"CMN", // ADD without result
		Add<false>,
		OperationType::Arithmetic,
		false
	};

	namespace Thumb {
		// Logical Shifts: A shifted by B
		const ALU::Operation LSL = {
			"LSL", // Logical Shift Left
			[](word arg1, word arg2, int carryIn){
				return ALU::OperationOutput(arg1 << arg2);
			},
			ALU::OperationType::Logical
		};
		const ALU::Operation LSR = {
			"LSR", // Logical Shift Right
			[](word arg1, word arg2, int carryIn){
				return ALU::OperationOutput(arg1 >> arg2);
			},
			ALU::OperationType::Logical
		};
		const ALU::Operation ASR = {
			"ASR", // Arithmetic Shift Right (sign extended)
			[](word arg1, word arg2, int carryIn){
				word result = arg1 >> arg2;
				if ((arg1 >> 31) & 1){ // TODO: This assumes the source width is word
					// Sign bit set, extend to the left
					result |= word(~0) << (32 - arg2);
				}
				return ALU::OperationOutput(result);
			},
			ALU::OperationType::Arithmetic
		};
		const ALU::Operation ROR = {
			"ROR", // Rotate Right (no carry)
			[](word arg1, word arg2, int carryIn){
				return ALU::OperationOutput(
					arg1 >> arg2 |
					((arg1 << (32 - arg2)) & word(~0))
					);
			},
			ALU::OperationType::Logical
		};

		// Misc. Ops
		const ALU::Operation MUL = {
			"MUL", // Multiply
			[](word arg1, word arg2, int carryIn){
				// TODO: Flags
				throw std::runtime_error("Thumb MUL doesn't have the correct flags implemented");
				return ALU::OperationOutput(arg1 * arg2);
			},
			ALU::OperationType::Arithmetic
		};
		const ALU::Operation NEG = {
			"NEG", // Negate
			[](word arg1, word arg2, int carryIn){
				// Use ALU::Sub to make sure the flags will be correct
				return ALU::Sub<false, false>(0, arg2, carryIn);
			},
			ALU::OperationType::Arithmetic
		};
	}
}
