#ifndef M6502_DISASSMEBLER_H
#define M6502_DISASSMEBLER_H

#include "typedefs.h"
#include "memory.h"

#include <vector>
#include <map>

/*
 * A `DisassembledInstruction` bundles a string representation of a
 * a disassembled instruction with it's size in bytes;
 */
struct DisassembledInstruction
{
    std::string str;
    u8 len;
};

/*
 * A `Block` represents a series of consecutive instructions
 * in memory.
 */
struct Block
{
    Block(u16 start) : start(start) {};

    // The start address of the block
    u16 start;

    // A sparse array that maps address offsets to disassembled instructions 
    // . I.e. mem[`start` + <offset>] = the op-code of the disassembled 
    // instruction at code[<offset>].
    std::vector<DisassembledInstruction> code; 

    // The size of the block
    u16 size() 
    {
        return code.size() - 1 + code[code.size() - 1].len;
    };
};

Block merge(const Block &lower, const Block &upper);

class Disassembler
{
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
        std::string (*addr_parser)(u8 **bytes);
        
        // A mnemonic
        std::string mnemonic;
    };
    InstructionInfo instruction_info_table[256];
    void populate_instruction_info_table();

    /*
     * `code_blocks` represents an entire disassembled program.
     */
    std::vector<Block> code_blocks;
public:
    Disassembler(const Memory &mem) : mem(mem) 
    {
        this->populate_instruction_info_table();    
    };
    void disassemble();
    std::vector<u16> disassemble_block(Block *block);
    void print(); // debug
};

#endif
