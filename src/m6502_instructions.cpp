
#include "m6502.h"

/*
 * Few helper functions and constants
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

u8 generic_compare(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu), u8 register_value)
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 m = *(fetched.data_ptr);

    u8 res = register_value - m;

    set_status_bit(cpu, BIT_C, (register_value > m));
    set_status_bit(cpu, BIT_N, (res >> 7));
    set_status_bit(cpu, BIT_Z, (res == 0));

    return fetched.additional_cycles;
}

u8 generic_branch(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu), u8 bit, bool value)
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 b = get_status_bit(cpu, bit);
    if (b == value) {
        cpu->PC = fetched.address;
        // 1 extra cycle if branch occurs to same page
        // 2 extra cycles if branch occurs to different page
        return 1 + fetched.additional_cycles;
    }
    return 0;
}

void stack_push(Cpu *cpu, u8 byte)
{
    cpu->mem[Layout::STACK_PAGE_LOW + cpu->SP] = byte; 
    cpu->SP--;
}

u8 stack_pop(Cpu *cpu)
{
    cpu->SP++;
    return cpu->mem[Layout::STACK_PAGE_LOW + cpu->SP]; 
}

/******************************************************************************
 *
 * Arithmetic & logic
 *
 * TODO remove unnecessary parentheses. 
 *
 ******************************************************************************/

// Add with carry
u8 op_adc(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    auto fetched = addr_mode(cpu);
    u16 a = (u16) cpu->ACC; 
    u16 m = (u16) *(fetched.data_ptr); 
    u16 c = (u16) get_status_bit(cpu, BIT_C);

    // op: perform operation in 16-bit space to capture potential carry bit
    u16 res = a + m + c;

    set_status_bit(cpu, BIT_C, (res >> 8));
    set_status_bit(cpu, BIT_N, ((res & 0xFF) >> 7));
    set_status_bit(cpu, BIT_Z, ((res & 0xFF) == 0));
    set_status_bit(cpu, BIT_V, ((~(a ^ m)) & (a ^ c) & 0x80)); // Thanks https://stackoverflow.com/a/16861251/5350029

    cpu->ACC = (u8) (res & 0xFF);
    return fetched.additional_cycles;
}

// Subtract with carry
u8 op_sbc(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u16 a = (u16) cpu->ACC; 
    u16 m = (u16) *(fetched.data_ptr); 
    u16 c = (u16) get_status_bit(cpu, BIT_C);
    
    // op: same as op_adc but we flip b
    u16 res = a + (m ^ 0xFF) + c;

    set_status_bit(cpu, BIT_C, (res >> 8));
    set_status_bit(cpu, BIT_N, ((res & 0xFF) >> 7));
    set_status_bit(cpu, BIT_Z, ((res & 0xFF) == 0));
    set_status_bit(cpu, BIT_V, ((~(a ^ m)) & (a ^ c) & 0x80)); // Thanks https://stackoverflow.com/a/16861251/5350029
    cpu->ACC = (u8) (res & 0xFF);

    return fetched.additional_cycles;
}

// Arithmetic shift left 
u8 op_asl(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u16 m = (u16) *(fetched.data_ptr);

    // op
    u16 res = m << 1;

    set_status_bit(cpu, BIT_C, (res >> 8));
    set_status_bit(cpu, BIT_N, ((res & 0xFF) >> 7));
    set_status_bit(cpu, BIT_Z, ((res & 0xFF) == 0));
    *(fetched.data_ptr) = (u8) (res & 0xFF);

    return fetched.additional_cycles;
}

// logical shift right 
u8 op_lsr(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 m = *(fetched.data_ptr);

    // op
    u8 res = m >> 1;

    set_status_bit(cpu, BIT_C, (bool) (m & 1));
    set_status_bit(cpu, BIT_N, LOW);
    set_status_bit(cpu, BIT_Z, (res == 0));
    *(fetched.data_ptr) = res;

    return fetched.additional_cycles;
}

