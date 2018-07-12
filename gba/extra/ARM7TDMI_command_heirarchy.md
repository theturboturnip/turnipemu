When interpreting the ARM32 instruction set, there is a certain order in which the instructions should be considered. Some instruction categories have a more generous signature than others, for example the BX instruction (Branch and eXchange) also matches the signature of generic ALU instructions. For this reason you should check if a particluar instruction is a BX instruction *before* checking if it's an ALU instruction.

The condition code is considered to be excluded from all representations of the instruction. When reading the patterns the sets are all inclusive, i.e. Bits 27-25 contains 27, 26 and 25 (in that order).

- Software Interrupt (Bits 27-24 = 1111)
- Undefined (Bit 4 = 1, Bits 27-25 = 011)
- Branch and Exchange (Bits 27-4 = 0001-0010-1111-1111-1111-0001)
- Branch (Bits 27-25 = 101)
- Multiply (Bits 27-23 = 0, Bits 7-4 = 1001)
- Single Data Swap (Bits 27-23 = 00010, Bits 21-20 = 00, Bits 11-4 = 00001001)
- Halfword Data Transfers (Bits 27-25 = 000, Bit 7 = 1, Bit 4 = 1)
  - Regster Offset (Bit 22 = 0, Bits 11-8 = 0000)
  - Immediate Offset (Bit 22 = 1)
- Data Processing (Bits 27-26 = 00)
- Single Data Transfer (Bits 27-26 = 01)
- Block Data Transfer (Bits 27-26 = 10)
- Coprocessor Ops (Bits 27-26 = 11)
  - Coprocessor Data Transfer (Bit 25 = 0)
  - Coprocessor Data Operation (Bit 25 = 1, Bit 4 = 0)
  - Coprocessor Register Transfer (Bit 25 = 1, Bit 4 = 1)