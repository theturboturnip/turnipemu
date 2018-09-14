#include "turnipemu/arm7tdmi/alu.h"

#include "turnipemu/utils.h"
#include "turnipemu/log.h"

namespace TurnipEmu::ARM7TDMI::ALU {
	template<bool WithCarry>
	auto Add(word arg1, word arg2, int carryIn){
		uint64_t ulongResult = (uint64_t)arg1 + (uint64_t)arg2 + (WithCarry ? carryIn : 0);
		int64_t slongResult = TURNIPEMU_UINT32_TO_SINT64(arg1) + TURNIPEMU_UINT32_TO_SINT64(arg2) + (WithCarry ? carryIn : 0);
		OpOutput<OpType::Arithmetic> output(
			word(ulongResult),
			(ulongResult >> 32) & 1,
			(slongResult > std::numeric_limits<int32_t>::max()) || (slongResult < std::numeric_limits<int32_t>::lowest())
			);
		return output;
	}
	template<bool WithCarry, bool Reverse>
	auto Sub(word arg1, word arg2, int carryIn){
		if (!WithCarry) carryIn = 1;
		if (Reverse)
			return Add<true>(arg2, ~arg1, carryIn);
		else
			return Add<true>(arg1, ~arg2, carryIn);
	}

	auto LogicalShiftLeft(word value, word amount) {
		return OpOutput<OpType::Logical>(value << amount);
	}
	auto LogicalShiftRight(word value, word amount) {
		return OpOutput<OpType::Logical>(value >> amount);
	}
	// Arithmetic Shift Right (sign extended)
	auto ArithmeticShiftRight(word value, word amount, int carryIn) {
		// Although this is arithmetic, it doesn't use Cin.
		// TODO: Should this return an OpOutput<Arithmetic>?
		word result = value >> amount;
		if ((value >> 31) & 1){ // TODO: This assumes the source width is word
			// Sign bit set, extend to the left
			result |= word(~0) << (32 - amount);
		}
		return OpOutput<OpType::Arithmetic>(result, false, false);
	}
	auto RotateRight(word value, word amount) {
		return OpOutput<OpType::Logical>(
			value >> amount |
			((value << (32 - amount)) & word(~0))
			);
	};
	auto RotateRightExtended(word value, word amount, int carryIn){
		return OpOutput<OpType::Arithmetic>(
			((carryIn << 31) & 1) |
			value >> amount |
			((value << (33 - amount)) & word(~0)),

			(value >> (amount - 1)) & 1,
			false // No Overflow
			);
	}

	word RequestInput::Evaluate(Instructions::InstructionRegisterInterface registers, int* shiftedCarryOut) const {
		word value = (type == ValueType::Immediate) ? immediateValue : registers.get(registerIndex);
		if (type == ValueType::Immediate && immediateRotation){
			value = RotateRight(value, immediateRotation).result;
		}
		
		if (shift.enabled){
			word amount = (shift.amountType == ValueType::Immediate) ? shift.amountImmediateValue : registers.get(shift.amountRegisterIndex);

			switch(shift.shiftType){
			case ShiftType::LogicalShiftLeft:
				if (value != 0 && shiftedCarryOut) *shiftedCarryOut = (value >> (32 - amount)) & 1;
				value = LogicalShiftLeft(value, amount).result;
				break;
			case ShiftType::LogicalShiftRight:
				if (shiftedCarryOut) *shiftedCarryOut = (value >> (amount - 1)) & 1;
				value = LogicalShiftRight(value, amount).result;
				break;
			case ShiftType::ArithmeticShiftRight:
				if (shiftedCarryOut) *shiftedCarryOut = (value >> (amount - 1)) & 1;
				value = ArithmeticShiftRight(value, amount, registers.cpsr().carry).result;
				break;
			case ShiftType::RotateRight:
				if (shift.amountType == ValueType::Immediate && amount == 0){
					auto rorxResult = RotateRightExtended(value, amount, registers.cpsr().carry);
					if (shiftedCarryOut) *shiftedCarryOut = rorxResult.carry;
					value = rorxResult.result;
				}else
					value = RotateRight(value, amount).result;
				break;
			}
		}

		return value;
	}
	std::ostream& operator << (std::ostream& os, const RequestInput& requestInput){
		if (requestInput.type == RequestInput::ValueType::Immediate){
			os << Utils::HexFormat<word>(requestInput.immediateValue);
			if (requestInput.immediateRotation)
				os << " ROR by " << (int)requestInput.immediateRotation;
		}else{
			os << "Register " << (int)requestInput.registerIndex;
		}
		if (requestInput.shift.enabled){
			switch(requestInput.shift.shiftType){
			case RequestInput::ShiftType::LogicalShiftLeft:
				os << " << ";
				break;
			case RequestInput::ShiftType::LogicalShiftRight:
				os << " >> ";
				break;
			case RequestInput::ShiftType::ArithmeticShiftRight:
				os << " ASR ";
				break;
			case RequestInput::ShiftType::RotateRight:
				os << ((requestInput.shift.amountType == RequestInput::ValueType::Immediate && requestInput.shift.amountImmediateValue == 0) ? " RORX " : " ROR ");
				break;
			}
			if (requestInput.shift.amountType == RequestInput::ValueType::Immediate){
				os << Utils::HexFormat<word>(requestInput.shift.amountImmediateValue);
			}else{
				os << "Register " << (int)requestInput.shift.amountRegisterIndex;
			}
		}

		return os;
	}

