
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
 * Few helper functions and constants
 */
const u8 MSB_MASK = 0b1000000;
const u8 LSB_MASK = 0b0000001;

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

    return 1 + fetched.additional_cycles; // TODO add 1?
}

void stack_push(Cpu *cpu, u8 byte)
{
    cpu->mem[cpu->SP--] = byte; 
}

u8 stack_pop(Cpu *cpu)
{
    return cpu->mem[cpu->SP++]; 
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
    set_status_bit(cpu, BIT_N, ((res && 0xFF) >> 7));
    set_status_bit(cpu, BIT_Z, ((res && 0xFF) == 0));
    set_status_bit(cpu, BIT_V, ((~(a ^ m)) & (a ^ c) & 0x80)); // Thanks https://stackoverflow.com/a/16861251/5350029

    cpu->ACC = (u8) (res & 0xFF);
    return 1 + fetched.additional_cycles;
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
    set_status_bit(cpu, BIT_N, ((res && 0xFF) >> 7));
    set_status_bit(cpu, BIT_Z, ((res && 0xFF) == 0));
    set_status_bit(cpu, BIT_V, ((~(a ^ m)) & (a ^ c) & 0x80)); // Thanks https://stackoverflow.com/a/16861251/5350029
    cpu->ACC = (u8) (res & 0xFF);

    return 1 + fetched.additional_cycles;
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
    set_status_bit(cpu, BIT_N, ((res && 0xFF) >> 7));
    set_status_bit(cpu, BIT_Z, ((res && 0xFF) == 0));
    *(fetched.data_ptr) = (u8) (res & 0xFF);

    return 1 + fetched.additional_cycles;
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

    return 1 + fetched.additional_cycles;
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
    set_status_bit(cpu, BIT_N, ((res && 0xFF) >> 7));
    set_status_bit(cpu, BIT_Z, ((res && 0xFF) == 0));
    *(fetched.data_ptr) = (u8) (res & 0xFF);

    return 1 + fetched.additional_cycles;
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

    return 1 + fetched.additional_cycles;
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
    
    return 1 + fetched.additional_cycles;
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
    
    return 1 + fetched.additional_cycles;
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
    
    return 1 + fetched.additional_cycles;
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
    
    return 1 + fetched.additional_cycles;
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
    
    return 1 + fetched.additional_cycles;
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
    
    return 2; 
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
    
    return 2;
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
    
    return 2; 
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
    
    return 2; 
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

    return 1 + fetched.additional_cycles;    
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
    return 3;
}

// Pull Cpu->SR from stack
u8 op_plp(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    cpu->SR = stack_pop(cpu);
    return 4;
}

// Push Cpu->ACC onto stack 
u8 op_pha(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    stack_push(cpu, cpu->ACC);
    return 3;
}

// Pull Cpu->ACC from stack
u8 op_pla(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    cpu->ACC = stack_pop(cpu);
    set_status_bit(cpu, BIT_N, (cpu->ACC >> 7));
    set_status_bit(cpu, BIT_Z, (cpu->ACC == 0));
    return 4;
}

// Store Cpu->ACC
u8 op_sta(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 a = cpu->ACC;
    u16 m = fetched.address;
   
    cpu->mem[m] = a;

    return 2 + fetched.additional_cycles;
}

// Store Cpu->X
u8 op_stx(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 x = cpu->X;
    u16 m = fetched.address;
   
    cpu->mem[m] = x;

    return 2 + fetched.additional_cycles; // should be 1 + additional?
}

// Store Cpu->Y
u8 op_sty(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 y = cpu->Y;
    u16 m = fetched.address;
   
    cpu->mem[m] = y;

    return 2 + fetched.additional_cycles;
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

    return 1 + fetched.additional_cycles;
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

    return 1 + fetched.additional_cycles;
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

    return 1 + fetched.additional_cycles;
}

// Transfer Cpu->X --> Cpu->ACC
u8 op_txa(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    cpu->ACC = cpu->X;
    set_status_bit(cpu, BIT_N, (cpu->ACC >> 7));
    set_status_bit(cpu, BIT_Z, (cpu->ACC == 0));
    return 2;
}

// Transfer Cpu->Y --> Cpu->ACC
u8 op_tya(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    cpu->ACC = cpu->Y;
    set_status_bit(cpu, BIT_N, (cpu->ACC >> 7));
    set_status_bit(cpu, BIT_Z, (cpu->ACC == 0));
    return 2;
}

// Transfer Cpu->ACC --> Cpu->X
u8 op_tax(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    cpu->X = cpu->ACC;
    set_status_bit(cpu, BIT_N, (cpu->X >> 7));
    set_status_bit(cpu, BIT_Z, (cpu->X == 0));
    return 2;
}

