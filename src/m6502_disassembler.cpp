#include "m6502_disassembler.h"

#include <iostream>

/*
 * Disassembles a block of code. It starts at `block->start` and makes it's way
 * through memory till it either reaches an invalid opcode, reaches end of memory,
 * reaches a `brk` instruction, or reaches another block's starting address.
 *
 * Returns a vector of jump destinations. I.e. a list of all addresses where the
 * execution could jump to (via jmp, bne, jsr, etc.).
 */
std::vector<u16> Disassembler::disassemble_block(Block *block)
{
    std::vector<u16> jmp_destinations;
    const u8 *block_start_ptr = this->mem.data + block->start;
    u8 *it = this->mem.data + block->start;

    while (true) {
        u16 offset = (u16) (it - block_start_ptr); // fel. -1
        u8 op_code = *it++;
        InstructionInfo info = instruction_info_table[op_code];

        if (info.mnemonic == "-") {
            break;
        }

        // TODO look a few bytes ahead. brk is still a valid instruction
        if (info.mnemonic == "brk") {
            block->code.push_back({info.mnemonic, 1}); 
            break;
        }

        if (block->start + offset > Layout::FREE_ROM_HIGH) {
            break;
        }

        if (false /* intersection with other block*/) {
            // TODO merge
        }
        
        block->code.resize((size_t) offset + 1);
        //block->code[offset] = info.mnemonic + info.addr_parser(&it); 
        if (offset == 0)
            std::cout << "> 0x" << std::hex << (block->start + offset) << std::dec << ":  " << info.mnemonic + " " + info.addr_parser(&it) << std::endl;
        else
            std::cout << "  0x" << std::hex << (block->start + offset) << std::dec << ":  " << info.mnemonic + " " + info.addr_parser(&it) << std::endl;
    }

    return jmp_destinations; 
}

/*
 * Disassembles the program pointed to by the reset vector.
 * Will recursively enter
 */
void Disassembler::disassemble()
{
    u16 addr = mem[Layout::RESET_VECTOR] + 
              (mem[Layout::RESET_VECTOR + 1] << 8);

    Block entry(addr);
    std::vector<u16> jmps = this->disassemble_block(&entry);

    this->code_blocks.push_back(entry);
}

std::string byte_to_hex(u8 b)
{
    char const nibble[16] = {
        '0', '1', '2', '3', '4', '5', '6', '7',
        '8', '9', 'A', 'B', 'C', 'D', 'E', 'F'
    };

    std::string hexstr;
    hexstr += nibble[(b & 0xF0) >> 4];
    hexstr += nibble[(b & 0x0F)];

    return hexstr;
}

/*
 * The addr_* functions correspond to the different addressing modes off the 
 * m6502. Each implements a small parser for the arguments beloning to an 
 * op-code.
 */
std::string addr_acc(u8 **bytes) 
{
    return "A";
}

std::string addr_abs(u8 **bytes)
{
    u8 ll = *(*bytes)++;
    u8 hh = *(*bytes)++;
    return "$" + byte_to_hex(hh) + byte_to_hex(ll);
}

std::string addr_abs_X(u8 **bytes)
{
    return addr_abs(bytes) + ", X";
}

std::string addr_abs_Y(u8 **bytes)
{
    return addr_abs(bytes) + ", Y";
}

std::string addr_imm(u8 **bytes)
{
    return "#$" + byte_to_hex(*(*bytes)++);
}

std::string addr_impl(u8 **bytes)
{
    return "";
}

std::string addr_ind(u8 **bytes)
{
    u8 ll = *(*bytes)++;
    u8 hh = *(*bytes)++;
    return "($" + byte_to_hex(hh) + byte_to_hex(ll) + ")";
}

std::string addr_ind_X(u8 **bytes)
{
    u8 ll = *(*bytes)++;
    return "($" + byte_to_hex(ll) + ", X)";
}

std::string addr_ind_Y(u8 **bytes)
{
    u8 ll = *(*bytes)++;
    return "($" + byte_to_hex(ll) + "), Y";
}

std::string addr_rel(u8 **bytes)
{
    u8 bb = *(*bytes)++;
    return "$" + byte_to_hex(bb);
}

std::string addr_zpg(u8 **bytes)
{
    u8 ll = *(*bytes)++;
    return "$" + byte_to_hex(ll);
}

std::string addr_zpg_X(u8 **bytes)
{
    u8 ll = *(*bytes)++;
    return "$" + byte_to_hex(ll);
}

std::string addr_zpg_Y(u8 **bytes)
{
    u8 ll = *(*bytes)++;
    return "$" + byte_to_hex(ll);
}

/*
 * Populates the instruction_info_table.
 */
