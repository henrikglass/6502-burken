#include <iostream>
#include <chrono>
#include <thread>

#include "layout.h"
#include "m6502.h"
#include "display.h"

int main() 
{

    Memory mem;                 // Create memory
    Cpu cpu(mem);               // Create Cpu and provide it with a reference to mem
    Display display(cpu, mem);  // Create display and provide references to cpu and memory

    // load program into memory
    if (mem.load_from_file("programs/a.out", FREE_ROM_LOW) != 0)
        return 1;
    
    // reset 
    cpu.reset();

    // print memory contents
    for(int i = 0x8000; i < 0x801A; i++) {
        printf("0x%04X    0x%02X -- %s\n", i, cpu.mem[i], instruction_table[cpu.mem[i]].mnemonic.c_str());
    }
    printf("\n");
    for(int i = 0xfff0; i < 0xffff; i++) {
        printf("0x%04X    0x%02X -- %s\n", i, cpu.mem[i], instruction_table[cpu.mem[i]].mnemonic.c_str());
    }
    printf("\n\n\n");

    // enter infinite fetch (decode) execute loop
    bool should_exit = false;
    while(!should_exit) {
        u8 cycles = cpu.fetch_execute_next();
        std::this_thread::sleep_for(std::chrono::milliseconds(cycles*20));
        should_exit = display.update();
    }

}
