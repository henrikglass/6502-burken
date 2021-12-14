
#include "m6502.h"

u8 op_brk(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    printf("Hit BRK instruction. Exit for now.\n");
    exit(0); // TODO implement correctly
    return 0;
}

u8 op_ora(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{

    printf("Hit BRK instruction. Exit for now.\n");
    exit(0); // TODO implement correctly
    return 0;
} 

u8 op_nop(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    printf("NOP \n");
    return 1;
}

u8 op_lda(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{

    printf("LDA ");
    auto res = addr_mode(cpu);
    cpu->ACC = res.data;
    return 2 + res.additional_cycles; // TODO check ??
}

u8 op_jmp(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    printf("JMP ");
    auto res = addr_mode(cpu);
    cpu->PC = res.address;
    return 3 + res.additional_cycles; // TODO check ??
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
    instruction_table[0x00] = {op_brk, addr_impl};
    instruction_table[0x01] = {op_ora, addr_X_ind};
    instruction_table[0x05] = {op_ora, addr_zpg};
    instruction_table[0x06] = {op_asl, addr_zpg};
    instruction_table[0x08] = {op_php, addr_impl};
    instruction_table[0x09] = {op_ora, addr_imm};
    instruction_table[0x0A] = {op_asl, addr_acc};
    instruction_table[0x0D] = {op_ora, addr_abs};
    instruction_table[0x0E] = {op_asl, addr_abs};

    instruction_table[0x4C] = {op_jmp, addr_abs};
    instruction_table[0xA9] = {op_lda, addr_imm};
    instruction_table[0xEA] = {op_nop, addr_impl};
}

