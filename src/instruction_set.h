#ifndef INSTRUCTION_SET_H
#define INSTRUCTION_SET_H

#include "m6502.h"

/*****************************************************************************
 *
 * This header file includes the declarations of all addressing modes and 
 * instructions supported by the emulator. Implementations are found in the 
 * files: <addressing_modes.cpp> and <instructions.cpp>
 *
 * Author: Henrik A. Glass
 *
 *****************************************************************************/

/*
 * Simple tuple for returning data from addressing mode functions.
 */
struct AddrModeRet
{
    u8  additional_cycles;
    u8  data;
    u16 address;
};

/*
 * Represents a table entry in the Instruction table. The first element is a 
 * function pointer to an instruction function. The second element is a 
 * function pointer to an addressing mode function.
 */
struct Operation
{
    u8 (*instr)(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu * cpu)); // instr.
    AddrModeRet (*addr_mode)(Cpu *cpu); // addressing mode.
    u8 perform(Cpu *cpu) 
    {
        return instr(cpu, addr_mode);
    }
};

/*****************************************************************************
 *
 * Memory addressing modes.
 *
 * All addressing mode functions expect a pointer to a Cpu which may be 
 * mutated.
 *
 *****************************************************************************/

/*
 * A --- Accumulator --- OPC A 
 *
 * Operand is AC (implied single byte instruction).
 */
AddrModeRet acc(Cpu *cpu)
{
    printf("ACC\n");
    return {0, cpu->ACC, 0xFFFF}; 
}

/*
 * abs --- absolute --- OPC $LLHH 
 *
 * Operand is address $HHLL.
 */
AddrModeRet abs(Cpu *cpu)
{
    printf("ABS\n");
    u8 ll = cpu->mem[cpu->PC++];
    u8 hh = cpu->mem[cpu->PC++];
    u16 addr = (hh << 8) + ll;
    u8 data  = cpu->mem[addr];
    return {0, data, addr}; // TODO # cycles
}

/*
 * abs,X --- absolute, X-indexed --- OPC $LLHH,X 
 *
 * Operand is address; effective address is address incremented by X with carry.
 */
AddrModeRet abs_X(Cpu *cpu)
{
    printf("ABS_X\n");
    u8 ll = cpu->mem[cpu->PC++];
    u8 hh = cpu->mem[cpu->PC++];
    u16 addr = (hh << 8) + ll + cpu->X;
    u8 data = cpu->mem[addr];
    return {0, data, addr}; // TODO # cycles
}

/*
 * abs,Y --- absolute, Y-indexed --- OPC $LLHH,Y
 *
 * Operand is address; effective address is address incremented by Y with carry.
 */
AddrModeRet abs_Y(Cpu *cpu)
{
    printf("ABS_Y\n");
    u8 ll = cpu->mem[cpu->PC++];
    u8 hh = cpu->mem[cpu->PC++];
    u16 addr = (hh << 8) + ll + cpu->Y;
    u8 data = cpu->mem[addr];
    return {0, data, addr}; // TODO # cycles
}

/*
 * # --- immediate --- OPC #$BB 
 *
 * Operand is byte BB.
 */
AddrModeRet imm(Cpu *cpu)
{
    printf("IMM\n");
    u16 addr = cpu->PC++;
    u8 data = cpu->mem[addr];
    return {0, data, addr}; // TODO # cycles
}

/*
 * impl --- implied --- OPC
 *
 * Operand implied.
 *
 * @NotImplemented
 */
AddrModeRet impl(Cpu *cpu)
{
    printf("IMPL\n");
    return {0, 0xFF, 0xFFFF};
}

/*
 * ind --- indirect --- OPC ($LLHH)
 *
 * Operand is address; effective address is contents of word at address: 
 * C.w($HHLL).
 */
AddrModeRet ind(Cpu *cpu)
{
    printf("IND\n");
    u8 ll = cpu->mem[cpu->PC++];
    u8 hh = cpu->mem[cpu->PC++];
    u16 addr = (hh << 8) + ll;
    ll = cpu->mem[addr];
    hh = cpu->mem[addr + 1];
    addr = (hh << 8) + ll + cpu->Y;
    u8 data = cpu->mem[addr];
    return {0, data, addr}; // TODO # cycles
}

/*
 * X,ind --- X-indexed, indirect --- OPC ($LL,X) 
 *
 * Operand is zeropage address; effective address is word in 
 * (LL + X, LL + X + 1), inc. without carry: C.w($00LL + X).
 */