// Rotate left
u8 op_rol(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u16 m = (u16) *(fetched.data_ptr);
    u16 c = get_status_bit(cpu, BIT_C);

    // op
    u16 res = (m << 1) + c;

    set_status_bit(cpu, BIT_C, (res >> 8));
    set_status_bit(cpu, BIT_N, ((res & 0xFF) >> 7));
    set_status_bit(cpu, BIT_Z, ((res & 0xFF) == 0));
    *(fetched.data_ptr) = (u8) (res & 0xFF);

    return fetched.additional_cycles;
}

// Rotate right
u8 op_ror(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 m = *(fetched.data_ptr);
    u16 c = get_status_bit(cpu, BIT_C);

    // op
    u8 res = (m >> 1) + (c << 7);

    set_status_bit(cpu, BIT_C, (bool) (m & 1));
    set_status_bit(cpu, BIT_N, (res >> 7));
    set_status_bit(cpu, BIT_Z, (res == 0));
    *(fetched.data_ptr) = (u8) (res & 0xFF);

    return fetched.additional_cycles;
}



// Compare with Cpu->X
u8 op_cpx(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    return generic_compare(cpu, addr_mode, cpu->X);
}

// Compare with Cpu->Y
u8 op_cpy(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    return generic_compare(cpu, addr_mode, cpu->Y);
}

// Compare with Cpu->AXX
u8 op_cmp(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    return generic_compare(cpu, addr_mode, cpu->ACC);
}

// Increment 
u8 op_inc(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 m = *(fetched.data_ptr);
    
    m++;

    set_status_bit(cpu, BIT_N, (m >> 7));
    set_status_bit(cpu, BIT_Z, (m == 0));
    *(fetched.data_ptr) = m;
    
    return fetched.additional_cycles;
}

// Decrement 
u8 op_dec(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 m = *(fetched.data_ptr);
    
    m--;

    set_status_bit(cpu, BIT_N, (m >> 7));
    set_status_bit(cpu, BIT_Z, (m == 0));
    *(fetched.data_ptr) = m;
    
    return fetched.additional_cycles;
}

// And  
u8 op_and(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 a = cpu->ACC;
    u8 m = *(fetched.data_ptr);
    
    a &= m;

    set_status_bit(cpu, BIT_N, (a >> 7));
    set_status_bit(cpu, BIT_Z, (a == 0));
    cpu->ACC = a;
    
    return fetched.additional_cycles;
}

// Or with Cpu->ACC 
u8 op_ora(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 a = cpu->ACC;
    u8 m = *(fetched.data_ptr);
    
    a |= m;

    set_status_bit(cpu, BIT_N, (a >> 7));
    set_status_bit(cpu, BIT_Z, (a == 0));
    cpu->ACC = a;
    
    return fetched.additional_cycles;
}

// Exclusive or
u8 op_eor(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 a = cpu->ACC;
    u8 m = *(fetched.data_ptr);
    
    a ^= m;

    set_status_bit(cpu, BIT_N, (a >> 7));
    set_status_bit(cpu, BIT_Z, (a == 0));
    cpu->ACC = a;
    
    return fetched.additional_cycles;
}

// Increment Cpu->X
u8 op_inx(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    u8 x = cpu->X;
   
    x++;

    set_status_bit(cpu, BIT_N, (x >> 7));
    set_status_bit(cpu, BIT_Z, (x == 0));
    cpu->X = x;
    
    return 0; 
}

// Increment Cpu->Y
u8 op_iny(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    u8 y = cpu->Y;
   
    y++;

    set_status_bit(cpu, BIT_N, (y >> 7));
    set_status_bit(cpu, BIT_Z, (y == 0));
    cpu->Y  = y;
    
    return 0;
}

// Decrement Cpu->X
u8 op_dex(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    u8 x = cpu->X;
   
    x--;

    set_status_bit(cpu, BIT_N, (x >> 7));
    set_status_bit(cpu, BIT_Z, (x == 0));
    cpu->X = x;
    
    return 0;
}

