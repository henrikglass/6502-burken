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
    this->SR  |=  STATUS_MASK_I;                     // ; disable interrupts
    this->PC   =  this->mem[RESET_VECTOR];           // ; put PC at where reset vector pointed
    this->PC  |= (this->mem[RESET_VECTOR + 1]) << 8;
    
    // TODO Do this through a reset routine
    this->SP  = STACK_PAGE_HIGH;                     // ; init stack pointer to ceiling of stack page ()
    this->SR &= ~STATUS_MASK_I;                      // ; enable interrupts again
}

u8 Cpu::fetch_execute_next() 
{

#ifdef DEBUG_PRINTS
    this->print_status();
#endif

    // fetch
    u8 opcode = this->mem[this->PC++];
    auto instr = instruction_table[opcode];
    
    // execute
    u8 cycles_taken = instr.execute(this);

    // return # of elapsed cpu cycles
    return cycles_taken;
}

void Cpu::print_status() 
{
    printf("ACC: 0x%02X\tX: 0x%02X\t\tY: 0x%02X\n", this->ACC, this->X, this->Y);
    printf("PC: %04X\n", this->PC);
    printf("OP: %s\n", instruction_table[this->mem[this->PC]].mnemonic.c_str());
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
