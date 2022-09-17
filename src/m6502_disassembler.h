#ifndef M6502_DISASSMEBLER_H
#define M6502_DISASSMEBLER_H

#include "typedefs.h"
#include "memory.h"

#include <vector>
#include <map>

class Disassembler
{
public:
    Disassembler(const Memory &mem) : mem(mem) {};
    void disassemble();
    void print(); // debug
private:
    const Memory &mem;

    enum AddrModeType 
    {
        IMPL,
        ACC,
        ABS,
        ABS_X,
        ABS_Y,
        IMM,
        IND,
        IND_X,
        IND_Y,
        REL,
        ZPG,
        ZPG_X,
        ZPG_Y
    };

    /*
     * The `InstructionInfo` struct and `instruction_info_table` contains all
     * the information needed to interpret a series of consecutive bytes as
     * plaintext m6502 assembly.
     */
    struct InstructionInfo 
    {
        // An addressing mode parser function 
        std::string (*addr_parser)(u8 *bytes);
        
        // A mnemonic
        std::string mnemonic;
    };
    InstructionInfo instruction_info_table[256];
    void populate_instruction_info_table();

    /*
     * A `Block` represents a series of consecutive instructions
     * in memory.
     */
    struct Block
    {
        Block(u16 start) : start(start) {};

        // The start address of the block
        u16 start;

        // The size of the block in bytes
        u16 size;

        // A map from address offsets to disassembled instructions (strings).
        // I.e. mem[`start` + <offset>] = the op-code of the disassembled 
        // instruction at code[<offset>].
        std::map<u16, std::string> code; 
    };

    Block merge(const Block &lower, const Block &upper);

    /*
     * `code_blocks` represents an entire disassembled program.
     */
    std::vector<Block> code_blocks;
};

#endif
