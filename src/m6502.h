#ifndef M6502_H
#define M6502_H

#include <string.h>

#include "typedefs.h"
#include "memory.h"

/*****************************************************************************
 *
 * Emulates the functionality of a MOS technology 6502 processor. The 
 * reference documentation for the implementation is from 
 * https://www.masswerk.at/6502/6502_instruction_set.html 
 * (Accessed 2021-12-12).
 *
 * Author: Henrik A. Glass
 *
 *****************************************************************************/

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
enum TTInstruction 
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
    Cpu();

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
