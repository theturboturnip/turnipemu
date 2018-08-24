#pragma once

#include "turnipemu/types.h"
#include "turnipemu/arm7tdmi/registers.h"
#include "turnipemu/arm7tdmi/instruction_category.h"

#include <functional>
#include <type_traits>

namespace TurnipEmu::ARM7TDMI::ALU {
	enum class OpType : bool{
		Logical = true,
		Arithmetic = false
	};

	template<OpType T>
	struct OpOutput;

	template<>
	struct OpOutput<OpType::Logical>{
		word result;
		constexpr OpOutput(word result) : result(result){}

		inline void ApplyToPSR(ProgramStatusRegister& psr, bool carry) const {
			psr.zero = (result == 0);
			psr.negative = (result >> 31) & 1;
			psr.carry = carry;
			// Overflow is unaffected
		}
	};
	template<>
	struct OpOutput<OpType::Arithmetic>{
		word result;
		bool carry;
		bool overflow;

		constexpr OpOutput(word result, bool carry, bool overflow) : result(result), carry(carry), overflow(overflow) {}

		inline void ApplyToPSR(ProgramStatusRegister& psr) const {
			psr.zero = (result == 0);
			psr.negative = (result >> 31) & 1;
			psr.carry = carry;
			psr.overflow = overflow;
		}
	};

	struct Thing{};

	template<OpType T>
	struct OpFunction;
	template<>
	struct OpFunction<OpType::Logical>{
		using type = std::function<OpOutput<OpType::Logical>(word arg1, word arg2)>;
	};
	template<>
	struct OpFunction<OpType::Arithmetic>{
		using type = std::function<OpOutput<OpType::Arithmetic>(word arg1, word arg2, int carryIn)>;
	};
	
	template<OpType T>
	struct TemplatedOp {
		
		//static_assert(std::is_literal_type<OpFunction>());
		//static_assert(std::is_literal_type<std::function<OpOutput<T>(word arg1, word arg2)>>());
		//static_assert(std::is_literal_type<OpOutput<T>>());
		char mnemonic[4];
		typename OpFunction<T>::type execute;
		bool writeResult;

		TemplatedOp() = default;
		TemplatedOp(const char* mnemonicFromStr, typename OpFunction<T>::type execute, bool writeResult = true)
			: execute(execute), writeResult(writeResult) {
			mnemonic[0] = mnemonicFromStr[0];
			mnemonic[1] = mnemonicFromStr[1];
			mnemonic[2] = mnemonicFromStr[2];
			mnemonic[3] = '\0';
		}
	};

	struct Operation {
		const TemplatedOp<OpType::Arithmetic> arithmeticOp;
		const TemplatedOp<OpType::Logical> logicalOp;
		const OpType selectedOperationType;

		Operation(const char* mnemonic, typename OpFunction<OpType::Logical>::type logicalOpFunction, bool writeResult = true)
			: arithmeticOp(), logicalOp(mnemonic, logicalOpFunction, writeResult), selectedOperationType(OpType::Logical) {}
		Operation(const char* mnemonic, typename OpFunction<OpType::Arithmetic>::type arithmeticOpFunction, bool writeResult = true)
			: arithmeticOp(mnemonic, arithmeticOpFunction, writeResult), logicalOp(), selectedOperationType(OpType::Arithmetic) {}
	};

	class RequestInput {
	public:
		enum class ValueType {
			Register,
			Immediate,
		};
		enum class ShiftType {
			LogicalShiftLeft = 0b00,
			LogicalShiftRight = 0b01,
			ArithmeticShiftRight = 0b10,
			RotateRight = 0b11
		};
		
		ValueType type = ValueType::Immediate;
		union {
			struct {
				word immediateValue;
				uint8_t immediateRotation;
			};
			uint8_t registerIndex;
		};
		struct {
			bool enabled = false;
			ValueType amountType;
			union {
				word amountImmediateValue;
				uint8_t amountRegisterIndex;
			};
			ShiftType shiftType;
		} shift;
		
		word Evaluate(Instructions::InstructionRegisterInterface, int* shifterCarryOut = nullptr);

		friend std::ostream& operator << (std::ostream&, RequestInput&);
	};

	class Request {
	public:
		const Operation* op = nullptr;
		
		RequestInput operand1;
		RequestInput operand2;
		uint8_t destinationRegister;

		bool setFlags = true;

		void Evaluate(Instructions::InstructionRegisterInterface);

		friend std::ostream& operator << (std::ostream&, Request&);
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
