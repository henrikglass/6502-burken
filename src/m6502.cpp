#include <iostream>

#include "m6502.h" 

Cpu::Cpu() {
    populate_instruction_table();
    this->reset();
}

void Cpu::reset() 
{
    printf("CPU reset.\n");
    // Done at startupt: <https://www.csh.rit.edu/~moffitt/docs/6502.html#BOOT>
    this->SR |= (1 << BIT_I);                       // ; disable interrupts
    this->PC  =  this->mem[RESET_VECTOR];           // ; put PC at where reset vector pointed
    this->PC |= (this->mem[RESET_VECTOR + 1]) << 8;
    
    // TODO Do this through a reset routine
    this->SP  = STACK_PAGE_HIGH;                    // ; init stack pointer to ceiling of stack page ()
    this->SR &= ~(1 << BIT_I);                      // ; enable interrupts again
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

void Cpu::print_status() 
{
    printf("ACC: 0x%02X\tX: 0x%02X\t\tY: 0x%02X\n", this->ACC, this->X, this->Y);
    printf("SP: %04X\n", this->SP);
    printf("PC: %04X\n", this->PC);
    printf("CARRY (C):\t%d\tZERO (Z):\t%d\tIRQB (I):\t%d\tDECIMAL (D):\t%d\n", 
            (bool)(this->SR & (1 << BIT_C)),
            (bool)(this->SR & (1 << BIT_Z)),
            (bool)(this->SR & (1 << BIT_I)),
            (bool)(this->SR & (1 << BIT_D))
    );
    printf("BRK (B):\t%d\tUNUSED:\t\t%d\tOVERFLOW (V):\t%d\tNEGATIVE (N):\t%d\n", 
            (bool)(this->SR & (1 << BIT_B)),
            (bool)(this->SR & (1 << BIT_UNUSED)),
            (bool)(this->SR & (1 << BIT_V)),
            (bool)(this->SR & (1 << BIT_N))
    );
    printf("\nOP: %s\n\n", instruction_table[this->mem[this->PC]].mnemonic.c_str());
}
