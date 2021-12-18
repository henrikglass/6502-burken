#include "m6502.h"

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
AddrModeRet addr_acc(Cpu *cpu)
{
    return {1, &(cpu->ACC), 0xFFFF}; 
}

/*
 * abs --- absolute --- OPC $LLHH 
 *
 * Operand is address $HHLL.
 */
AddrModeRet addr_abs(Cpu *cpu)
{
    u8 ll = cpu->mem[cpu->PC++];
    u8 hh = cpu->mem[cpu->PC++];
    u16 addr = (hh << 8) + ll;
    u8 *data_ptr = &(cpu->mem[addr]); // TODO 
    return {3, data_ptr, addr}; // TODO # cycles
}

/*
 * abs,X --- absolute, X-indexed --- OPC $LLHH,X 
 *
 * Operand is address; effective address is address incremented by X with carry.
 */
AddrModeRet addr_abs_X(Cpu *cpu)
{
    u8 ll = cpu->mem[cpu->PC++];
    u8 hh = cpu->mem[cpu->PC++];
    u16 addr = (hh << 8) + ll + cpu->X;
    u8 *data_ptr = &(cpu->mem[addr]); // TODO 
    return {3, data_ptr, addr}; // TODO # cycles
}

/*
 * abs,Y --- absolute, Y-indexed --- OPC $LLHH,Y
 *
 * Operand is address; effective address is address incremented by Y with carry.
 */
AddrModeRet addr_abs_Y(Cpu *cpu)
{
    u8 ll = cpu->mem[cpu->PC++];
    u8 hh = cpu->mem[cpu->PC++];
    u16 addr = (hh << 8) + ll + cpu->Y;
    u8 *data_ptr = &(cpu->mem[addr]); // TODO 
    return {3, data_ptr, addr}; // TODO # cycles
}

/*
 * # --- immediate --- OPC #$BB 
 *
 * Operand is byte BB.
 */
AddrModeRet addr_imm(Cpu *cpu)
{
    u16 addr = cpu->PC++;
    u8 *data_ptr = &(cpu->mem[addr]); // don't write to this... 
    return {1, data_ptr, addr}; // TODO # cycles
}

/*
 * ind --- indirect --- OPC ($LLHH)
 *
 * Operand is address; effective address is contents of word at address: 
 * C.w($HHLL).
 */
AddrModeRet addr_ind(Cpu *cpu)
{
    u8 ll = cpu->mem[cpu->PC++];
    u8 hh = cpu->mem[cpu->PC++];
    u16 addr = (hh << 8) + ll;
    ll = cpu->mem[addr];
    hh = cpu->mem[addr + 1];
    addr = (hh << 8) + ll + cpu->Y;
    u8 *data_ptr = &(cpu->mem[addr]); // TODO 
    return {0, data_ptr, addr}; // TODO # cycles
}

/*
 * X,ind --- X-indexed, indirect --- OPC ($LL,X) 
 *
 * Operand is zeropage address; effective address is word in 
 * (LL + X, LL + X + 1), inc. without carry: C.w($00LL + X).
 */
AddrModeRet addr_X_ind(Cpu *cpu)
{
    u8 ll = cpu->mem[cpu->PC++];
    u8 hh = 0x00;                         // zero page
    ll = (ll + cpu->X);                  // No carry!
    u16 zp_addr = ll;
    ll = cpu->mem[zp_addr];
    hh = cpu->mem[zp_addr + 1];
    u16 addr = (hh << 8) + ll;
    u8 *data_ptr = &(cpu->mem[addr]); // TODO 
    return {0, data_ptr, addr}; // TODO # cycles
}

/*
 * ind,Y --- indirect, Y-indexed --- OPC ($LL),Y 
 *
 * Operand is zeropage address; effective address is word in (LL, LL + 1) 
 * incremented by Y with carry: C.w($00LL) + Y.
 */
AddrModeRet addr_ind_Y(Cpu *cpu)
{
    u8 ll = cpu->mem[cpu->PC++];
    u16 hh = 0x0000;                        // zero page
    u16 zp_addr = ll;
    ll = cpu->mem[zp_addr];
    hh = cpu->mem[zp_addr + 1];
    u16 addr = hh + ll + cpu->Y; // with carry
    u8 *data_ptr = &(cpu->mem[addr]); // TODO 
    return {0, data_ptr, addr}; // TODO # cycles
}

/*
 * rel --- relative --- OPC $BB 
 *
 * Branch target is PC + signed offset BB ***.
 */
AddrModeRet addr_rel(Cpu *cpu)
{
    u8 offset = cpu->mem[cpu->PC++];
    u16 addr = cpu->PC + offset;        // last PC or cpu? 
    u8 *data_ptr = &(cpu->mem[addr]); // TODO 
    return {0, data_ptr, addr}; // TODO # cycles
}

/*
 * zpg --- zeropage --- OPC $LL 
 *
 * Operand is zeropage address (hi-byte is zero, address = $00LL).
 */
AddrModeRet addr_zpg(Cpu *cpu)
{
    u8 ll = cpu->mem[cpu->PC++];
    u16 addr = ll;
    u8 *data_ptr = &(cpu->mem[addr]); // TODO 
    return {0, data_ptr, addr}; // TODO # cycles
}

/*
 * zpg,X --- zeropage, X-indexed --- OPC $LL,X 
 *
 * Operand is zeropage address; effective address is address incremented by X 
 * without carry **.
 */
AddrModeRet addr_zpg_X(Cpu *cpu)
{
    u8 ll = cpu->mem[cpu->PC++];
    u16 addr = ll + cpu->X; // TODO check add done in 8 bit space?
    u8 *data_ptr = &(cpu->mem[addr]); // TODO 
    return {0, data_ptr, addr}; // TODO # cycles
}

/*
 * zpg,Y --- zeropage, Y-indexed --- OPC $LL,Y
 *
 * Operand is zeropage address; effective address is address incremented by Y
 * without carry **.
 */
AddrModeRet addr_zpg_Y(Cpu *cpu)
{
    u8 ll = cpu->mem[cpu->PC++];
    u16 addr = ll + cpu->Y;
    u8 *data_ptr = &(cpu->mem[addr]); // TODO 
    return {0, data_ptr, addr}; // TODO # cycles
}

