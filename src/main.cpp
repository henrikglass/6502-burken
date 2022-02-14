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
    
    // load vga text buffer with garbage
    for (int i = Layout::VGA_TEXT_BUF_LOW; i <= Layout::VGA_TEXT_BUF_HIGH; i++) {
        if (i % 2 == 0) {
            mem[i] = (i/2) % 0x80;
        } else if ((i / (5*160)) % 2 == 0) {
            int temp = (i-2) % 160;
            temp /= 10;
            mem[i] = (temp << 4) + (0b0000);
            //mem[i] = 0x0A; // bg color index: 0b0000, fg color index: 0b1111
        } else { // on odd rows swap fg and bg colors
            int temp = (i-2) % 160;
            temp /= 10;
            mem[i] = (0b0000 << 4) + (temp);
        }

    }
    
    // reset cpu 
    cpu.reset();

    // start display
    auto render_thread = display.start();
    //auto render_thread = std::thread(display.enter_render_loop);

    // DEBUG
    // print memory contents
    //for(int i = Layout::VGA_TEXT_BUF_LOW; i <= Layout::VGA_TEXT_BUF_HIGH; i++) {
    //    printf("0x%04X    0x%02X\n", i, cpu.mem[i]);

    //for(int i = Layout::VGA_CHAR_BUF_LOW; i <= Layout::VGA_CHAR_BUF_HIGH; i++) {
    //    printf("0x%04X    0x%02X\n", i, cpu.mem[i]);
    //}
    // print memory contents
    //for(int i = 0x8000; i < 0x802A; i++) {
    //    printf("0x%04X    0x%02X -- %s\n", i, cpu.mem[i], instruction_table[cpu.mem[i]].mnemonic.c_str());
    //}
    //printf("\n");
    //for(int i = 0xfff0; i < 0xffff; i++) {
    //    printf("0x%04X    0x%02X -- %s\n", i, cpu.mem[i], instruction_table[cpu.mem[i]].mnemonic.c_str());
    //}
    //printf("\n\n\n");
    cpu.print_status();

    // enter infinite fetch (decode) execute loop
    while(true) {
        u8 elapsed_cycles = cpu.fetch_execute_next();
        timer1.step((u16) elapsed_cycles);
        std::this_thread::sleep_for(std::chrono::milliseconds(elapsed_cycles*20));
    }

}
