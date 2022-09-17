#include "m6502_disassembler.h"

#include <iostream>

/*
 * Disassembles the program pointed to by the reset vector
 */
void Disassembler::disassemble()
{
    u16 addr = mem[Layout::RESET_VECTOR] + 
              (mem[Layout::RESET_VECTOR + 1] << 8);
    std::cout << addr << std::endl;
}

    //enum AddrModeType 
    //{
    //    IMPL,
    //    ACC,
    //    ABS,
    //    ABS_X,
    //    ABS_Y,
    //    IMM,
    //    IND,
    //    IND_X,
    //    IND_X,
    //    REL,
    //    ZPG,
    //    ZPG_X,
    //    ZPG_Y
    //};
    ///*
    // * The `InstructionInfo` struct and `instruction_info_table` contains all
    // * the information needed to interpret a series of consecutive bytes as
    // * plaintext m6502 assembly.
    // */
    //struct InstructionInfo 
    //{
    //    // A mnemonic
    //    std::string mnemonic;

    //    // An addressing mode type. This determines the formatting of the 
    //    // disassembled instruction string and the byte size of the instruction.
    //    AddrModeType addr_mode_type;
    //};


std::string addr_acc(u8 *bytes) 
{
    return "asdf";
}
std::string addr_abs(u8 *bytes)
{
    return "asdf";
}
std::string addr_abs_X(u8 *bytes)
{
    return "asdf";
}
std::string addr_abs_Y(u8 *bytes)
{
    return "asdf";
}
std::string addr_imm(u8 *bytes)
{
    return "asdf";
}
std::string addr_impl(u8 *bytes)
{
    return "asdf";
}
std::string addr_ind(u8 *bytes)
{
    return "asdf";
}
std::string addr_ind_X(u8 *bytes)
{
    return "asdf";
}
std::string addr_ind_Y(u8 *bytes)
{
    return "asdf";
}
std::string addr_rel(u8 *bytes)
{
    return "asdf";
}
std::string addr_zpg(u8 *bytes)
{
    return "asdf";
}
std::string addr_zpg_X(u8 *bytes)
{
    return "asdf";
}
std::string addr_zpg_Y(u8 *bytes)
{
    return "asdf";
}

/*
 * Populates the instruction_info_table.
 */
