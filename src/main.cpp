#include <iostream>
#include <chrono>
#include <thread>

#include "memory.h"
#include "m6502.h"
#include "display.h"
#include "timer.h"

const char *usage = "Usage: 6502-burken <program>\n";

int main(int argc, char *argv[]) 
{
    if (argc != 2) {
        printf("%s", usage);
        return 1;
    }

    // Create memory
    Memory mem;

    // Create Cpu and provide it with a reference to mem
    Cpu cpu(mem); 

    // Create display and provide references to cpu and memory
    Display display(mem);
    
    // Create an imgui layer and attach it to the OpenGL context of `display`
    ImguiLayer imgui_layer(cpu, mem);
    display.attach_imgui_layer(&imgui_layer);

    // Create timers and provide references to cpu and memory
    Timer timer1(cpu, mem, Layout::TIMER1_CTRL, Layout::TIMER1_DATA);
    Timer timer2(cpu, mem, Layout::TIMER2_CTRL, Layout::TIMER2_DATA);

    // load program into memory
    if (mem.load_from_file(Layout::FREE_ROM_LOW, argv[1]) != 0)
        return 1;
    
    // load VGA charset into memory
    if (mem.load_from_file(Layout::VGA_CHAR_BUF_LOW, "extra/6502burken_charset.bin") != 0)
        return 1;
    
    // reset cpu (starts executing at wherever the reset vector is pointing) 
    cpu.reset();


    // -------------- DEBUG ---------------
    // put garbage in vga_text_buffer
    for (int i = Layout::VGA_TEXT_BUF_LOW; i < Layout::VGA_TEXT_BUF_HIGH; i++) {
        mem[i] = i % 256;
    }
    // ------------------------------------


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