// Decrement Cpu->Y
u8 op_dey(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    u8 y = cpu->Y;
   
    y--;

    set_status_bit(cpu, BIT_N, (y >> 7));
    set_status_bit(cpu, BIT_Z, (y == 0));
    cpu->Y = y;
    
    return 0;
}

// Bit test
u8 op_bit(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 a = cpu->ACC;
    u8 m = *(fetched.data_ptr);
    
    u8 res = a & m;

    set_status_bit(cpu, BIT_Z, (res == 0));
    set_status_bit(cpu, BIT_N, (m >> 7));
    set_status_bit(cpu, BIT_V, (m >> 6));

    return fetched.additional_cycles;    
}

/******************************************************************************
 *
 * Memory 
 *
 ******************************************************************************/

// Push Cpu->SR onto stack 
u8 op_php(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    stack_push(cpu, cpu->SR);
    return 0;
}

// Pull Cpu->SR from stack
u8 op_plp(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    cpu->SR = stack_pop(cpu);
    return 0;
}

// Push Cpu->ACC onto stack 
u8 op_pha(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    stack_push(cpu, cpu->ACC);
    return 0;
}

// Pull Cpu->ACC from stack
u8 op_pla(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    cpu->ACC = stack_pop(cpu);
    set_status_bit(cpu, BIT_N, (cpu->ACC >> 7));
    set_status_bit(cpu, BIT_Z, (cpu->ACC == 0));
    return 0;
}

// Store Cpu->ACC
u8 op_sta(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 a = cpu->ACC;
    u16 m = fetched.address;
   
    cpu->mem[m] = a;

    return fetched.additional_cycles;
}

// Store Cpu->X
u8 op_stx(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 x = cpu->X;
    u16 m = fetched.address;
   
    cpu->mem[m] = x;

    return fetched.additional_cycles;
}

// Store Cpu->Y
u8 op_sty(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 y = cpu->Y;
    u16 m = fetched.address;
   
    cpu->mem[m] = y;

    return fetched.additional_cycles;
}

// Load Cpu->ACC
u8 op_lda(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u16 m = fetched.address;
   
    cpu->ACC = cpu->mem[m];
    set_status_bit(cpu, BIT_N, (cpu->ACC >> 7));
    set_status_bit(cpu, BIT_Z, (cpu->ACC == 0));

    return fetched.additional_cycles;
}

// Load Cpu->X
u8 op_ldx(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u16 m = fetched.address;
   
    cpu->X = cpu->mem[m];
    set_status_bit(cpu, BIT_N, (cpu->X >> 7));
    set_status_bit(cpu, BIT_Z, (cpu->X == 0));

    return fetched.additional_cycles;
}

// Load Cpu->Y
u8 op_ldy(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u16 m = fetched.address;
   
    cpu->Y = cpu->mem[m];
    set_status_bit(cpu, BIT_N, (cpu->Y >> 7));
    set_status_bit(cpu, BIT_Z, (cpu->Y == 0));

    return fetched.additional_cycles;
}

// Transfer Cpu->X --> Cpu->ACC
u8 op_txa(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    cpu->ACC = cpu->X;
    set_status_bit(cpu, BIT_N, (cpu->ACC >> 7));
    set_status_bit(cpu, BIT_Z, (cpu->ACC == 0));
    return 0;
}

// Transfer Cpu->Y --> Cpu->ACC
u8 op_tya(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    cpu->ACC = cpu->Y;
    set_status_bit(cpu, BIT_N, (cpu->ACC >> 7));
    set_status_bit(cpu, BIT_Z, (cpu->ACC == 0));
    return 0;
}

// Transfer Cpu->ACC --> Cpu->X
u8 op_tax(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    cpu->X = cpu->ACC;
    set_status_bit(cpu, BIT_N, (cpu->X >> 7));
    set_status_bit(cpu, BIT_Z, (cpu->X == 0));
    return 0;
}

// Transfer Cpu->ACC --> Cpu->Y
u8 op_tay(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    cpu->Y = cpu->ACC;
    set_status_bit(cpu, BIT_N, (cpu->Y >> 7));
    set_status_bit(cpu, BIT_Z, (cpu->Y == 0));
    return 0;
}