// Transfer Cpu->ACC --> Cpu->Y
u8 op_tay(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    cpu->Y = cpu->ACC;
    set_status_bit(cpu, BIT_N, (cpu->Y >> 7));
    set_status_bit(cpu, BIT_Z, (cpu->Y == 0));
    return 2;
}

// Transfer Cpu->X --> Cpu->SP
u8 op_txs(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    cpu->SP = cpu->X;
    return 2;
}

// Transfer Cpu->SP --> Cpu->X
u8 op_tsx(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    cpu->X = cpu->SP;
    set_status_bit(cpu, BIT_N, (cpu->X >> 7));
    set_status_bit(cpu, BIT_Z, (cpu->X == 0));
    return 2;
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
    return 3 + fetched.additional_cycles; // TODO check ??
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

    return 6;
}

// Branch on equal
u8 op_beq(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 z = get_status_bit(cpu, BIT_Z);
    if (z == 1)
        cpu->PC = fetched.address;
    return 2 + fetched.additional_cycles; // TODO
}

// Branch on not equal
u8 op_bne(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 z = get_status_bit(cpu, BIT_Z);
    if (z == 0)
        cpu->PC = fetched.address;
    return 2 + fetched.additional_cycles; // TODO
}

// Branch on overflow set
u8 op_bvs(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 v = get_status_bit(cpu, BIT_V);
    if (v == 1)
        cpu->PC = fetched.address;
    return 2 + fetched.additional_cycles; // TODO
}

// Branch on overflow clear
u8 op_bvc(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 v = get_status_bit(cpu, BIT_V);
    if (v == 0)
        cpu->PC = fetched.address;
    return 2 + fetched.additional_cycles; // TODO
}

// Branch on plus 
u8 op_bpl(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 n = get_status_bit(cpu, BIT_N);
    if (n == 0)
        cpu->PC = fetched.address;
    return 2 + fetched.additional_cycles; // TODO
}

// Branch on minus 
u8 op_bmi(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 n = get_status_bit(cpu, BIT_N);
    if (n == 1)
        cpu->PC = fetched.address;
    return 2 + fetched.additional_cycles; // TODO
}

// Branch on carry set
u8 op_bcs(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 c = get_status_bit(cpu, BIT_C);
    if (c == 1)
        cpu->PC = fetched.address;
    return 2 + fetched.additional_cycles; // TODO
}

// Branch on carry clear
u8 op_bcc(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    auto fetched = addr_mode(cpu);
    u8 c = get_status_bit(cpu, BIT_C);
    if (c == 0)
        cpu->PC = fetched.address;
    return 2 + fetched.additional_cycles; // TODO
}

// Return from interrupt
u8 op_rti(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    cpu->SR = stack_pop(cpu); 
    cpu->PC = (u16) stack_pop(cpu);
    cpu->PC |= (u16) (stack_pop(cpu) << 8);
    return 6;
}

// Return from subroutine
u8 op_rts(Cpu *cpu, AddrModeRet (*addr_mode)(Cpu *cpu)) 
{
    // TODO test
    cpu->PC = (u16) stack_pop(cpu);
    cpu->PC |= (u16) (stack_pop(cpu) << 8);
    cpu->PC++;
    return 6;
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

    const auto IMPLIED = nullptr;

    // "zero" fill
    for (int i = 0; i < 256; i++)
        instruction_table[i] = {OP_INVALID, nullptr};

    // populate
    // row 1
    instruction_table[0x00] = {op_brk, IMPLIED,    "BRK"};
    instruction_table[0x01] = {op_ora, addr_X_ind, "ORA X,ind"};
    instruction_table[0x05] = {op_ora, addr_zpg,   "ORA zpg"};
    instruction_table[0x06] = {op_asl, addr_zpg,   "ASL zpg"};
    instruction_table[0x08] = {op_php, IMPLIED,    "PHP"};
    instruction_table[0x09] = {op_ora, addr_imm,   "ORA #"};
    instruction_table[0x0A] = {op_asl, addr_acc,   "ASL A"};
    instruction_table[0x0D] = {op_ora, addr_abs,   "ORA abs"};
    instruction_table[0x0E] = {op_asl, addr_abs,   "ASL abs"};
    
    // row 2
    instruction_table[0x18] = {op_clc, IMPLIED,    "CLC"};

    instruction_table[0x69] = {op_adc, addr_imm,   "ADC #"};
    instruction_table[0x4C] = {op_jmp, addr_abs,   "JMP abs"};
    instruction_table[0xA9] = {op_lda, addr_imm,   "LDA #"};
    instruction_table[0xEA] = {op_nop, IMPLIED,    "NOP"};
}

