#pragma once

#include "turnipemu/display.h"
#include "turnipemu/log.h"
#include "turnipemu/types.h"
#include "turnipemu/memory/map.h"

#include <string>
#include <memory>
#include <functional>

namespace TurnipEmu::ARM7TDMI {
	enum class CPUExecState : bool {
		ARM = false,
		Thumb = true
	};
	enum class Mode : uint8_t {
		User       = 0b10000,
		FIQ        = 0b10001, // TODO: What is this?
		IRQ        = 0b10010, // TODO: What is this?
		Supervisor = 0b10011,
		Abort      = 0b10111,
		Undefined  = 0b11011,
		System     = 0b11111,
	};
	inline std::string ModeString(Mode m){
		switch(m){
		case Mode::User: return "User";
		case Mode::FIQ: return "FIQ";
		case Mode::IRQ: return "IRQ";
		case Mode::Supervisor: return "Supervisor";
		case Mode::Abort: return "Abort";
		case Mode::Undefined: return "Undefined";
		case Mode::System: return "System";
		}
		return "Invalid";
	}

	// Integer value = priority, lower is better.
	// e.x. a FIQ exception is handled BEFORE an UndefinedInstruction exception
	enum class Exception {
		Reset = 0,
		DataAbort = 1,
		FIQ = 2,
		IRQ = 3,
		PrefetchAbort = 4,
		UndefinedInstruction = 5,
		SoftwareInterrupt = 6
	};
	struct ExceptionData {
		std::string name;
		word handlerAddress;
		Mode modeOnEntry;
	};
	const static std::array<ExceptionData, 7> exceptionsData  = {{
		{ "Reset",                 0x00, Mode::Supervisor },
		{ "Data Abort",            0x10, Mode::Abort      },
		{ "FIQ",                   0x1C, Mode::FIQ        },
		{ "IRQ",                   0x18, Mode::IRQ        },
		{ "Prefetch Abort",        0x0C, Mode::Abort      },
		{ "Undefined Instruction", 0x04, Mode::Undefined  },
		{ "Software Interrupt",    0x08, Mode::Supervisor },
		}};
	
	union ProgramStatusRegister{
		word value;
#pragma pack(1) // No Padding
		struct {
			Mode mode : 5; // Bits 0-4
			CPUExecState state : 1; // Bit 5, determines if in Thumb or Arm mode
			bool fiqDisable : 1; // Bit 6
			bool irqDisable : 1; // Bit 7
			uint32_t reserved : 20; // Bits 8 through 27 inclusive
			bool overflow : 1;
			bool carry : 1;
			bool zero : 1;
			bool negative : 1;
		};
#pragma pack() // Reset packing mode
	};
	static_assert(sizeof(ProgramStatusRegister) == 4, "ProgramStatusRegister must be a single word");
		
    // This is used by the instructions 
	struct RegisterPointers{
		word* main[16] = {nullptr};
			
		ProgramStatusRegister* cpsr = nullptr;
		ProgramStatusRegister* spsr = nullptr; // Not enabled in System/User mode
	};

	class CPU;

	struct InstructionMask {
		struct MaskRange {
			const uint8_t end;
			const uint8_t start;
			const word mask;
			const word value;

			MaskRange(uint8_t end, uint8_t start, uint32_t value) : end(end), start(start), mask((word(~0) << start) & (word(~0) >> (32 - end - 1))), value((value << start) & mask)  {
				assert(start <= end);
			}
			MaskRange(uint8_t bit, bool set) : MaskRange(bit, bit, set){}
				
			inline void updateMask(word& theirMask) const {
				theirMask |= mask; 
			}
			inline void updateExpectedValue(word& expectedValue) const {
				expectedValue |= value;
			}
		};
		word mask;
		word expectedValue;
		InstructionMask(std::initializer_list<MaskRange>);
		inline bool matches(word input) const {
			return (input & mask) == expectedValue;
		}
	};
	class Instruction {
	public:
		struct Condition {
			char name[2];
			std::string debugString;
			std::function<bool(ProgramStatusRegister)> fulfilsCondition;
		};
			
		Instruction(std::string category, InstructionMask mask);
		virtual ~Instruction(){}
			
		const Condition& getCondition(word instructionWord);
			
		virtual std::string disassembly(word instructionWord){
			return "NO DISASSEMBLY PRESENT";
		}
		virtual void execute(CPU& cpu, const RegisterPointers, word instructionWord){
			throw std::runtime_error("Instruction is not implemented!");
		}

		const std::string category;
		const InstructionMask mask;
	protected:
		const static std::array<const Condition, 15> conditions;
	};
	
	class CPU : public Display::CustomWindow {
	public:	   
		CPU(const Memory::Map& memoryMap);
		
		void tick();

		void queuePipelineFlush();
		
		void addCycles(uint32_t cycles);

		void reset();

		void drawCustomWindowContents() override;

		const Memory::Map& memoryMap;
	protected:		
		// Determines the register pointers for the current state, taking into account the execution state and current instruction type
		const RegisterPointers registersForCurrentState();
		
		struct{
			word main[16];

			ProgramStatusRegister cpsr;
			
			struct{
				word r8;
				word r9;
				word r10;
				word r11;
				word r12;
				word r13;
				word r14;

				ProgramStatusRegister spsr;
			} fiq;
			struct{
				word r13;
				word r14;

				ProgramStatusRegister spsr;
			} svc;
			struct{
				word r13;
				word r14;

				ProgramStatusRegister spsr;
			} abt;
			struct{
				word r13;
				word r14;

				ProgramStatusRegister spsr;
			} irq;
			struct{
				word r13;
				word r14;

				ProgramStatusRegister spsr;
			} und;
		} registers;

		uint32_t cyclesThisTick;
		uint32_t cyclesTotal;

		struct {
			bool hasFetchedInstruction;
			word fetchedInstructionWord;
			word fetchedInstructionAddress;
			
			bool hasDecodedInstruction;
			Instruction* decodedInstruction;
			word decodedInstructionWord;
			word decodedInstructionAddress;

			bool hasExecutedInstruction;
			word executedInstructionAddress;

			bool queuedFlush;
		} pipeline;
		void flushPipeline();
		void tickPipeline();
		
		// This has to be vector of unique_ptr because Instruction is virtual
		std::vector<std::unique_ptr<Instruction>> instructions;
		void setupInstructions();
		Instruction* matchInstruction(word instructionWord);
	};
}
