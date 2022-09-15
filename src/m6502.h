
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

    // debug
    void print_status();
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
    std::string mnemonic;
    u8 n_cycles;
    u8 execute(Cpu *cpu) 
    {
        return n_cycles + op(cpu, addr_mode); // Apply operation with chosen addressing mode
    }
};

/*
 * Arithmetic & logic
 */
u8 op_adc(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Add with carry
u8 op_sbc(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Subtract with carry
u8 op_asl(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Arithmetic shift left 
u8 op_lsr(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // logical shift right 
u8 op_rol(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Rotate left
u8 op_ror(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Rotate right
u8 op_cpx(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Compare with Cpu->X
u8 op_cpy(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Compare with Cpu->Y
u8 op_cmp(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Compare with Cpu->AXX
u8 op_inc(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Increment 
u8 op_dec(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Decrement 
u8 op_and(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // And  
u8 op_ora(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Or with Cpu->ACC 
u8 op_eor(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Exclusive or
u8 op_inx(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Increment Cpu->X
u8 op_iny(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Increment Cpu->Y
u8 op_dex(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Decrement Cpu->X
u8 op_bit(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Bit test8 op_dey(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Decrement Cpu->Y

/*
 * Memory
 */
u8 op_php(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Push Cpu->SR onto stack 
u8 op_plp(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Pull Cpu->SR from stack
u8 op_pha(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Push Cpu->ACC onto stack 
u8 op_pla(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Pull Cpu->ACC from stack
u8 op_sta(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Store Cpu->ACC
u8 op_stx(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Store Cpu->X
u8 op_sty(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Store Cpu->Y
u8 op_lda(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Load Cpu->ACC
u8 op_ldx(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Load Cpu->X
u8 op_ldy(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Load Cpu->Y
u8 op_txa(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Transfer Cpu->X     --> Cpu->ACC
u8 op_tya(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Transfer Cpu->Y     --> Cpu->ACC
u8 op_tax(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Transfer Cpu->ACC   --> Cpu->X
u8 op_tay(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Transfer Cpu->ACC   --> Cpu->Y
u8 op_txs(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Transfer Cpu->X     --> Cpu->SP
u8 op_tsx(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Transfer Cpu->SP    --> Cpu->X

/*
 * Branching, jumps...
 */
u8 op_jmp(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Jump 
u8 op_jsr(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Jump subroutine
u8 op_beq(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Branch on equal
u8 op_bne(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Branch on not equal
u8 op_bvs(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Branch on overflow set
u8 op_bvc(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Branch on overflow clear
u8 op_bpl(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Branch on plus 
u8 op_bmi(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Branch on minus 
u8 op_bcs(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Branch on carry set
u8 op_bcc(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Branch on carry clear
u8 op_rti(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Return from interrupt
u8 op_rts(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Return from subroutine

/*
 * Flags 
 */
u8 op_sec(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Set carry 
u8 op_sed(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Set decimal
u8 op_sei(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // set interrupt disable
u8 op_clc(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Clear carry
u8 op_cld(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Clear decimal
u8 op_clv(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Clear overflow
u8 op_cli(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Clear interrupt disable

/*
 * Misc.
 */
u8 op_brk(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Break 
u8 op_nop(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // No operation 
u8 OP_INVALID(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)); // Special naughty instruction


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
