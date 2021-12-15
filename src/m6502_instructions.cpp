
#include "m6502.h"

/*
 * Temporary. TODO remove.
 */
void not_implemented() 
{
    printf("Instruction not yet implemented. Exiting.");
    exit(0);
}

/*
 * Few helper functions
 */
u8 get_status_bit(Cpu *cpu, u8 bit_pos) 
{
    return (cpu->SR & (1 << bit_pos)) >> bit_pos;
}

void set_status_bit(Cpu *cpu, u8 bit_pos, bool value) 
{
    u8 mask = (1 << bit_pos);
    if (value == HIGH)
        cpu->SR |= mask;
    else
        cpu->SR &= ~mask; 
}

/******************************************************************************
 *
 * Arithmetic & logic
 *
 ******************************************************************************/

// Add with carry
u8 op_adc(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    auto fetched = addr_mode(cpu);
    u16 acc      = (u16) cpu->ACC; 
    u16 oper     = (u16) fetched.data; 
    u16 carry    = (u16) get_status_bit(cpu, BIT_C);

    // perform operation in 16-bit space to capture potential carry bit
    u16 res = acc + oper + carry;

    set_status_bit(cpu, BIT_C, (bool) (res >> 8));
    set_status_bit(cpu, BIT_N, (bool) ((res && 0xFF) >> 7));
    set_status_bit(cpu, BIT_Z, (bool) ((res && 0xFF) == 0));
    // TODO overflow flag

    cpu->ACC = (u8) (res & 0xFF);
    return 1 + fetched.additional_cycles;
}

// Subtract with carry
u8 op_sbc(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Arithmetic shift left 
u8 op_asl(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// logical shift right 
u8 op_lsr(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Rotate left
u8 op_rol(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Rotate right
u8 op_ror(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Compare with Cpu->X
u8 op_cpx(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Compare with Cpu->Y
u8 op_cpy(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Compare with Cpu->AXX
u8 op_cmp(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Increment 
u8 op_inc(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Decrement 
u8 op_dec(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// And  
u8 op_and(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Or with Cpu->ACC 
u8 op_ora(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Exclusive or
u8 op_eor(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Increment Cpu->X
u8 op_inx(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Increment Cpu->Y
u8 op_iny(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Decrement Cpu->X
u8 op_dex(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Decrement Cpu->Y
u8 op_dey(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

/******************************************************************************
 *
 * Memory 
 *
 ******************************************************************************/

// Push Cpu->SR onto stack 
u8 op_php(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Pull Cpu->SR from stack
u8 op_plp(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Push Cpu->ACC onto stack 
u8 op_pha(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Pull Cpu->ACC from stack
u8 op_pla(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Store Cpu->ACC
u8 op_sta(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Store Cpu->X
u8 op_stx(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Store Cpu->Y
u8 op_sty(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Load Cpu->ACC
u8 op_lda(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    auto res = addr_mode(cpu);
    cpu->ACC = res.data;
    return 2 + res.additional_cycles; // TODO check ??
}

// Load Cpu->X
u8 op_ldx(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Load Cpu->Y
u8 op_ldy(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Transfer Cpu->X --> Cpu->ACC
u8 op_txa(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Transfer Cpu->Y --> Cpu->ACC
u8 op_tya(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Transfer Cpu->ACC --> Cpu->X
u8 op_tax(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Transfer Cpu->ACC --> Cpu->Y
u8 op_tay(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Transfer Cpu->X --> $(Cpu->SP)
u8 op_txs(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Transfer $(Cpu->SP) --> Cpu->X
u8 op_tsx(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

/******************************************************************************
 *
 * Branching, Jumps, etc.
 *
 ******************************************************************************/

// Jump 
u8 op_jmp(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    auto res = addr_mode(cpu);
    cpu->PC = res.address;
    return 3 + res.additional_cycles; // TODO check ??
}

// Jump subroutine
u8 op_jsr(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Branch on equal
u8 op_beq(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Branch on not equal
u8 op_bne(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Branch on overflow set
u8 op_bvs(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Branch on overflow clear
u8 op_bvc(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Branch on plus 
u8 op_bpl(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Branch on minus 
u8 op_bmi(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Branch on carry set
u8 op_bcs(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Branch on carry clear
u8 op_bcc(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Return from interrupt
u8 op_rti(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

// Return from subroutine
u8 op_rts(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;
}

/******************************************************************************
 *
 * Flags & status. 
 *
 ******************************************************************************/

// Set carry 
u8 op_sec(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    set_status_bit(cpu, BIT_C, HIGH);
    return 2;
} 

// Set decimal
u8 op_sed(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    set_status_bit(cpu, BIT_D, HIGH);
    return 2;
}

// set interrupt disable
u8 op_sei(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    set_status_bit(cpu, BIT_I, HIGH);
    return 2;
}

// Clear carry
u8 op_clc(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    set_status_bit(cpu, BIT_C, LOW);
    return 2;
}

// Clear decimal
u8 op_cld(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    set_status_bit(cpu, BIT_D, LOW);
    return 2;
}

// Clear overflow
u8 op_clv(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    set_status_bit(cpu, BIT_V, LOW);
    return 2;
}

// Clear interrupt disable
u8 op_cli(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    set_status_bit(cpu, BIT_I, LOW);
    return 2;
}

/******************************************************************************
 *
 * Misc.
 *
 ******************************************************************************/

// Break
u8 op_brk(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    printf("Hit BRK instruction. Exit for now.\n");
    exit(0); // TODO implement correctly
    return 0;
} 

// Bit test
u8 op_bit(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    not_implemented();
    return 1;    
}

// No operation
u8 op_nop(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    return 1;
} 

/*
 * Special instruction for invalid op codes. Exits the program with an error 
 * message.
 */
u8 OP_INVALID(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    printf("Error: Hit unkown opcode 0x%02X . Exiting.\n", cpu->mem[cpu->PC - 1]);
    exit(1);
    return 1; // unreachable
}


/******************************************************************************
 *
 * Misc.
 *
 ******************************************************************************/

Instruction instruction_table[256];

/*
 * Populates the op table according to 
 * https://www.masswerk.at/6502/6502_instruction_set.html
 */
void populate_instruction_table() 
{
    // "zero" fill
    for (int i = 0; i < 256; i++)
        instruction_table[i] = {OP_INVALID, nullptr};

    // populate
    // row 1
    instruction_table[0x00] = {op_brk, addr_impl,  "BRK"};
    instruction_table[0x01] = {op_ora, addr_X_ind, "ORA X,ind"};
    instruction_table[0x05] = {op_ora, addr_zpg,   "ORA zpg"};
    instruction_table[0x06] = {op_asl, addr_zpg,   "ASL zpg"};
    instruction_table[0x08] = {op_php, addr_impl,  "PHP"};
    instruction_table[0x09] = {op_ora, addr_imm,   "ORA #"};
    instruction_table[0x0A] = {op_asl, addr_acc,   "ASL A"};
    instruction_table[0x0D] = {op_ora, addr_abs,   "ORA abs"};
    instruction_table[0x0E] = {op_asl, addr_abs,   "ASL abs"};
    
    // row 2
    instruction_table[0x18] = {op_clc, addr_impl,   "CLC"};

    instruction_table[0x69] = {op_adc, addr_imm,   "ADC #"};
    instruction_table[0x4C] = {op_jmp, addr_abs,   "JMP abs"};
    instruction_table[0xA9] = {op_lda, addr_imm,   "LDA #"};
    instruction_table[0xEA] = {op_nop, addr_impl,  "NOP"};
}

