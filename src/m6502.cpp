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

#ifdef DEBUG_PRINTS
    this->print_status();
#endif

    // return # of elapsed cpu cycles
    return cycles_taken;
}

void Cpu::reset() 
{
    printf("RESET signal\n");
    // Done at startupt: <https://www.csh.rit.edu/~moffitt/docs/6502.html#BOOT>
    this->SR |= (1 << BIT_I);                       // ; disable interrupts
    this->PC  =  this->mem[Layout::RESET_VECTOR];   // ; put PC at where reset vector pointed
    this->PC |= (this->mem[Layout::RESET_VECTOR + 1]) << 8;
    
    // TODO Do this through a reset routine
    this->SP  = Layout::STACK_PAGE_HIGH;            // ; init stack pointer to ceiling of stack page ()
    this->SR &= ~(1 << BIT_I);                      // ; enable interrupts again
}

void Cpu::irq() 
{
    //printf("IRQ signal\n");
    this->PC  =  this->mem[Layout::IRQ_BRK_VECTOR];
    this->PC |= (this->mem[Layout::IRQ_BRK_VECTOR + 1]) << 8;
}

void Cpu::nmi()
{
    //printf("NMI signal\n");
    this->PC  =  this->mem[Layout::NMI_VECTOR];
    this->PC |= (this->mem[Layout::NMI_VECTOR + 1]) << 8;
}
