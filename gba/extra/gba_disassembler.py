#!/usr/bin/env python

import sys

rom_path = sys.argv[1]
with open(rom_path, "rb") as f:
    rom_data = f.read()

print(len(rom_data))

pc = 0
in_thumb_mode = False

class ArmInstruction:
    CONDITION_STRINGS = [
        "EQ (Equal i.e. Z set)",
        "NE (Not Equal i.e. Z clear)",
        "CS (Unsigned higher/same i.e. C set)",
        "CC (Unsigned lower i.e. C clear)",
        "MI (Negative i.e. N set)",
        "PL (Positive or Zero i.e. N clear)",
        "VS (Overflow i.e. V set)",
        "VC (No overflow i.e. V clear)",
        "HI (Unsigned higher i.e. C set and Z clear)",
        "LS (Unsigned lower or equal i.e. C clear and Z set)",
        "GE (Greater or equal i.e. N=V)",
        "LT (Less than i.e. N!=V)",
        "GT (Greater than i.e Z clear and N=V)",
        "LE (Less than or equal i.e. Z set or N!=V)",
        "AL (Always)",
    ]

    @staticmethod
    def fits(word):
        return True
    
    def __init__(self, word):
        self.condition = word >> 28
        self.condition_string = self.CONDITION_STRINGS[self.condition]

    def changes_pc(self):
        return False
    def action_string(self):
        return "Defined but not inplemented"
    
    def __str__(self):
        return self.action_string() + "\n[ COND ] " + self.condition_string
class ArmPUWLInstruction(ArmInstruction):
    def __init__(self, word):
        ArmInstruction.__init__(self, word)
        self.P = (word >> 24) & 1
        self.U = (word >> 23) & 1
        self.writeback = (word >> 21) & 1
        self.load = (word >> 20) & 1 # Previously L
class ArmUndefinedInstruction(ArmInstruction):
    @staticmethod
    def fits(word):
        return ((word >> 25) & 0b111) == 0b011 and ((word >> 4) & 1) == 1
    def action_string(self):
        return "Undefined"
class ArmBranchInstruction(ArmInstruction):
    @staticmethod
    def fits(word):
        return ((word >> 25) & 0b111) == 0b101

    def __init__(self, word):
        ArmInstruction.__init__(self, word)
        self.branch_offset = (word & ~(0xFF << 24)) << 2
        self.store_old_in_link = (word >> 24) & 1
    def changes_pc(self):
        return ["delta", self.branch_offset]
    def action_string(self):
        return "Branch by %d, stores old in link = %d" % (self.branch_offset, self.store_old_in_link)
class ArmBranchExchangeInstruction(ArmInstruction):
    @staticmethod
    def fits(word):
        return ((word >> 4) & 0xFFFFFF) == 0b000100101111111111110001
    def __init__(self, word):
        global in_thumb_mode
        ArmInstruction.__init__(self, word)
        self.source_register = ((word >> 4) & 0xF)
    def action_string(self):
        return "Branch to register {}, set thumb mode to bit 0 of that".format(self.source_register)
