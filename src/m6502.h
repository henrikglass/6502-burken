#ifndef M6502_H
#define M6502_H

#include <string.h>

#include "typedefs.h"

/*****************************************************************************
 *
 * Emulates the functionality of a MOS technology 6502 processor. The 
 * reference documentation for the implementation is from 
 * https://eater.net/datasheets/w65c02s.pdf (Accessed 2021-12-10). 
 *
 * Author: Henrik A. Glass
 *
 *****************************************************************************/

/*
 * Layout proposed by Rockwell and implemented by most: 
 * <https://www.csh.rit.edu/~moffitt/docs/6502.html#MEM_MAP>
 */
const u16 ZERO_PAGE_LOW   = 0x0000;
const u16 ZERO_PAGE_HIGH  = 0x00FF;
const u16 STACK_PAGE_LOW  = 0x0100;
const u16 STACK_PAGE_HIGH = 0x01FF;
const u16 FREE_RAM_LOW    = 0x0200;
const u16 FREE_RAM_HIGH   = 0x3FFF;
const u16 IO_MEM_LOW      = 0x4000;
const u16 IO_MEM_HIGH     = 0x7FFF;
const u16 FREE_ROM_LOW    = 0x8000;
const u16 FREE_ROM_HIGH   = 0xFFF9;
const u16 NMI_VECTOR      = 0xFFFA; 
const u16 RESET_VECTOR    = 0xFFFC; // TODO implement at 0xFCE2;
const u16 IRQ_BRK_VECTOR  = 0xFFFE;

/*
 * A general purpose memory.
 */
struct Memory 
{
    
    Memory()
    {
        printf("MEM init.\n");
        data = new u8[0xFFFF + 1];
        memset(data, 0, (0xFFFF + 1) * sizeof(u8)); // Not really neccessary.
        data[RESET_VECTOR]     =  FREE_ROM_LOW & 0x00FF; // TODO temporary
        data[RESET_VECTOR + 1] = (FREE_ROM_LOW & 0xFF00) >> 8; 
    }

    ~Memory()
    {
        delete[] data;
    }

    // 8-bit data bus --> byte-aligned array.
    u8 *data;

    // read/write
    u8  operator[](u16 address) const 
    {
        return data[address];
    }
    u8 &operator[](u16 address) 
    {
        return data[address];
    }

    /*
     * Loads memory from file at specified offset.
     */
    void load_from_file(char *path, u16 offset);
};

/*
 * Status register layout
 */
const u8 STATUS_MASK_C      = 0b00000001; // Carry
const u8 STATUS_MASK_Z      = 0b00000010; // Zero
const u8 STATUS_MASK_I      = 0b00000100; // block IRQ interrupts
const u8 STATUS_MASK_D      = 0b00001000; // Decimal mode
const u8 STATUS_MASK_B      = 0b00010000; // BRK was executed
const u8 STATUS_MASK_UNUSED = 0b00100000; //
const u8 STATUS_MASK_V      = 0b01000000; // oVerflow
const u8 STATUS_MASK_N      = 0b10000000; // Negative

/*
 * Instruction set
 */
enum Instruction 
{
    BRK     = 0x00,
    NOP     = 0xEA,
    PHA     = 0x48,
    LDA_I   = 0xA9,
    LDX_I   = 0xA2,
    LDY_I   = 0xA0,
    JMP_ABS = 0x4c
};

/*
 * A 6502 cpu.
 */
struct Cpu 
{
    // Program counter
    u16 PC;

    // Accumulator and index registers 
    u8  ACC, X, Y;
    
    // Process status register
    u8  SR;

    // Stack pointer
    u16  SP;

    // Memory is included for simplicity.
    Memory mem;

    // reset
    void reset();

    // Execute a single instruction, returning the # of cycles taken
    u8 fetch_execute_next();

    // debug
    void print_status();
};


#endif
