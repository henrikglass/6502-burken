#include <iostream>
#include <chrono>
#include <thread>

#include "m6502.h"


int main() 
{

    Cpu cpu;

    if (cpu.mem.load_from_file("programs/a.out", FREE_ROM_LOW) != 0)
        return 1;

    cpu.reset();

    // print memory contents
    for(int i = 0x8000; i < 0x801A; i++) {
        printf("0x%04X    0x%02X -- %s\n", i, cpu.mem[i], instruction_table[cpu.mem[i]].mnemonic.c_str());
    }
    printf("\n\n\n");
    for(int i = 0xfff0; i < 0xffff; i++) {
        printf("0x%04X    0x%02X -- %s\n", i, cpu.mem[i], instruction_table[cpu.mem[i]].mnemonic.c_str());
    }
    printf("\n\n\n");

    // enter infinite fetch (decode) execute loop
    for(;;) {
        u8 cycles = cpu.fetch_execute_next();
        std::this_thread::sleep_for(std::chrono::milliseconds(cycles*5));
    }

}