// Transfer Cpu->X --> Cpu->SP
u8 op_txs(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    cpu->SP = cpu->X;
    return 0;
}

// Transfer Cpu->SP --> Cpu->X
u8 op_tsx(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    cpu->X = cpu->SP;
    set_status_bit(cpu, BIT_N, (cpu->X >> 7));
    set_status_bit(cpu, BIT_Z, (cpu->X == 0));
    return 0;
}

/******************************************************************************
 *
 * Branching, Jumps, etc.
 *
 ******************************************************************************/

// Jump 
u8 op_jmp(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    cpu->PC = fetched.address;
    return 0; // TODO check ??
}

// Jump subroutine
u8 op_jsr(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);

    // push return address onto stack
    u16 return_address = cpu->PC - 1; // @caution: offset
    stack_push(cpu, (u8) ((return_address & 0xFF00) >> 8)); // @caution: endianness
    stack_push(cpu, (u8) (return_address & 0x00FF));

    // jump
    cpu->PC = fetched.address;

    return 0;
}

// Branch on equal
u8 op_beq(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    return generic_branch(cpu, addr_mode, BIT_Z, HIGH);
}

// Branch on not equal
u8 op_bne(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    return generic_branch(cpu, addr_mode, BIT_Z, LOW);
}

// Branch on overflow set
u8 op_bvs(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    return generic_branch(cpu, addr_mode, BIT_V, HIGH);
}

// Branch on overflow clear
u8 op_bvc(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    return generic_branch(cpu, addr_mode, BIT_V, LOW);
}

// Branch on plus 
u8 op_bpl(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    return generic_branch(cpu, addr_mode, BIT_N, LOW);
}

// Branch on minus 
u8 op_bmi(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    return generic_branch(cpu, addr_mode, BIT_N, HIGH);
}

// Branch on carry set
u8 op_bcs(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    return generic_branch(cpu, addr_mode, BIT_C, HIGH);
}

// Branch on carry clear
u8 op_bcc(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    return generic_branch(cpu, addr_mode, BIT_C, LOW);
}

// Return from interrupt
u8 op_rti(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    cpu->SR = stack_pop(cpu); 
    cpu->PC = (u16) stack_pop(cpu);
    cpu->PC |= (u16) (stack_pop(cpu) << 8);
    return 0;
}

// Return from subroutine
u8 op_rts(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    cpu->PC = (u16) stack_pop(cpu);
    cpu->PC |= (u16) (stack_pop(cpu) << 8);
    cpu->PC++;
    return 0;
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
    return 0;
} 

// Set decimal
u8 op_sed(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    set_status_bit(cpu, BIT_D, HIGH);
    return 0;
}

// set interrupt disable
u8 op_sei(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    set_status_bit(cpu, BIT_I, HIGH);
    return 0;
}

// Clear carry
u8 op_clc(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    set_status_bit(cpu, BIT_C, LOW);
    return 0;
}

// Clear decimal
u8 op_cld(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    set_status_bit(cpu, BIT_D, LOW);
    return 0;
}

// Clear overflow
u8 op_clv(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    set_status_bit(cpu, BIT_V, LOW);
    return 0;
}

// Clear interrupt disable
u8 op_cli(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    set_status_bit(cpu, BIT_I, LOW);
    return 0;
}

/******************************************************************************
 *
 * Misc.
 *
 ******************************************************************************/

