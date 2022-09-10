#include <iostream>
#include <chrono>
#include <thread>

#include "memory.h"
#include "m6502.h"
#include "display.h"
#include "timer.h"

int main() 
{

    // Create memory
    Memory mem;

    // Create Cpu and provide it with a reference to mem
    Cpu cpu(mem); 

    // Create display and provide references to cpu and memory
    Display display(cpu, mem);
    
    // Create a timer and provide references to cpu and memory
    Timer timer1(cpu, mem, Layout::TIMER1_CTRL, Layout::TIMER1_DATA);

    // load program into memory
    if (mem.load_from_file("programs/a.out", Layout::FREE_ROM_LOW) != 0)
        return 1;
    
    // load VGA charset into memory
    if (mem.load_from_file("extra/6502burken_charset.bin", Layout::VGA_CHAR_BUF_LOW) != 0)
        return 1;
    
    // reset cpu 
    cpu.reset();

    // start display
    auto render_thread = display.start();

    cpu.print_status();

    // enter infinite fetch (decode) execute loop
    while(true) {
        u8 elapsed_cycles = cpu.fetch_execute_next();
        timer1.step((u16) elapsed_cycles);
        std::this_thread::sleep_for(std::chrono::milliseconds(elapsed_cycles*20));
    }

}
