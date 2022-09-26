#include "m6502.h" 

#include <bitset>
#include <iostream>

Cpu::Cpu(Memory &mem) : mem(mem) {
    populate_instruction_table();
    this->reset();
}

u8 Cpu::fetch_execute_next() 
{
    // fetch
    u8 opcode = this->mem[this->PC++];
    auto instr = instruction_table[opcode];
    
    // execute
    u8 cycles_taken = instr.execute(this);

    // return # of elapsed cpu cycles
    return cycles_taken;
}

void Cpu::reset() 
{
#ifdef DEBUG_PRINTS
    printf("RESET signal\n");
#endif
    // Done at startupt: <https://www.csh.rit.edu/~moffitt/docs/6502.html#BOOT>
    this->SR |= (1 << BIT_I);                                     // ; disable interrupts
    this->PC  =  this->mem[Layout::RESET_VECTOR];                 // ; put PC at where reset vector pointed
    this->PC |= (this->mem[Layout::RESET_VECTOR + 1]) << 8;
    
    // TODO Do this through a reset routine
    this->SP  = Layout::STACK_PAGE_HIGH - Layout::STACK_PAGE_LOW; // ; init stack pointer to ceiling of stack page ()
    this->SR &= ~(1 << BIT_I);                                    // ; enable interrupts again
}

void Cpu::irq() 
{
#ifdef DEBUG_PRINTS
    printf("IRQ signal\n");
#endif
    if (!(this->SR & (1 << BIT_I)))
        this->nmi();
}

void Cpu::nmi()
{
#ifdef DEBUG_PRINTS
    printf("NMI signal\n");
#endif
    // push PC onto stack
    this->mem[Layout::STACK_PAGE_LOW + (this->SP--)] = (PC & 0xFF00) >> 8; 
    this->mem[Layout::STACK_PAGE_LOW + (this->SP--)] = (PC & 0x00FF); 

    // push SR onto stack
    u8 sr = this->SR;
    sr |= (0 << BIT_B);
    sr |= (1 << BIT_UNUSED);
    this->mem[Layout::STACK_PAGE_LOW + (this->SP--)] = sr; 

    // set I bit
    this->SR |= (1 << BIT_I);
   
    // set PC to the IRQ vector
    this->PC  =  this->mem[Layout::IRQ_BRK_VECTOR];
    this->PC |= (this->mem[Layout::IRQ_BRK_VECTOR + 1]) << 8;
}
