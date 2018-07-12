Common Data Transfer Instruction bits (defined as PUWL instructions):
  All Data Transfer instructions have these bits which define common behaviour.
  All Data Transfer Instructions have some method of applying an offset to the address which is initially specified by the instruction. 
  Bit 24 = Pre/Post Indexing. If set, pre-indexing is used, and the offset is applied to the value BEFORE writeback. Otherwise, the offset is applied AFTER writeback. If this bit is clear and writeback bit is set, then the writeback is effectively redundant. However, if the program is in privileged mode then in this scenario 
  Bit 23 = Up/Down bit. If set, the offset is ADDED to the base value, otherwise it's SUBTRACTED.
  Bit 22 is instruction-specific
  Bit 21 = Writeback. If set, the address which is loaded from/stored to is written into the "Base Register".
  Bit 20 = Load Bit. If set, the instruction loads from memory, if cleared, the instruction stores to memory.
  Bits 19-16 = Base Register

BX - Branch and eXchange
   This instruction is like a branch, but it sets the processor mode based on the lowest bit of the branch value. If that bit is set then the processor continues in Thumb mode. Otherwise execution continues in Arm mode.
  This instruction can be detected as a Data Processing instruction, meeting the following conditions:
  - Immediate = 0
  - Opcode = 1001b (9, which is ALU op TEQ)
  - Set Flags = 0
  - Source Register = 15
  - Destination Register = 15
  - Operand = Register 15 logical-shifted left by the Register containing the destination
  However, this is quite specific and placing the logic for detecting BX inside the generic ALU instruction code would be quite confusing for newcomers to your code. I recommend you detect this instruction separately.

Multiply Instructions
  Bit 23 = Long Mode (If this is set, then the result is stored as a 64bit value inside 2 registers. The low register is specified in Bits 15-12, the high register is specified in Bits 19-16)
  Bit 22 = Unsigned Mode (Only used in Long Mode, should always be 0 otherwise. If it's set in Long Mode, the inputs are treated as 2s-complement signed numbers and produce a 2s-complement signed result. Otherwise, the inputs are treated as unsigned and the result is unsigned.)
  Bit 21 = Accumulate (This behaves in different ways depending on the Long Mode. If this bit is set in Long Mode, the value inside the low and high registers is added to the multiplication result. If this bit is set outside of Long Mode, the value inside the register designated by Bits 15-12 is added to the multiplication result.)
  Bit 20 = Set Flags

Swap
  Bit 22 = Swap Byte (If set, the instruction swaps a byte. If reset, it swaps the whole word.)
  Bits 19-16 = Base Register (contains the Swap Address)
  Bits 15-12 = Destination Register
  Bits 3-0 = Source Register.
  The value located at the Swap Address is written to the Destination Register, and the value in the Source Register is then written to the Swap Address.

Halfword/Byte Data Transfers
    PUWL Bits are used here
    Bits 15-12 = Source/Destination Register
    Bit 6 = Sign-Extension Bit (Must be a store instruction if this is set. If set, the sign of the loaded value is extended, i.e. if a byte is loaded all bits in the destination register after Bit 7 are equal to bit 7.)
    Bit 5 = Data Selection Bit (If set, the instruction loads/stores halfwords, otherwise it uses bytes.)
  - Register Offset
  	Bits 11-8 = Unused, should be all 0's
	Bits 3-0 = Offset Register
  - Immediate Offset
  	Bits 11-8 = Top nybble of immediate offset
    Bits 3-0 = Bottom nybble of immediate offset.

Single/Block Data Transfers use PUWL bits, as well as the Coprocessor Data Transfer