void Disassembler::populate_instruction_info_table()
{

    // "zero" fill
    for (int i = 0; i < 256; i++)
        instruction_info_table[i] = {nullptr, "-"};

    // populate
    // row 0
    this->instruction_info_table[0x00] = {addr_impl,  "brk"};
    this->instruction_info_table[0x01] = {addr_ind_X, "ora"};
    this->instruction_info_table[0x05] = {addr_zpg,   "ora"};
    this->instruction_info_table[0x06] = {addr_zpg,   "asl"};
    this->instruction_info_table[0x08] = {addr_impl,  "php"};
    this->instruction_info_table[0x09] = {addr_imm,   "ora"};
    this->instruction_info_table[0x0A] = {addr_acc,   "asl"};
    this->instruction_info_table[0x0D] = {addr_abs,   "ora"};
    this->instruction_info_table[0x0E] = {addr_abs,   "asl"};
                                                                    
    // row 1                                                        
    this->instruction_info_table[0x10] = {addr_impl,  "bpl"};
    this->instruction_info_table[0x11] = {addr_ind_Y, "ora"};
    this->instruction_info_table[0x15] = {addr_zpg_X, "ora"};
    this->instruction_info_table[0x16] = {addr_zpg_X, "asl"};
    this->instruction_info_table[0x18] = {addr_impl,  "clc"};
    this->instruction_info_table[0x19] = {addr_abs_Y, "ora"};
    this->instruction_info_table[0x1D] = {addr_abs_X, "ora"};
    this->instruction_info_table[0x1E] = {addr_abs_X, "asl"};
                                                                   
    // row 2                                                       
    this->instruction_info_table[0x20] = {addr_abs,   "jsr"};
    this->instruction_info_table[0x21] = {addr_ind_X, "and"};
    this->instruction_info_table[0x24] = {addr_zpg,   "bit"};
    this->instruction_info_table[0x25] = {addr_zpg,   "and"};
    this->instruction_info_table[0x26] = {addr_zpg,   "rol"};
    this->instruction_info_table[0x28] = {addr_impl,  "plp"};
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
    this->instruction_info_table[0x38] = {addr_impl,  "sec"};
    this->instruction_info_table[0x39] = {addr_abs_Y, "and"};
    this->instruction_info_table[0x3D] = {addr_abs_X, "and"};
    this->instruction_info_table[0x3E] = {addr_abs_X, "rol"};
                                                                   
    // row 4                                                       
    this->instruction_info_table[0x40] = {addr_impl,  "rti"};
    this->instruction_info_table[0x41] = {addr_ind_X, "eor"};
    this->instruction_info_table[0x45] = {addr_zpg,   "e0r"};
    this->instruction_info_table[0x46] = {addr_zpg,   "lsr"};
    this->instruction_info_table[0x48] = {addr_impl,  "pha"};
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
    this->instruction_info_table[0x58] = {addr_impl,  "cli"}; // TODO ???
    this->instruction_info_table[0x59] = {addr_impl,  "cli"};
    this->instruction_info_table[0x5D] = {addr_impl,  "cli"};
    this->instruction_info_table[0x5E] = {addr_impl,  "cli"};
                                                                   
    // row 6                                                       
    this->instruction_info_table[0x60] = {addr_rel,   "bvc"};
    this->instruction_info_table[0x61] = {addr_ind_Y, "eor"};
    this->instruction_info_table[0x65] = {addr_zpg_X, "eor"};
    this->instruction_info_table[0x66] = {addr_zpg_X, "lsr"};
    this->instruction_info_table[0x68] = {addr_impl,  "cli"};
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
    this->instruction_info_table[0x78] = {addr_impl,  "sei"};
    this->instruction_info_table[0x79] = {addr_abs_Y, "adc"};
    this->instruction_info_table[0x7D] = {addr_abs_X, "adc"};
    this->instruction_info_table[0x7E] = {addr_abs_X, "ror"};
                                                                   
    // row 8                                                       
    this->instruction_info_table[0x81] = {addr_ind_X, "sta"};
    this->instruction_info_table[0x84] = {addr_zpg,   "sty"};
    this->instruction_info_table[0x85] = {addr_zpg,   "sta"};
    this->instruction_info_table[0x86] = {addr_zpg,   "stx"};
    this->instruction_info_table[0x88] = {addr_impl,  "dey"};
    this->instruction_info_table[0x8A] = {addr_impl,  "txa"};
    this->instruction_info_table[0x8C] = {addr_abs,   "sty"};
    this->instruction_info_table[0x8D] = {addr_abs,   "sta"};
    this->instruction_info_table[0x8E] = {addr_abs,   "stx"};
                                                                    
    // row 9                                                        
    this->instruction_info_table[0x90] = {addr_rel,   "bcc"};
    this->instruction_info_table[0x91] = {addr_ind_Y, "sta"};
    this->instruction_info_table[0x94] = {addr_zpg_X, "sty"};
    this->instruction_info_table[0x95] = {addr_zpg_X, "sta"};
    this->instruction_info_table[0x96] = {addr_zpg_Y, "stx"};
    this->instruction_info_table[0x98] = {addr_impl,  "tya"};
    this->instruction_info_table[0x99] = {addr_abs_Y, "sta"};
    this->instruction_info_table[0x9A] = {addr_impl,  "txs"};
    this->instruction_info_table[0x9D] = {addr_abs_X, "sta"};
                                                                   
    // row A                                                       
    this->instruction_info_table[0xA0] = {addr_imm,   "ldy"};
    this->instruction_info_table[0xA1] = {addr_ind_X, "lda"};
    this->instruction_info_table[0xA2] = {addr_imm,   "ldx"};
    this->instruction_info_table[0xA4] = {addr_zpg,   "ldy"};
    this->instruction_info_table[0xA5] = {addr_zpg,   "lda"};
    this->instruction_info_table[0xA6] = {addr_zpg,   "ldx"};
    this->instruction_info_table[0xA8] = {addr_impl,  "tay"};
    this->instruction_info_table[0xA9] = {addr_imm,   "lda"};
    this->instruction_info_table[0xAA] = {addr_impl,  "tax"};
    this->instruction_info_table[0xAC] = {addr_abs,   "ldy"};
    this->instruction_info_table[0xAD] = {addr_abs,   "lda"};
    this->instruction_info_table[0xAE] = {addr_abs,   "ldx"};
                                                                   
    // row B                                                       
    this->instruction_info_table[0xB0] = {addr_rel,   "bcs"};
    this->instruction_info_table[0xB1] = {addr_ind_Y, "lda"};
    this->instruction_info_table[0xB4] = {addr_zpg_X, "ldy"};
    this->instruction_info_table[0xB5] = {addr_zpg_X, "lda"};
    this->instruction_info_table[0xB6] = {addr_zpg_Y, "ldx"};
    this->instruction_info_table[0xB8] = {addr_impl,  "clv"};
    this->instruction_info_table[0xB9] = {addr_abs_Y, "lda"};
    this->instruction_info_table[0xBA] = {addr_impl,  "tsx"};
    this->instruction_info_table[0xBC] = {addr_abs_X, "ldy"};
    this->instruction_info_table[0xBD] = {addr_abs_X, "lda"};
    this->instruction_info_table[0xBE] = {addr_abs_Y, "ldx"};
                                                                   
    // row C                                                       
    this->instruction_info_table[0xC0] = {addr_imm,   "cpy"};
    this->instruction_info_table[0xC1] = {addr_ind_X, "cmp"};
    this->instruction_info_table[0xC4] = {addr_zpg,   "cpy"};
    this->instruction_info_table[0xC5] = {addr_zpg,   "cmp"};
    this->instruction_info_table[0xC6] = {addr_zpg,   "dec"};
    this->instruction_info_table[0xC8] = {addr_impl,  "iny"};
    this->instruction_info_table[0xC9] = {addr_imm,   "cmp"};
    this->instruction_info_table[0xCA] = {addr_impl,  "dex"};
    this->instruction_info_table[0xCC] = {addr_abs,   "cpy"};
    this->instruction_info_table[0xCD] = {addr_abs,   "cmp"};
    this->instruction_info_table[0xCE] = {addr_abs,   "dec"};
                                                                   
    // row D                                                       
    this->instruction_info_table[0xD0] = {addr_rel,   "bne"};
    this->instruction_info_table[0xD1] = {addr_ind_Y, "cmp"};
    this->instruction_info_table[0xD5] = {addr_zpg_X, "cmp"};
    this->instruction_info_table[0xD6] = {addr_zpg_X, "dec"};
    this->instruction_info_table[0xD8] = {addr_impl,  "cld"};
    this->instruction_info_table[0xD9] = {addr_abs_Y, "cmp"};
    this->instruction_info_table[0xDD] = {addr_abs_X, "cmp"};
    this->instruction_info_table[0xDE] = {addr_abs_X, "dec"};
                                                                   
    // row E                                                       
    this->instruction_info_table[0xE0] = {addr_imm,   "cpx"};
    this->instruction_info_table[0xE1] = {addr_ind_X, "sbc"};
    this->instruction_info_table[0xE4] = {addr_zpg,   "cpx"};
    this->instruction_info_table[0xE5] = {addr_zpg,   "sbc"};
    this->instruction_info_table[0xE6] = {addr_zpg,   "inc"};
    this->instruction_info_table[0xE8] = {addr_impl,  "inx"};
    this->instruction_info_table[0xE9] = {addr_imm,   "sbc"};
    this->instruction_info_table[0xEA] = {addr_impl,  "nop"};
    this->instruction_info_table[0xEC] = {addr_abs,   "cpx"};
    this->instruction_info_table[0xED] = {addr_abs,   "sbc"};
    this->instruction_info_table[0xEE] = {addr_abs,   "inc"};
                                                                   
    // row F                                                       
    this->instruction_info_table[0xF0] = {addr_rel,   "beq"};
    this->instruction_info_table[0xF1] = {addr_ind_Y, "sbc"};
    this->instruction_info_table[0xF5] = {addr_zpg_X, "sbc"};
    this->instruction_info_table[0xF6] = {addr_zpg_X, "inc"};
    this->instruction_info_table[0xF8] = {addr_impl,  "sed"};
    this->instruction_info_table[0xF9] = {addr_abs_Y, "sbc"};
    this->instruction_info_table[0xFD] = {addr_abs_X, "sbc"};
    this->instruction_info_table[0xFE] = {addr_abs_X, "inc"};
}
