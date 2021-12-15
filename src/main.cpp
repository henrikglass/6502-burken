#include <iostream>
#include <chrono>
#include <thread>

#include "m6502.h"


int main() 
{

    Cpu cpu;
    cpu.reset();

    cpu.mem[0x8000] = TTInstruction::NOP;
    cpu.mem[0x8001] = TTInstruction::LDA_I;
    cpu.mem[0x8002] = 0xFA;
    cpu.mem[0x8003] = TTInstruction::NOP;
    cpu.mem[0x8004] = TTInstruction::NOP;
    cpu.mem[0x8005] = 0x69; // ADC #imm
    cpu.mem[0x8006] = 25;    // 5
    cpu.mem[0x8007] = TTInstruction::NOP; //0x13;
    cpu.mem[0x8008] = 0x18;  // CLC;
    cpu.mem[0x8009] = TTInstruction::LDA_I;
    cpu.mem[0x800a] = 0xFB;
    cpu.mem[0x800b] = TTInstruction::JMP_ABS;
    cpu.mem[0x800c] = 0x00; // low
    cpu.mem[0x800d] = 0x80; // high
    cpu.mem[0x800e] = TTInstruction::NOP;

    // print memory contents
    for(int i = 0x8000; i < 0x801A; i++) {
        printf("0x%04X    0x%02X -- %s\n", i, cpu.mem[i], instruction_table[cpu.mem[i]].mnemonic.c_str());
    }
    printf("\n\n\n");

    // enter infinite fetch (decode) execute loop
    for(;;) {
        u8 cycles = cpu.fetch_execute_next();
        std::this_thread::sleep_for(std::chrono::milliseconds(cycles*50));
    }

}
