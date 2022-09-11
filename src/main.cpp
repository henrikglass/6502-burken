#include <iostream>
#include <chrono>
#include <thread>
#include <cstring>
#include <signal.h>

#include "memory.h"
#include "m6502.h"
#include "display.h"
#include "timer.h"
#include "util.h"

const char *usage = "Usage: 6502-burken <program>\n";
bool running = true;

void sigint_handler(int signal)
{
    running = false;
}

int main(int argc, char *argv[]) 
{
    if (argc != 2) {
        printf("%s", usage);
        return 1;
    }

    signal(SIGINT, sigint_handler);

    // Create memory
    Memory mem;

    // Create Cpu and provide it with a reference to mem
    Cpu cpu(mem); 

    // Create display and provide references to cpu and memory
    Display display(mem);
    
    // Create an imgui layer and attach it to the OpenGL context of `display`
    ImguiLayerInfo info;
    ImguiLayer imgui_layer(cpu, mem, &info);
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
        mem[i] = (i/2) % 256;
    }
    // ------------------------------------


    // start display
    auto render_thread = display.start();
    render_thread.detach();

    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::nanoseconds;

    // this is easier than naming their types lol
    auto t_start = high_resolution_clock::now();
    auto t_end = high_resolution_clock::now();

    // keep track of remaining ns to sleep from last cycle 
    int64_t sleep_remainder = 0;
    int64_t ns_to_sleep = 0;
    int64_t n_cycles_total = 0;

    // enter infinite fetch (decode) execute loop
    while(running) {
        // handle paused execution
        if (info.execution_paused) {
            printf("freq %f\n", info.execution_speed);
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            if (!info.step_execution)
                continue;
            info.step_execution = false;
        }

        // handle manual (imgui) reset
        if (info.reset_cpu) {
            cpu.reset();
            info.reset_cpu = false;
        }

        // run simulation
        u8 elapsed_cycles = cpu.fetch_execute_next();
        timer1.step((u16) elapsed_cycles);
        timer2.step((u16) elapsed_cycles);
        
        // sleep for however long we need to sleep
        t_end = high_resolution_clock::now();
        auto duration = duration_cast<nanoseconds>(t_end - t_start);
        t_start = high_resolution_clock::now();
        ns_to_sleep = elapsed_cycles * (1000000000.0f / info.execution_speed);
        ns_to_sleep -= duration.count(); // subtract the actual CPU time
        ns_to_sleep += sleep_remainder;  // add the sleep remainder
        sleep_remainder = Util::precise_sleep(ns_to_sleep);
        n_cycles_total += elapsed_cycles;
        
        //if (sleep_remainder < 0)
        //    printf("%ld\n", sleep_remainder);

        // (PRECISELY) sleep to match selected clock speed
        // TODO don't do this
        //std::this_thread::sleep_for(std::chrono::milliseconds(elapsed_cycles*20));
    }

    // DEBUG
    printf("ns_to_sleep = %ld\n", ns_to_sleep);
    printf("sleep_remainder = %ld\n", sleep_remainder);
    printf("n_cycles_total = %ld\n", n_cycles_total);

}