void Disassembler::populate_instruction_info_table()
{
    const auto IMPLIED = nullptr;

    // "zero" fill
    for (int i = 0; i < 256; i++)
        instruction_info_table[i] = {IMPLIED, "-"};

    // populate
    // row 0
    this->instruction_info_table[0x00] = {IMPLIED,    "brk"};
    this->instruction_info_table[0x01] = {addr_ind_X, "ora"};
    this->instruction_info_table[0x05] = {addr_zpg,   "ora"};
    this->instruction_info_table[0x06] = {addr_zpg,   "asl"};
    this->instruction_info_table[0x08] = {IMPLIED,    "php"};
    this->instruction_info_table[0x09] = {addr_imm,   "ora"};
    this->instruction_info_table[0x0A] = {addr_acc,   "asl"};
    this->instruction_info_table[0x0D] = {addr_abs,   "ora"};
    this->instruction_info_table[0x0E] = {addr_abs,   "asl"};
                                                                    
    // row 1                                                        
    this->instruction_info_table[0x10] = {IMPLIED,    "bpl"};
    this->instruction_info_table[0x11] = {addr_ind_Y, "ora"};
    this->instruction_info_table[0x15] = {addr_zpg_X, "ora"};
    this->instruction_info_table[0x16] = {addr_zpg_X, "asl"};
    this->instruction_info_table[0x18] = {IMPLIED,    "clc"};
    this->instruction_info_table[0x19] = {addr_abs_Y, "ora"};
    this->instruction_info_table[0x1D] = {addr_abs_X, "ora"};
    this->instruction_info_table[0x1E] = {addr_abs_X, "asl"};
                                                                   
    // row 2                                                       
    this->instruction_info_table[0x20] = {addr_abs,   "jsr"};
    this->instruction_info_table[0x21] = {addr_ind_X, "and"};
    this->instruction_info_table[0x24] = {addr_zpg,   "bit"};
    this->instruction_info_table[0x25] = {addr_zpg,   "and"};
    this->instruction_info_table[0x26] = {addr_zpg,   "rol"};
    this->instruction_info_table[0x28] = {IMPLIED,    "plp"};
    this->instruction_info_table[0x29] = {addr_imm,   "and"};
    this->instruction_info_table[0x2A] = {addr_acc,   "rol"};
    this->instruction_info_table[0x2C] = {addr_abs,   "bit"};
    this->instruction_info_table[0x2D] = {addr_abs,   "and"};
    this->instruction_info_table[0x2E] = {addr_abs,   "rol"};
                                                                   
    // row 3                                                       
    this->instruction_info_table[0x30] = {addr_rel,   "bmi"};
    this->instruction_info_table[0x31] = {addr_ind_Y, "and"};
    this->instruction_info_table[0x35] = {addr_zpg_X, "and"};
    this->instruction_info_table[0x36] = {addr_zpg_X, "rol"};
    this->instruction_info_table[0x38] = {IMPLIED,    "sec"};
    this->instruction_info_table[0x39] = {addr_abs_Y, "and"};
    this->instruction_info_table[0x3D] = {addr_abs_X, "and"};
    this->instruction_info_table[0x3E] = {addr_abs_X, "rol"};
                                                                   
    // row 4                                                       
    this->instruction_info_table[0x40] = {IMPLIED,    "rti"};
    this->instruction_info_table[0x41] = {addr_ind_X, "eor"};
    this->instruction_info_table[0x45] = {addr_zpg,   "e0r"};
    this->instruction_info_table[0x46] = {addr_zpg,   "lsr"};
    this->instruction_info_table[0x48] = {IMPLIED,    "pha"};
    this->instruction_info_table[0x49] = {addr_imm,   "eor"};
    this->instruction_info_table[0x4A] = {addr_acc,   "lsr"};
    this->instruction_info_table[0x4C] = {addr_abs,   "jmp"};
    this->instruction_info_table[0x4D] = {addr_abs,   "eor"};
    this->instruction_info_table[0x4E] = {addr_abs,   "lsr"};
                                                                   
    // row 5                                                       
    this->instruction_info_table[0x50] = {addr_rel,   "bvc"};
    this->instruction_info_table[0x51] = {addr_ind_Y, "eor"};
    this->instruction_info_table[0x55] = {addr_zpg_X, "eor"};
    this->instruction_info_table[0x56] = {addr_zpg_X, "lsr"};
    this->instruction_info_table[0x58] = {IMPLIED,    "cli"}; // TODO ???
    this->instruction_info_table[0x59] = {IMPLIED,    "cli"};
    this->instruction_info_table[0x5D] = {IMPLIED,    "cli"};
    this->instruction_info_table[0x5E] = {IMPLIED,    "cli"};
                                                                   
    // row 6                                                       
    this->instruction_info_table[0x60] = {addr_rel,   "bvc"};
    this->instruction_info_table[0x61] = {addr_ind_Y, "eor"};
    this->instruction_info_table[0x65] = {addr_zpg_X, "eor"};
    this->instruction_info_table[0x66] = {addr_zpg_X, "lsr"};
    this->instruction_info_table[0x68] = {IMPLIED,    "cli"};
    this->instruction_info_table[0x69] = {addr_imm,   "adc"};
    this->instruction_info_table[0x6A] = {addr_acc,   "ror"};
    this->instruction_info_table[0x6C] = {addr_ind,   "jmp"};
    this->instruction_info_table[0x6D] = {addr_abs,   "adc"};
    this->instruction_info_table[0x6E] = {addr_abs,   "ror"};
                                                                   
    // row 7                                                       
    this->instruction_info_table[0x70] = {addr_rel,   "bvs"};
    this->instruction_info_table[0x71] = {addr_ind_Y, "adc"};
    this->instruction_info_table[0x75] = {addr_zpg_X, "adc"};
    this->instruction_info_table[0x76] = {addr_zpg_X, "ror"};
    this->instruction_info_table[0x78] = {IMPLIED,    "sei"};
    this->instruction_info_table[0x79] = {addr_abs_Y, "adc"};
    this->instruction_info_table[0x7D] = {addr_abs_X, "adc"};
    this->instruction_info_table[0x7E] = {addr_abs_X, "ror"};
                                                                   
    // row 8                                                       
    this->instruction_info_table[0x81] = {addr_ind_X, "sta"};
    this->instruction_info_table[0x84] = {addr_zpg,   "sty"};
    this->instruction_info_table[0x85] = {addr_zpg,   "sta"};
    this->instruction_info_table[0x86] = {addr_zpg,   "stx"};
    this->instruction_info_table[0x88] = {IMPLIED,    "dey"};
    this->instruction_info_table[0x8A] = {IMPLIED,    "txa"};
    this->instruction_info_table[0x8C] = {addr_abs,   "sty"};
    this->instruction_info_table[0x8D] = {addr_abs,   "sta"};
    this->instruction_info_table[0x8E] = {addr_abs,   "stx"};
                                                                    
    // row 9                                                        
    this->instruction_info_table[0x90] = {addr_rel,   "bcc"};
    this->instruction_info_table[0x91] = {addr_ind_Y, "sta"};
    this->instruction_info_table[0x94] = {addr_zpg_X, "sty"};
    this->instruction_info_table[0x95] = {addr_zpg_X, "sta"};
    this->instruction_info_table[0x96] = {addr_zpg_Y, "stx"};
    this->instruction_info_table[0x98] = {IMPLIED,    "tya"};
    this->instruction_info_table[0x99] = {addr_abs_Y, "sta"};
    this->instruction_info_table[0x9A] = {IMPLIED,    "txs"};
    this->instruction_info_table[0x9D] = {addr_abs_X, "sta"};
                                                                   
    // row A                                                       
    this->instruction_info_table[0xA0] = {addr_imm,   "ldy"};
    this->instruction_info_table[0xA1] = {addr_ind_X, "lda"};
    this->instruction_info_table[0xA2] = {addr_imm,   "ldx"};
    this->instruction_info_table[0xA4] = {addr_zpg,   "ldy"};
    this->instruction_info_table[0xA5] = {addr_zpg,   "lda"};
    this->instruction_info_table[0xA6] = {addr_zpg,   "ldx"};
    this->instruction_info_table[0xA8] = {IMPLIED,    "tay"};
    this->instruction_info_table[0xA9] = {addr_imm,   "lda"};
    this->instruction_info_table[0xAA] = {IMPLIED,    "tax"};
    this->instruction_info_table[0xAC] = {addr_abs,   "ldy"};
    this->instruction_info_table[0xAD] = {addr_abs,   "lda"};
    this->instruction_info_table[0xAE] = {addr_abs,   "ldx"};
                                                                   
    // row B                                                       
    this->instruction_info_table[0xB0] = {addr_rel,   "bcs"};
    this->instruction_info_table[0xB1] = {addr_ind_Y, "lda"};
    this->instruction_info_table[0xB4] = {addr_zpg_X, "ldy"};
    this->instruction_info_table[0xB5] = {addr_zpg_X, "lda"};
    this->instruction_info_table[0xB6] = {addr_zpg_Y, "ldx"};
    this->instruction_info_table[0xB8] = {IMPLIED,    "clv"};
    this->instruction_info_table[0xB9] = {addr_abs_Y, "lda"};
    this->instruction_info_table[0xBA] = {IMPLIED,    "tsx"};
    this->instruction_info_table[0xBC] = {addr_abs_X, "ldy"};
    this->instruction_info_table[0xBD] = {addr_abs_X, "lda"};
    this->instruction_info_table[0xBE] = {addr_abs_Y, "ldx"};
                                                                   
    // row C                                                       
    this->instruction_info_table[0xC0] = {addr_imm,   "cpy"};
    this->instruction_info_table[0xC1] = {addr_ind_X, "cmp"};
    this->instruction_info_table[0xC4] = {addr_zpg,   "cpy"};
    this->instruction_info_table[0xC5] = {addr_zpg,   "cmp"};
    this->instruction_info_table[0xC6] = {addr_zpg,   "dec"};
    this->instruction_info_table[0xC8] = {IMPLIED,    "iny"};
    this->instruction_info_table[0xC9] = {addr_imm,   "cmp"};
    this->instruction_info_table[0xCA] = {IMPLIED,    "dex"};
    this->instruction_info_table[0xCC] = {addr_abs,   "cpy"};
    this->instruction_info_table[0xCD] = {addr_abs,   "cmp"};
    this->instruction_info_table[0xCE] = {addr_abs,   "dec"};
                                                                   
    // row D                                                       
    this->instruction_info_table[0xD0] = {addr_rel,   "bne"};
    this->instruction_info_table[0xD1] = {addr_ind_Y, "cmp"};
    this->instruction_info_table[0xD5] = {addr_zpg_X, "cmp"};
    this->instruction_info_table[0xD6] = {addr_zpg_X, "dec"};
    this->instruction_info_table[0xD8] = {IMPLIED,    "cld"};
    this->instruction_info_table[0xD9] = {addr_abs_Y, "cmp"};
    this->instruction_info_table[0xDD] = {addr_abs_X, "cmp"};
    this->instruction_info_table[0xDE] = {addr_abs_X, "dec"};
                                                                   
    // row E                                                       
    this->instruction_info_table[0xE0] = {addr_imm,   "cpx"};
    this->instruction_info_table[0xE1] = {addr_ind_X, "sbc"};
    this->instruction_info_table[0xE4] = {addr_zpg,   "cpx"};
    this->instruction_info_table[0xE5] = {addr_zpg,   "sbc"};
    this->instruction_info_table[0xE6] = {addr_zpg,   "inc"};
    this->instruction_info_table[0xE8] = {IMPLIED,    "inx"};
    this->instruction_info_table[0xE9] = {addr_imm,   "sbc"};
    this->instruction_info_table[0xEA] = {IMPLIED,    "nop"};
    this->instruction_info_table[0xEC] = {addr_abs,   "cpx"};
    this->instruction_info_table[0xED] = {addr_abs,   "sbc"};
    this->instruction_info_table[0xEE] = {addr_abs,   "inc"};
                                                                   
    // row F                                                       
    this->instruction_info_table[0xF0] = {addr_rel,   "beq"};
    this->instruction_info_table[0xF1] = {addr_ind_Y, "sbc"};
    this->instruction_info_table[0xF5] = {addr_zpg_X, "sbc"};
    this->instruction_info_table[0xF6] = {addr_zpg_X, "inc"};
    this->instruction_info_table[0xF8] = {IMPLIED,    "sed"};
    this->instruction_info_table[0xF9] = {addr_abs_Y, "sbc"};
    this->instruction_info_table[0xFD] = {addr_abs_X, "sbc"};
    this->instruction_info_table[0xFE] = {addr_abs_X, "inc"};
}
