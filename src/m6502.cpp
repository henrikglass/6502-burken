#include <iostream>

#include "m6502.h" 

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
            u8 data = this->mem[this->PC++];
            this->ACC = data;
            return 2;
        }
        case Instruction::JMP_ABS:
        {
            printf("JMP_ABS\n");
            u8 addr_low  = this->mem[this->PC++];
            u8 addr_high = this->mem[this->PC++];
            this->PC  = addr_low;
            this->PC |= addr_high << 8;
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