	void Request::Evaluate(Instructions::InstructionRegisterInterface registers) const {
		if (op->selectedOperationType == OpType::Logical){
			int carryOut = 0;
			word operand1Value = operand1.Evaluate(registers, &carryOut);
			word operand2Value = operand2.Evaluate(registers, &carryOut);
			
			const auto result = op->logicalOp.execute(operand1Value, operand2Value);

			if (op->logicalOp.writeResult) registers.set(destinationRegister, result.result);
			if (setFlags) result.ApplyToPSR(registers.cpsr(), carryOut);

			if (!op->logicalOp.writeResult && !setFlags)
				throw std::runtime_error("ALU Request did not set flags or write result!");
		}else{
			word operand1Value = operand1.Evaluate(registers);
			word operand2Value = operand2.Evaluate(registers);
			
			const auto result = op->arithmeticOp.execute(operand1Value, operand2Value, registers.cpsr().carry ? 1 : 0);

			if (op->arithmeticOp.writeResult) registers.set(destinationRegister, result.result);
			if (setFlags) result.ApplyToPSR(registers.cpsr());

			if (!op->arithmeticOp.writeResult && !setFlags)
				throw std::runtime_error("ALU Request did not set flags or write result!");
		}
	}
	std::ostream& operator << (std::ostream& os, const Request& request) {
		os << "ALU OP " << ((request.op->selectedOperationType == OpType::Logical) ? request.op->logicalOp.mnemonic : request.op->arithmeticOp.mnemonic);
		os << "\nOperand 1: " << request.operand1;
		os << "\nOperand 2: " << request.operand2;
		os << "\nDestination: Register " << ((int)request.destinationRegister);
		os << "\nFlags: " << std::boolalpha << request.setFlags;
	    return os;
	}

	void MultiplyRequest::Evaluate(Instructions::InstructionRegisterInterface registers) const {
		word operand1Value = operand1.Evaluate(registers);
		word operand2Value = operand2.Evaluate(registers);

		uint64_t uResult;
		
		if (isSigned){
			uResult = static_cast<uint64_t>(
				TURNIPEMU_UINT32_TO_SINT64(operand1Value) * TURNIPEMU_UINT32_TO_SINT64(operand2Value)
				);
		}else{
			uResult = operand1Value * operand2Value;
		}

		uint64_t uFullResult = uResult;
		if (accumulate){
			uFullResult += registers.get(destinationRegister1);
			if (isLong) uFullResult += ((uint64_t)registers.get(destinationRegister2)) << 32;
		}

		registers.set(destinationRegister1, uFullResult & word(~0));
		if (isLong) registers.set(destinationRegister2, (uFullResult << 32) & word(~0));

		registers.cpsr().zero = uFullResult == 0;
		registers.cpsr().negative = (uFullResult >> 63) & 1;
		// Carry and Overflow are defined to "set garbage values", so we don't need to modify them.
	}
	std::ostream& operator << (std::ostream& os, const MultiplyRequest& request) {
		os << "Multiply " << (request.isSigned ? 'S' : '/') << (request.accumulate ? 'A' : '/') << (request.isLong ? 'L' : '/');
		os << "\nOperand 1: " << request.operand1;
		os << "\nOperand 2: " << request.operand2;
		os << "\nDestination: Register " << ((int)request.destinationRegister1);
		if (request.isLong) os << " [Low] Register " << ((int)request.destinationRegister2) << " [High]";
		os << "\nFlags: " << std::boolalpha << request.setFlags;
	    return os;
	}

