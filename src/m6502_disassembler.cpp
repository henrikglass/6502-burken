#include "m6502_disassembler.h"

#include <iostream>

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

std::string get_bytes_str(u8 *data, u16 len)
{
    std::string bytes_str;
    for (int i = 0; i < len; i++) {
        bytes_str += byte_to_hex(data[i]) + " ";
    }
    return bytes_str;
}

/*
 * Puts the disassembly for a page into `*instrs`
 */
void Disassembler::Page::get_disassembly(std::vector<DisassembledInstruction *> *instrs)
{
    for (int offset = this->first_instr_offset; offset < (int)Layout::PAGE_SIZE;) {
        if ((size_t)offset > this->code.size() - 1)
            break;
        DisassembledInstruction *instr = &(this->code[offset]);
        if (instr == nullptr || instr->len == 0)
            break;
        instrs->push_back(instr);
        offset += instr->len;
    }
}

/*
 * Not used by disassemble_page() or get_disassembly(). Simply used as a single
 * instruction disassembler for other parts of the code.
 */
DisassembledInstruction Disassembler::disassemble_instruction(u16 addr)
{
    DisassembledInstruction ret;
    ret.addr = addr;
    u8 *it = this->mem.data + addr;
    u8 op_code = *it++;
    InstructionInfo info = instruction_info_table[op_code];

    // at end of memory
    if (addr > Layout::FREE_ROM_HIGH) {
        ret.assembly_str = "end of memory lol TODO"; // TODO
        ret.len = 0;
        return ret;
    }

    ret.assembly_str = info.mnemonic + " " + info.addr_parser(&it);
    ret.len = it - (this->mem.data + addr);
    return ret;
}

/*
 * Disassembles one (1) page of memory starting at the instruction at
 * `first_instr_addr`. Returns the offset of the "next instruction" from
 * the page address of the next page. I.e. if the function returns 2, then
 * the third byte of the next page is an opcode (probably). If the function
 * returns something < 0, then the code terminates early in the page and 
 * the "next instruction" isn't a valid instruction.
 */
int Disassembler::disassemble_page(Page *page, u16 first_instr_addr)
{
    // TODO compute checksum to see if we need to update the disassembled text.

    const u8 *page_start = this->mem.data + page->page_addr;
    const u8 *page_end   = this->mem.data + page->page_addr + Layout::PAGE_SIZE;
    page->first_instr_offset = first_instr_addr % Layout::PAGE_SIZE;
    u8 *it = this->mem.data + first_instr_addr;

    while (it < page_end) {
        u8 *instr_start = it;
        u16 offset      = (u16) (it - page_start); // fel. -1
        u8 op_code      = *it++;
        InstructionInfo info = instruction_info_table[op_code];

        // TODO check for jmps and add labels...

        // reached an invalid op code
        if (info.mnemonic == "-") {
            return it - page_end;
        }

        // if page ends on a brk, stop disassembling (for now)
        if (info.mnemonic == "brk" && offset == 255) {
            page->code.push_back({false, info.mnemonic, "00 ", (u16)(page->page_addr + offset), 1}); 
            return -1;
        }

        // reached end of memory
        if (page->page_addr + offset > Layout::FREE_ROM_HIGH) {
            return it - page_end;
        }

        page->code.resize((size_t) offset + 1);
        std::string disassembly = info.mnemonic + " " + info.addr_parser(&it);
        page->code[offset].addr         = page->page_addr + offset;
        page->code[offset].len          = (it - page_start) - offset;
        page->code[offset].assembly_str = disassembly; 
        page->code[offset].bytes_str    = get_bytes_str(instr_start, page->code[offset].len); 
    }
    
    return it - page_end;
}

/*
 * Returns the disassembled code at and surrounding `addr`.
 */
std::vector<DisassembledInstruction *> Disassembler::get_disassembly(u16 start_addr, u16 stop_addr)
{
    std::vector<DisassembledInstruction *> instrs;

    // figure out where to start disassembling/printing from.
    u16 page_nr   = start_addr / Layout::PAGE_SIZE;
    int instr_off = start_addr % Layout::PAGE_SIZE;
    //if (this->page_table[page_nr].first_instr_offset != -1) {
    //    
    //    // If this page has been disassembled. Find the first disassembled
    //    // page contigous with this one.
    //    while (page_nr > 0 && this->page_table[page_nr - 1].first_instr_offset != -1) {
    //        page_nr--;
    //    }

    //    // overwrite `instr_off`
    //    instr_off = this->page_table[page_nr].first_instr_offset;
    //}
    const u16 first_page_nr = page_nr;

    // disassemble pages
    Page *page = &(this->page_table[page_nr]);
    while (instr_off >= 0) {
        // get the address of the first instruction on the page
        u16 first_instr_addr = page_nr * Layout::PAGE_SIZE + instr_off;

        // TODO checksum

        // disassemble page starting at `first_instr_addr`.
        instr_off = this->disassemble_page(page, first_instr_addr);
        
        // get the next page
        page = &(this->page_table[++page_nr]);

        // manually stop once we reach a page we don't
        // care about
        if (page_nr * 0x100 > stop_addr)
            break;
        
        // Idk if this is handled elsewhere, but let's still make sure
        // we don't run past the end of memory
        if (page_nr >= Layout::N_PAGES)
            break;
    }

    // print disassembly
    for (page_nr = first_page_nr; this->page_table[page_nr].first_instr_offset != -1; page_nr++) {
        if (page_nr * 0x100 > stop_addr)
            break;
        this->page_table[page_nr].get_disassembly(&instrs);
    }
   
    return instrs;
}

/*
 * The addr_* functions correspond to the different addressing modes of the 
 * m6502. Each implements a small parser for the arguments belonging to an 
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
    return std::to_string((s8)bb);
}

std::string addr_zpg(u8 **bytes)
{
    u8 ll = *(*bytes)++;
    return "$" + byte_to_hex(ll);
}

std::string addr_zpg_X(u8 **bytes)
{
    u8 ll = *(*bytes)++;
    return "$" + byte_to_hex(ll), ", X";
}

std::string addr_zpg_Y(u8 **bytes)
{
    u8 ll = *(*bytes)++;
    return "$" + byte_to_hex(ll), ", Y";
}

/*
 * Sets up the page table.
 */
void Disassembler::init_page_table()
{
    for (int i = 0; i < Layout::N_PAGES; i++)
        page_table[i].page_addr = i * Layout::PAGE_SIZE;
}

/*
 * Populates the instruction_info_table.
 */
void Disassembler::populate_instruction_info_table()
{

    // "zero" fill
    for (int i = 0; i < 256; i++)
        instruction_info_table[i] = {addr_impl, "-"};

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
    this->instruction_info_table[0x59] = {addr_abs_Y, "eor"};
    this->instruction_info_table[0x5D] = {addr_abs_X, "eor"};
    this->instruction_info_table[0x5E] = {addr_abs_X, "lsr"};
                                                                   
    // row 6                                                       
    this->instruction_info_table[0x60] = {addr_impl,  "rts"};
    this->instruction_info_table[0x61] = {addr_ind_X, "adc"};
    this->instruction_info_table[0x65] = {addr_zpg,   "adc"};
    this->instruction_info_table[0x66] = {addr_zpg,   "ror"};
    this->instruction_info_table[0x68] = {addr_impl,  "pla"};
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