// Break
u8 op_brk(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{

    //printf("Hit BRK instruction. Exit for now.\n");
    //exit(0); // TODO implement correctly
    return 0;
} 


// No operation
u8 op_nop(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    return 0;
} 

/*
 * Special instruction for invalid op codes. Exits the program with an error 
 * message.
 */
u8 OP_INVALID(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    printf("Error: Hit unkown opcode 0x%02X . Exiting.\n", cpu->mem[cpu->PC - 1]);
    exit(1);
    return 0; // unreachable
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

    const auto IMPLIED = nullptr;

    // "zero" fill
    for (int i = 0; i < 256; i++)
        instruction_table[i] = {OP_INVALID, IMPLIED, 0};

    // populate
    // row 0
    instruction_table[0x00] = {op_brk,  IMPLIED,    7};
    instruction_table[0x01] = {op_ora,  addr_ind_X, 6};
    instruction_table[0x05] = {op_ora,  addr_zpg,   3};
    instruction_table[0x06] = {op_asl,  addr_zpg,   5};
    instruction_table[0x08] = {op_php,  IMPLIED,    3};
    instruction_table[0x09] = {op_ora,  addr_imm,   2};
    instruction_table[0x0A] = {op_asl,  addr_acc,   2};
    instruction_table[0x0D] = {op_ora,  addr_abs,   4};
    instruction_table[0x0E] = {op_asl,  addr_abs,   6};
    
    // row 1
    instruction_table[0x10] = {op_bpl,  addr_rel,   2};
    instruction_table[0x11] = {op_ora,  addr_ind_Y, 5};
    instruction_table[0x15] = {op_ora,  addr_zpg_X, 4};
    instruction_table[0x16] = {op_asl,  addr_zpg_X, 6};
    instruction_table[0x18] = {op_clc,  IMPLIED,    2};
    instruction_table[0x19] = {op_ora,  addr_abs_Y, 4};
    instruction_table[0x1D] = {op_ora,  addr_abs_X, 4};
    instruction_table[0x1E] = {op_asl,  addr_abs_X, 7};

    // row 2
    instruction_table[0x20] = {op_jsr,  addr_abs,   6};
    instruction_table[0x21] = {op_and,  addr_ind_X, 6};
    instruction_table[0x24] = {op_bit,  addr_zpg,   3};
    instruction_table[0x25] = {op_and,  addr_zpg,   3};
    instruction_table[0x26] = {op_rol,  addr_zpg,   5};
    instruction_table[0x28] = {op_plp,  IMPLIED,    4};
    instruction_table[0x29] = {op_and,  addr_imm,   2};
    instruction_table[0x2A] = {op_rol,  addr_acc,   2};
    instruction_table[0x2C] = {op_bit,  addr_abs,   4};
    instruction_table[0x2D] = {op_and,  addr_abs,   4};
    instruction_table[0x2E] = {op_rol,  addr_abs,   6};

    // row 3
    instruction_table[0x30] = {op_bmi,  addr_rel,   2};
    instruction_table[0x31] = {op_and,  addr_ind_Y, 5};
    instruction_table[0x35] = {op_and,  addr_zpg_X, 4};
    instruction_table[0x36] = {op_rol,  addr_zpg_X, 6};
    instruction_table[0x38] = {op_sec,  IMPLIED,    6};
    instruction_table[0x39] = {op_and,  addr_abs_Y, 4};
    instruction_table[0x3D] = {op_and,  addr_abs_X, 4};
    instruction_table[0x3E] = {op_rol,  addr_abs_X, 7};

    // row 4
    instruction_table[0x40] = {op_rti,  IMPLIED,    6};
    instruction_table[0x41] = {op_eor,  addr_ind_X, 6};
    instruction_table[0x45] = {op_eor,  addr_zpg,   3};
    instruction_table[0x46] = {op_lsr,  addr_zpg,   5};
    instruction_table[0x48] = {op_pha,  IMPLIED,    3};
    instruction_table[0x49] = {op_eor,  addr_imm,   2};
    instruction_table[0x4A] = {op_lsr,  addr_acc,   2};
    instruction_table[0x4C] = {op_jmp,  addr_abs,   3};
    instruction_table[0x4D] = {op_eor,  addr_abs,   3};
    instruction_table[0x4E] = {op_lsr,  addr_abs,   3};

    // row 5
    instruction_table[0x50] = {op_bvc,  addr_rel,   2};
    instruction_table[0x51] = {op_eor,  addr_ind_Y, 5};
    instruction_table[0x55] = {op_eor,  addr_zpg_X, 4};
    instruction_table[0x56] = {op_lsr,  addr_zpg_X, 6};
    instruction_table[0x58] = {op_cli,  IMPLIED,    2}; // TODO??
    instruction_table[0x59] = {op_cli,  IMPLIED,    2};
    instruction_table[0x5D] = {op_cli,  IMPLIED,    2};
    instruction_table[0x5E] = {op_cli,  IMPLIED,    2};

    // row 6
    instruction_table[0x60] = {op_bvc,  addr_rel,   2};
    instruction_table[0x61] = {op_eor,  addr_ind_Y, 5};
    instruction_table[0x65] = {op_eor,  addr_zpg_X, 4};
    instruction_table[0x66] = {op_lsr,  addr_zpg_X, 6};
    instruction_table[0x68] = {op_pla,  IMPLIED,    4};
    instruction_table[0x69] = {op_adc,  addr_imm,   2};
    instruction_table[0x6A] = {op_ror,  addr_acc,   2};
    instruction_table[0x6C] = {op_jmp,  addr_ind,   5};
    instruction_table[0x6D] = {op_adc,  addr_abs,   4};
    instruction_table[0x6E] = {op_ror,  addr_abs,   6};

    // row 7
    instruction_table[0x70] = {op_bvs,  addr_rel,   2};
    instruction_table[0x71] = {op_adc,  addr_ind_Y, 5};
    instruction_table[0x75] = {op_adc,  addr_zpg_X, 4};
    instruction_table[0x76] = {op_ror,  addr_zpg_X, 6};
    instruction_table[0x78] = {op_sei,  IMPLIED,    2};
    instruction_table[0x79] = {op_adc,  addr_abs_Y, 4};
    instruction_table[0x7D] = {op_adc,  addr_abs_X, 4};
    instruction_table[0x7E] = {op_ror,  addr_abs_X, 7};

    // row 8
    instruction_table[0x81] = {op_sta,  addr_ind_X, 6};
    instruction_table[0x84] = {op_sty,  addr_zpg,   3};
    instruction_table[0x85] = {op_sta,  addr_zpg,   3};
    instruction_table[0x86] = {op_stx,  addr_zpg,   3};
    instruction_table[0x88] = {op_dey,  IMPLIED,    2};
    instruction_table[0x8A] = {op_txa,  IMPLIED,    2};
    instruction_table[0x8C] = {op_sty,  addr_abs,   4};
    instruction_table[0x8D] = {op_sta,  addr_abs,   4};
    instruction_table[0x8E] = {op_stx,  addr_abs,   4};

    // row 9
    instruction_table[0x90] = {op_bcc, addr_rel,    2};
    instruction_table[0x91] = {op_sta, addr_ind_Y,  6};
    instruction_table[0x94] = {op_sty, addr_zpg_X,  4};
    instruction_table[0x95] = {op_sta, addr_zpg_X,  4};
    instruction_table[0x96] = {op_stx, addr_zpg_Y,  4};
    instruction_table[0x98] = {op_tya, IMPLIED,     2};
    instruction_table[0x99] = {op_sta, addr_abs_Y,  5};
    instruction_table[0x9A] = {op_txs, IMPLIED,     2};
    instruction_table[0x9D] = {op_sta, addr_abs_X,  5};

    // row A
    instruction_table[0xA0] = {op_ldy, addr_imm,    2};
    instruction_table[0xA1] = {op_lda, addr_ind_X,  6};
    instruction_table[0xA2] = {op_ldx, addr_imm,    2};
    instruction_table[0xA4] = {op_ldy, addr_zpg,    3};
    instruction_table[0xA5] = {op_lda, addr_zpg,    3};
    instruction_table[0xA6] = {op_ldx, addr_zpg,    3};
    instruction_table[0xA8] = {op_tay, IMPLIED,     2};
    instruction_table[0xA9] = {op_lda, addr_imm,    2};
    instruction_table[0xAA] = {op_tax, IMPLIED,     2};
    instruction_table[0xAC] = {op_ldy, addr_abs,    4};
    instruction_table[0xAD] = {op_lda, addr_abs,    4};
    instruction_table[0xAE] = {op_ldx, addr_abs,    4};

    // row B
    instruction_table[0xB0] = {op_bcs, addr_rel,    2};
    instruction_table[0xB1] = {op_lda, addr_ind_Y,  5};
    instruction_table[0xB4] = {op_ldy, addr_zpg_X,  4};
    instruction_table[0xB5] = {op_lda, addr_zpg_X,  4};
    instruction_table[0xB6] = {op_ldx, addr_zpg_Y,  4};
    instruction_table[0xB8] = {op_clv, IMPLIED,     2};
    instruction_table[0xB9] = {op_lda, addr_abs_Y,  4};
    instruction_table[0xBA] = {op_tsx, IMPLIED,     2};
    instruction_table[0xBC] = {op_ldy, addr_abs_X,  4};
    instruction_table[0xBD] = {op_lda, addr_abs_X,  4};
    instruction_table[0xBE] = {op_ldx, addr_abs_Y,  4};
    
    // row C
    instruction_table[0xC0] = {op_cpy, addr_imm,    2};
    instruction_table[0xC1] = {op_cmp, addr_ind_X,  6};
    instruction_table[0xC4] = {op_cpy, addr_zpg,    3};
    instruction_table[0xC5] = {op_cmp, addr_zpg,    3};
    instruction_table[0xC6] = {op_dec, addr_zpg,    5};
    instruction_table[0xC8] = {op_iny, IMPLIED,     2};
    instruction_table[0xC9] = {op_cmp, addr_imm,    2};
    instruction_table[0xCA] = {op_dex, IMPLIED,     2};
    instruction_table[0xCC] = {op_cpy, addr_abs,    4};
    instruction_table[0xCD] = {op_cmp, addr_abs,    4};
    instruction_table[0xCE] = {op_dec, addr_abs,    6};

    // row D
    instruction_table[0xD0] = {op_bne, addr_rel,    2};
    instruction_table[0xD1] = {op_cmp, addr_ind_Y,  5};
    instruction_table[0xD5] = {op_cmp, addr_zpg_X,  4};
    instruction_table[0xD6] = {op_dec, addr_zpg_X,  6};
    instruction_table[0xD8] = {op_cld, IMPLIED,     2};
    instruction_table[0xD9] = {op_cmp, addr_abs_Y,  4};
    instruction_table[0xDD] = {op_cmp, addr_abs_X,  4};
    instruction_table[0xDE] = {op_dec, addr_abs_X,  7};

    // row E
    instruction_table[0xE0] = {op_cpx, addr_imm,    2};
    instruction_table[0xE1] = {op_sbc, addr_ind_X,  6};
    instruction_table[0xE4] = {op_cpx, addr_zpg,    3};
    instruction_table[0xE5] = {op_sbc, addr_zpg,    3};
    instruction_table[0xE6] = {op_inc, addr_zpg,    5};
    instruction_table[0xE8] = {op_inx, IMPLIED,     2};
    instruction_table[0xE9] = {op_sbc, addr_imm,    2};
    instruction_table[0xEA] = {op_nop, IMPLIED,     2};
    instruction_table[0xEC] = {op_cpx, addr_abs,    4};
    instruction_table[0xED] = {op_sbc, addr_abs,    4};
    instruction_table[0xEE] = {op_inc, addr_abs,    6};

    // row F
    instruction_table[0xF0] = {op_beq, addr_rel,    2};
    instruction_table[0xF1] = {op_sbc, addr_ind_Y,  5};
    instruction_table[0xF5] = {op_sbc, addr_zpg_X,  4};
    instruction_table[0xF6] = {op_inc, addr_zpg_X,  6};
    instruction_table[0xF8] = {op_sed, IMPLIED,     2};
    instruction_table[0xF9] = {op_sbc, addr_abs_Y,  4};
    instruction_table[0xFD] = {op_sbc, addr_abs_X,  4};
    instruction_table[0xFE] = {op_inc, addr_abs_X,  7};

    // well. That's sure to have no typos or mistakes.
}