AddrModeRet X_ind(Cpu *cpu)
{
    printf("X_IND\n");
    u8 ll = cpu->mem[cpu->PC++];
    u8 hh = 0x00;                         // zero page
    ll = (ll + cpu->X);                  // No carry!
    u16 zp_addr = ll;
    ll = cpu->mem[zp_addr];
    hh = cpu->mem[zp_addr + 1];
    u16 addr = (hh << 8) + ll;
    u8  data = cpu->mem[addr];
    return {0, data, addr}; // TODO # cycles
}

/*
 * ind,Y --- indirect, Y-indexed --- OPC ($LL),Y 
 *
 * Operand is zeropage address; effective address is word in (LL, LL + 1) 
 * incremented by Y with carry: C.w($00LL) + Y.
 */
AddrModeRet ind_Y(Cpu *cpu)
{
    printf("IND_Y\n");
    u8 ll = cpu->mem[cpu->PC++];
    u8 hh = 0x00;                        // zero page
    u16 zp_addr = ll;
    ll = cpu->mem[zp_addr];
    hh = cpu->mem[zp_addr + 1];
    u16 addr = (hh << 8) + ll + cpu->Y; // with carry
    u8  data = cpu->mem[addr];
    return {0, data, addr}; // TODO # cycles
}

/*
 * rel --- relative --- OPC $BB 
 *
 * Branch target is PC + signed offset BB ***.
 */
AddrModeRet rel(Cpu *cpu)
{
    printf("REL\n");
    u8 offset = cpu->mem[cpu->PC++];
    u16 addr = cpu->PC + offset;        // last PC or cpu? 
    u8 data = cpu->mem[addr]; 
    return {0, data, addr}; // TODO # cycles
}

/*
 * zpg --- zeropage --- OPC $LL 
 *
 * Operand is zeropage address (hi-byte is zero, address = $00LL).
 */
AddrModeRet zpg(Cpu *cpu)
{
    printf("ZPG\n");
    u8 ll = cpu->mem[cpu->PC++];
    u16 addr = ll;
    u8 data = cpu->mem[addr];
    return {0, data, addr}; // TODO # cycles
}

/*
 * zpg,X --- zeropage, X-indexed --- OPC $LL,X 
 *
 * Operand is zeropage address; effective address is address incremented by X 
 * without carry **.
 */
AddrModeRet zpg_X(Cpu *cpu)
{
    printf("ZPG_X\n");
    u8 ll = cpu->mem[cpu->PC++];
    u16 addr = ll + cpu->X; // TODO check add done in 8 bit space?
    u8 data = cpu->mem[addr];
    return {0, data, addr}; // TODO # cycles
}

/*
 * zpg,Y --- zeropage, Y-indexed --- OPC $LL,Y
 *
 * Operand is zeropage address; effective address is address incremented by Y
 * without carry **.
 */
AddrModeRet zpg_Y(Cpu *cpu)
{
    printf("ZPG_Y\n");
    u8 ll = cpu->mem[cpu->PC++];
    u16 addr = ll + cpu->Y;
    u8 data = cpu->mem[addr];
    return {0, data, addr}; // TODO # cycles
}

/*****************************************************************************
 *
 * Instructions. 
 *
 * All instruction functions expect a pointer to a Cpu which may 
 * be mutated and a function pointer to an addressing mode function.
 *
 *****************************************************************************/

u8 brk(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    printf("Hit BRK instruction. Exit for now.\n");
    exit(0); // TODO implement correctly
    return 0;
}

u8 nop(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    printf("NOP \n");
    return 1;
}

u8 lda(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{

    printf("LDA ");
    auto res = addr_mode(cpu);
    cpu->ACC = res.data;
    return 2 + res.additional_cycles; // TODO check ??
}

u8 jmp(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
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
u8 INVALID(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    printf("Error: Hit unkown opcode 0x%02X . Exiting.\n", cpu->mem[cpu->PC - 1]);
    exit(1);
    return 1; // unreachable
}

/*****************************************************************************
 *
 * Op table.
 *
 * Mapping from opcodes to instructions + addressing modes. We call such a 
 * pair an "operation".
 *
 * Table containing all legal combinations of instructions and addressing modes.
 * The table fits all possible (incl. illegal) instructions, but only the legal 
 * ones are implemented. The rest map to the "INVALID" instruction
 *
 *****************************************************************************/

Operation op_table[256];

void populate_op_table() 
{
    // "zero" fill
    for (int i = 0; i < 256; i++)
        op_table[i] = {INVALID, nullptr};

    // populate
    op_table[0x00] = {brk, impl};
    op_table[0x4C] = {jmp, abs};
    op_table[0xA9] = {lda, imm};
    op_table[0xEA] = {nop, impl};
}

#endif
