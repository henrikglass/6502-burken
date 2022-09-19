
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

#ifndef M6502_H
#define M6502_H

#include <string>

#include "typedefs.h"
#include "memory.h"

namespace M6502Constants 
{
    const float CLOCK_SPEED_MAX = 3000000.0f; // 3 MHz
};

/*****************************************************************************
 *
 * Cpu structure and registers.
 *
 *****************************************************************************/

/*
 * Status register layout
 */
const u8 BIT_C      = 0; // Carry
const u8 BIT_Z      = 1; // Zero
const u8 BIT_I      = 2; // block IRQ interrupts
const u8 BIT_D      = 3; // Decimal mode
const u8 BIT_B      = 4; // BRK was executed
const u8 BIT_UNUSED = 5; //
const u8 BIT_V      = 6; // oVerflow
const u8 BIT_N      = 7; // Negative


/*
 * A 6502 cpu.
 *
 */
struct Cpu 
{
    Cpu(Memory &mem);

    // Program counter
    u16 PC = 0x00;

    // Accumulator and index registers 
    u8 ACC = 0x00, X = 0x00, Y = 0x00;
    
    // Process status register
    u8 SR = 0x00;

    // Stack pointer
    u16 SP = 0x0000;

    // Memory
    Memory &mem;

    // Execute a single instruction, returning the # of cycles taken
    u8 fetch_execute_next();

    // interrupts
    void reset();
    void irq();
    void nmi();
};

/*****************************************************************************
 *
 * Memory addressing modes.
 *
 *****************************************************************************/

/*
 * Simple tuple for returning data from addressing mode functions.
 */
struct AddrModeRet
{
    u8  additional_cycles;
    u8 *data_ptr;
    u16 address;
};

/*
 * Addressing mode functions
 *
 * TODO these might not need to be here?
 */
AddrModeRet addr_acc(Cpu *cpu);
AddrModeRet addr_abs(Cpu *cpu);
AddrModeRet addr_abs_X(Cpu *cpu);
AddrModeRet addr_abs_Y(Cpu *cpu);
AddrModeRet addr_imm(Cpu *cpu);
AddrModeRet addr_impl(Cpu *cpu);
AddrModeRet addr_ind(Cpu *cpu);
AddrModeRet addr_ind_X(Cpu *cpu);
AddrModeRet addr_ind_Y(Cpu *cpu);
AddrModeRet addr_rel(Cpu *cpu);
AddrModeRet addr_zpg(Cpu *cpu);
AddrModeRet addr_zpg_X(Cpu *cpu);
AddrModeRet addr_zpg_Y(Cpu *cpu);

/*****************************************************************************
 *
 * Instructions. 
 *
 *****************************************************************************/

/*
 * Represents a table entry in the Instruction table. The first element is a 
 * function pointer to an operation function. The second element is a 
 * function pointer to an addressing mode function. The third element is
 * a mnemonic. The fourth element specifies the min # of cycles for the 
 * instruction (additional cycles may result from the addressign modes,
 * these are returned from the addressing mode function and subsequently
 * from the operation function).
 */
struct Instruction
{
    u8 (*op)(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu * cpu)); // instr.
    AddrModeRet (*addr_mode)(Cpu *cpu); // addressing mode.
    u8 n_cycles;
    u8 execute(Cpu *cpu) 
    {
        return n_cycles + op(cpu, addr_mode); // Apply operation with chosen addressing mode
    }
};

/*
 * Instruction table.
 */
extern Instruction instruction_table[256];

/*
 * Populates the op table according to 
 * https://www.masswerk.at/6502/6502_instruction_set.html
 */
void populate_instruction_table(); 

#endif