	// Arithmetic Ops
	const ALU::Operation ADD = {
		"ADD",
		ALU::Add<false>
	};
	const ALU::Operation ADC = {
		"ADC",
		ALU::Add<true>
	};
	const ALU::Operation SUB = {
		"SUB",
		ALU::Sub<false, false>
	};
	const ALU::Operation SBC = {
		"SBC",
		ALU::Sub<true, false>
	};
	const ALU::Operation RSB = {
		"RSB",
		ALU::Sub<false, true>
	};
	const ALU::Operation RSC = {
		"RSC",
		ALU::Sub<true, true>
	};

	// Logical Ops
	const ALU::Operation AND = {
		"AND",
		[](word arg1, word arg2){
			return OpOutput<OpType::Logical>(arg1 & arg2);
		}
	};
	const ALU::Operation EOR = {
		"EOR", // XOR
		[](word arg1, word arg2){
			return OpOutput<OpType::Logical>(arg1 ^ arg2);
		}
	};
	const ALU::Operation ORR = {
		"ORR", // OR
		[](word arg1, word arg2){
			return OpOutput<OpType::Logical>(arg1 | arg2);
		}
	};
	const ALU::Operation MOV = {
		"MOV",
		[](word arg1, word arg2){
			return OpOutput<OpType::Logical>(arg2);
		}
	};
	const ALU::Operation BIC = {
		"BIC", // A AND NOT B
		[](word arg1, word arg2){
			return OpOutput<OpType::Logical>(arg1 & ~arg2);
		}
	};
	const ALU::Operation MVN = {
		"MVN", // MOV NOT
		[](word arg1, word arg2){
			return OpOutput<OpType::Logical>(~arg2);
		}
	};

	// Test Ops (only sets flags)
	const ALU::Operation TST = {
		"TST", // AND without result
		[](word arg1, word arg2){
			return OpOutput<OpType::Logical>(arg1 & arg2);
		},
		false
	};
	const ALU::Operation TEQ = {
		"TEQ", // XOR without result
		[](word arg1, word arg2){
			return OpOutput<OpType::Logical>(arg1 ^ arg2);
		},
		false
	};
	const ALU::Operation CMP = {
		"CMP", // SUB without result
		Sub<false, false>,
		false
	};
	const ALU::Operation CMN = {
		"CMN", // ADD without result
		Add<false>,
		false
	};

	namespace Thumb {
		// Logical Shifts: A shifted by B
		const ALU::Operation LSL = {
			"LSL", // Logical Shift Left
			LogicalShiftLeft
		};
		const ALU::Operation LSR = {
			"LSR", // Logical Shift Right
			LogicalShiftRight
		};
		const ALU::Operation ASR = {
			"ASR", // Arithmetic Shift Right (sign extended)
			ArithmeticShiftRight
		};
		const ALU::Operation ROR = {
			"ROR", // Rotate Right (no carry)
			RotateRight
		};

		// Misc. Ops
		const ALU::Operation MUL = {
			"MUL", // Multiply
			[](word arg1, word arg2, int carryIn){
				// TODO: Flags
				throw std::runtime_error("Thumb MUL doesn't have the correct flags implemented");
				return ALU::OpOutput<OpType::Arithmetic>(arg1 * arg2, false, false);
			}
		};
		const ALU::Operation NEG = {
			"NEG", // Negate
			[](word arg1, word arg2, int carryIn){
				// Use ALU::Sub to make sure the flags will be correct
				return ALU::Sub<false, false>(0, arg2, carryIn);
			}
		};
	}
}
