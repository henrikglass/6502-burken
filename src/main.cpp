#include <iostream>
#include <chrono>
#include <thread>

#include "m6502.h"


int main() 
{

    Cpu cpu;
    cpu.reset();

    cpu.mem[0x8000] = Instruction::NOP;
    cpu.mem[0x8001] = Instruction::LDA_I;
    cpu.mem[0x8002] = 0xFA;
    cpu.mem[0x8003] = Instruction::NOP;
    cpu.mem[0x8004] = Instruction::LDA_I;
    cpu.mem[0x8005] = 0xFB;
    cpu.mem[0x8006] = Instruction::JMP_ABS;
    cpu.mem[0x8007] = 0x00; // low
    cpu.mem[0x8008] = 0x80; // high
    cpu.mem[0x8009] = Instruction::NOP;

    // print memory contents
    for(int i = 0x8000; i < 0x801A; i++) {
        printf("%02X ", cpu.mem[i]);
    }
    printf("\n\n\n");

    for(;;) {
        cpu.print_status();
        u8 cycles = cpu.fetch_execute_next();
        std::this_thread::sleep_for(std::chrono::milliseconds(cycles*50));
    }

}