class ArmALUInstruction(ArmInstruction):
    OPCODE_STRINGS = [
        "AND (src AND operand)",
        "EOR (src XOR operand)",
        "SUB (src - operand)",
        "RSB (operand - sub)",
        "ADD (src + operand)",
        "ADC (src + operand with carry)",
        "SBC (src - operand with carry)",
        "RSC (operand - src with carry)",
        "TST (Set flags with AND result)",
        "TEQ (Set flags with XOR result)",
        "CMP (Set flags on src - operand)",
        "CMN (Set flags on src + operand)",
        "ORR (src OR operand)",
        "MOV (MOV operand to dest)",
        "BIC (src AND NOT operand)",
        "MVN (NOT operand)"
    ]
    
    @staticmethod
    def fits(word):
        if ((word >> 25) & 0b111) == 0b001:
            return True
        if ((word >> 25) & 0b111) != 0b000:
            return False
        if ArmBranchExchangeInstruction.fits(word):
            return False
        middle_of_operand = word >> 4 & 0xF
        invalid_middle_of_operands = [0b1001, 0b1011, 0b1101, 0b1111]
        if middle_of_operand in invalid_middle_of_operands:
            return False
        return True
    def __init__(self, word):
        ArmInstruction.__init__(self, word)
        self.immediate = (word >> 25) & 1
        self.opcode = (word >> 21) & 0xF
        self.set_flags = (word >> 20) & 1
        
        self.r_register = (word >> 16) & 0xF
        self.destination_register = (word >> 12) & 0xF

        if self.immediate:
            rotation = ((word >> 8) & 0xF) * 2
            byte_value = word & 0xFF
            self.operand = (byte_value >> rotation) + (((byte_value << 32) >> rotation) & 0xFFFFFFFF)
            if rotation != 0:
                pass # The carry bit is set to bit 31 of self.operand in this case
        else:
            self.operand_transform_type = ""
            self.operand_transform_amount = "" # int value or "register N"
            full_operand = (word & 0xFFF)
            register_as_transform = (full_operand >> 4) & 1
            if register_as_transform:
                self.operand_transform_amount = "register {}".format((full_operand >> 8) & 0xF)
            else:
                self.operand_transform_amount = (full_operand >> 7) & 0b11111
            OPERAND_TRANSFORM_TYPES = ["logical_shift_left", "logical_shift_right", "arithmetic_shift_right", "rotate_right"]
            self.operand_transform_type = OPERAND_TRANSFORM_TYPES[(full_operand >> 5) & 0b11]
            if ((full_operand >> 5) & 0b11 == 0b11) and self.operand_transform_amount == 0 and not register_as_transform:
                self.operand_transform_type = "rotate_right_with_extend"
            self.operand_register = full_operand & 0xF

        if self.opcode == 0b1001 and not self.set_flags:
            self.opcode_string = "TEQP (Move SPSR to CPSR IF in privileged mode)" 
        else:
            self.opcode_string = self.OPCODE_STRINGS[self.opcode]
            
    def changes_pc(self):
        if (self.destination_register == 15):
            return False # TODO: Update PC if possible?
        return False
    def action_string(self):
        operand_string = ""
        if self.immediate:
            operand_string = "immediate {}".format(self.operand)
        else:
            operand_string = "register {0} with transformation {1} by {2}".format(self.operand_register, self.operand_transform_type, self.operand_transform_amount)
        return "ALU Command {0} from register {1} to register {2} with operand '{3}', set flags:{4}".format(self.opcode_string, self.r_register, self.destination_register, operand_string, self.set_flags)

arm_instruction_types = [ArmBranchInstruction, ArmBranchExchangeInstruction, ArmALUInstruction, ArmUndefinedInstruction]

def disassemble_current():
    global pc, rom_data

    if (in_thumb_mode):
        assert pc % 2 == 0
    else:
        assert pc % 4 == 0
    
        instruction_word = (rom_data[pc + 3] << 24) + (rom_data[pc + 2] << 16) + (rom_data[pc + 1] << 8) + (rom_data[pc + 0])
        disassembled_instruction = ArmInstruction(instruction_word)
        for arm_instruction_type in arm_instruction_types:
            if (arm_instruction_type.fits(instruction_word)):
                disassembled_instruction = arm_instruction_type(instruction_word)
        # TODO: If reg 15 is written to by some constant then apply the change
        print("[0x{0:04x}] {1:04b} {2:0b}".format(pc, instruction_word >> 28, instruction_word & ~(0xF << 28)))
        print("[ARM32 ] %s" % str(disassembled_instruction))
        pc += 4
        pc_change = disassembled_instruction.changes_pc()
        new_pc = pc
        if pc_change != False:
            if pc_change[0] == "delta":
                # When PC is incremented it has to be 2 instructions further because of how pipelining worked
                new_pc += pc_change[1] + 4 * 2
            elif pc_change[0] == "abs":
                new_pc = pc_change[1]
            else:
                print("Invalid pc_change %s" % pc_change[0])
            if new_pc != pc:
                pc = new_pc
                print("New PC: 0x%04x" % pc)

while True:
    disassemble_current()
    command = input("> ").lower().strip()
    if command in ["stop", "quit", "exit"]:
        break
    elif command == "thumb":
        in_thumb_mode = True
    elif command == "arm":
        in_thumb_mode = False
