#include <iostream>

#include "m6502.h" 

/*****************************************************************************
 *
 * Memory addressing modes.
 *
 * TODO 1. manage stuff extra cycle with carry 
 *      2. double check correctness
 *      3. set correct bits
 *
 *****************************************************************************/

/*
 * A --- Accumulator --- OPC A 
 *
 * Operand is AC (implied single byte instruction).
 */
AddrDataPair Cpu::addr_acc()
{
    return {this->ACC, 0xFFFF}; 
}

/*
 * abs --- absolute --- OPC $LLHH 
 *
 * Operand is address $HHLL.
 */
AddrDataPair Cpu::addr_abs()
{
    u8 ll = this->mem[this->PC++];
    u8 hh = this->mem[this->PC++];
    u16 addr = (hh << 8) + ll;
    u8  data = this->mem[addr];
    return {data, addr};
}

/*
 * abs,X --- absolute, X-indexed --- OPC $LLHH,X 
 *
 * Operand is address; effective address is address incremented by X with carry.
 */
AddrDataPair Cpu::addr_abs_X()
{

    u8 ll = this->mem[this->PC++];
    u8 hh = this->mem[this->PC++];
    u16 addr = (hh << 8) + ll + this->X;
    u8  data = this->mem[addr];
    return {data, addr};
}

/*
 * abs,Y --- absolute, Y-indexed --- OPC $LLHH,Y
 *
 * Operand is address; effective address is address incremented by Y with carry.
 */
AddrDataPair Cpu::addr_abs_Y()
{

    u8 ll = this->mem[this->PC++];
    u8 hh = this->mem[this->PC++];
    u16 addr = (hh << 8) + ll + this->Y;
    u8  data = this->mem[addr];
    return {data, addr};
}

/*
 * # --- immediate --- OPC #$BB 
 *
 * Operand is byte BB.
 */
AddrDataPair Cpu::addr_imm()
{
    u16 addr = this->PC++;
    u8  data = this->mem[addr];
    return {data, addr}; 
}

/*
 * impl --- implied --- OPC
 *
 * Operand implied.
 *
 * @NotImplemented
 */
AddrDataPair Cpu::addr_impl(){
    return {0xFF, 0xFFFF};
}

/*
 * ind --- indirect --- OPC ($LLHH)
 *
 * Operand is address; effective address is contents of word at address: 
 * C.w($HHLL).
 */
AddrDataPair Cpu::addr_ind()
{
    u8 ll = this->mem[this->PC++];
    u8 hh = this->mem[this->PC++];
    u16 addr = (hh << 8) + ll;
    ll = this->mem[addr];
    hh = this->mem[addr + 1];
    addr = (hh << 8) + ll + this->Y;
    u8 data = this->mem[addr];
    return {data, addr};
}

/*
 * X,ind --- X-indexed, indirect --- OPC ($LL,X) 
 *
 * Operand is zeropage address; effective address is word in 
 * (LL + X, LL + X + 1), inc. without carry: C.w($00LL + X).
 */
AddrDataPair Cpu::addr_X_ind()
{
    u8 ll = this->mem[this->PC++];
    u8 hh = 0x00;                         // zero page
    ll = (ll + this->X);                  // No carry!
    u16 zp_addr = ll;
    ll = this->mem[zp_addr];
    hh = this->mem[zp_addr + 1];
    u16 addr = (hh << 8) + ll;
    u8  data = this->mem[addr];
    return {data, addr};
}

/*
 * ind,Y --- indirect, Y-indexed --- OPC ($LL),Y 
 *
 * Operand is zeropage address; effective address is word in (LL, LL + 1) 
 * incremented by Y with carry: C.w($00LL) + Y.
 */
AddrDataPair Cpu::addr_ind_Y()
{
    u8 ll = this->mem[this->PC++];
    u8 hh = 0x00;                        // zero page
    u16 zp_addr = ll;
    ll = this->mem[zp_addr];
    hh = this->mem[zp_addr + 1];
    u16 addr = (hh << 8) + ll + this->Y; // with carry
    u8  data = this->mem[addr];
    return {data, addr};
}

