#ifndef M6502_DISASSMEBLER_H
#define M6502_DISASSMEBLER_H

#include "typedefs.h"
#include "memory.h"

#include <vector>
#include <map>
#include <sstream>

/*
 * A `DisassembledInstruction` bundles a string representation of a
 * a disassembled instruction with it's size in bytes;
 */
struct DisassembledInstruction
{
    bool label = false; // TODO implement "best guess" labels
    std::string assembly_str;
    std::string bytes_str;
    u16 addr;
    u8 len;
};

class Disassembler
{
private:
    const Memory &mem;

    
    struct Page
    {
        // The address of the page (e.g 0x8000, 0x8100, etc.)
        u16 page_addr;

        // offset to the first instruction beloning to this page. See below
        // for definition of "belonging". -1 indicates this page hasn't yet 
        // been disassembled
        int first_instr_offset = -1;

        // `code_offsets` holds the offsets where code resides in the page.
        //std::vector<int> code_offsets;

        // We compute a checksum over all bytes belonging to instructions in
        // the page. We count instructions whose 1-byte opcode resides in a page
        // as "belonging" to the page. Note: this might include bytes from the 
        // next page (`page_addr` + 0x100) and exclude bytes from the start of 
        // the current page (I.e. those at offsets < `first_instr_offset`).
        u32 checksum = 0;

        // A sparse array that maps address offsets to disassembled instructions 
        // . I.e. mem[`start` + <offset>] = the op-code of the disassembled 
        // instruction at code[<offset>].
        std::vector<DisassembledInstruction> code; 

        // Returns the disassembled text as a stringstream.
        void get_disassembly(std::vector<DisassembledInstruction *> *instrs);
    };

    /*
     * We keep a table of all pages in memory, indexed by the page number.
     */
    Page page_table[Layout::N_PAGES];
    void init_page_table();
    int disassemble_page(Page *page, u16 first_instr_addr);

    /*
     * The `InstructionInfo` struct and `instruction_info_table` contains all
     * the information needed to interpret a series of consecutive bytes as
     * plaintext m6502 assembly.
     */
    struct InstructionInfo 
    {
        // An addressing mode parser function 
        std::string (*addr_parser)(u8 **bytes);
        
        // A mnemonic
        std::string mnemonic;
    };
    InstructionInfo instruction_info_table[256];
    void populate_instruction_info_table();

public:
    Disassembler(const Memory &mem) : mem(mem) 
    {
        this->init_page_table();
        this->populate_instruction_info_table();    
    };
    DisassembledInstruction disassemble_instruction(u16 addr);
    std::vector<DisassembledInstruction *> get_disassembly(u16 start_addr, u16 stop_addr);
};

#endif