/*
 * rel --- relative --- OPC $BB 
 *
 * Branch target is PC + signed offset BB ***.
 */
AddrDataPair Cpu::addr_rel()
{
    u8 offset = this->mem[this->PC++];
    u16 addr = this->PC + offset;        // last PC or this? 
    u8 data = this->mem[addr]; 
    return {data, addr};
}

/*
 * zpg --- zeropage --- OPC $LL 
 *
 * Operand is zeropage address (hi-byte is zero, address = $00LL).
 */
AddrDataPair Cpu::addr_zpg()
{

    u8 ll = this->mem[this->PC++];
    u16 addr = ll;
    u8 data = this->mem[addr];
    return {data, addr};
}

/*
 * zpg,X --- zeropage, X-indexed --- OPC $LL,X 
 *
 * Operand is zeropage address; effective address is address incremented by X 
 * without carry **.
 */
AddrDataPair Cpu::addr_zpg_X()
{
    u8 ll = this->mem[this->PC++];
    u16 addr = ll + this->X; // TODO check add done in 8 bit space?
    u8 data = this->mem[addr];
    return {data, addr};
}

/*
 * zpg,Y --- zeropage, Y-indexed --- OPC $LL,Y
 *
 * Operand is zeropage address; effective address is address incremented by Y
 * without carry **.
 */
AddrDataPair Cpu::addr_zpg_Y()
{
    u8 ll = this->mem[this->PC++];
    u16 addr = ll + this->Y;
    u8 data = this->mem[addr];
    return {data, addr};
}

/*****************************************************************************
 *
 * Instruction implementations.
 *
 *****************************************************************************/

void Cpu::reset() 
{
    printf("CPU reset.\n");
    // Done at startupt: <https://www.csh.rit.edu/~moffitt/docs/6502.html#BOOT>
    this->SR  &=  STATUS_MASK_I;                     // ; disable interrupts
    this->PC   =  this->mem[RESET_VECTOR];           // ; put PC at where reset vector pointed
    this->PC  |= (this->mem[RESET_VECTOR + 1]) << 8;
    
    // TODO Do this through a reset routine
    this->SP  = STACK_PAGE_HIGH;                     // ; init stack pointer to ceiling of stack page ()
    this->SR &= ~STATUS_MASK_I;                      // ; enable interrupts again
}



u8 Cpu::fetch_execute_next() 
{
    u8 op = this->mem[this->PC++];
    switch (op) {
        case Instruction::NOP: 
        {
            printf("NOP\n");
            return 1;
        }
        case Instruction::LDA_I: 
        {
            printf("LDA_I\n");
            this->ACC = addr_imm().data;
            return 2;
        }
        case Instruction::JMP_ABS:
        {
            printf("JMP_ABS\n");
            this->PC = addr_abs().addr;
            return 3;
        }
        case Instruction::BRK: 
        {
            printf("Hit BRK instruction. Exit for now.\n");
            exit(0); // TODO implement correctly
        }
        default:
        {
            printf("Unknown instruction: %02X at %02X", op, (PC - 1));
            exit(1);
        }
    }
}

void Cpu::print_status() 
{
    printf("ACC: %02X\tX: %02X\tY: %02X\n", this->ACC, this->X, this->Y);
    printf("PC: %04X\n", this->PC);
    printf("CARRY (C):\t%d\tZERO (Z):\t%d\tIRQB (I):\t%d\tDECIMAL (D):\t%d\n", 
            (bool)(this->SR & STATUS_MASK_C),
            (bool)(this->SR & STATUS_MASK_Z),
            (bool)(this->SR & STATUS_MASK_I),
            (bool)(this->SR & STATUS_MASK_D)
    );
    printf("BRK (B):\t%d\tUNUSED:\t\t%d\tOVERFLOW (V):\t%d\tNEGATIVE (N):\t%d\n\n", 
            (bool)(this->SR & STATUS_MASK_B),
            (bool)(this->SR & STATUS_MASK_UNUSED),
            (bool)(this->SR & STATUS_MASK_V),
            (bool)(this->SR & STATUS_MASK_N)
    );
}
