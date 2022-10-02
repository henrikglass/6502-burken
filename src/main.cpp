#ifndef TEST

#include <iostream>
#include <chrono>
#include <thread>
#include <cstring>
#include <signal.h>

#include "memory.h"
#include "m6502.h"
#include "m6502_disassembler.h"
#include "display.h"
#include "timer.h"
#include "util.h"

const char *usage = "Usage: 6502-burken <program>\n";
bool running = true;

struct System
{
    Memory  *mem;
    Cpu     *cpu;
    Timer   *timer1;
    Timer   *timer2;
    Display *display;
};

void sigint_handler(int signal)
{
    running = false;
}

void run_simulation(System *system, UiInfo *info)
{
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;
    using std::chrono::microseconds;
    using std::chrono::nanoseconds;
    
    // this is easier than naming their types lol
    auto t_start = high_resolution_clock::now(); // time at start of fetch decode execute cycle

    // reset cpu (starts executing at wherever the reset vector is pointing) 
    system->cpu->reset();
    
    // start display
    auto render_thread = system->display->start();
    render_thread.detach();

    // Some state related to keeping CPU clock in check
    int64_t ns_to_sleep = 0;
    int64_t last_diff_ns = 0;
    int total_n_cycles = 0;
    int n_cycles_thresh = 10000;
    
    // enter infinite fetch (decode) execute loop
    while(running) {

        if (info->changed) {
            // set a resonable value for `n_cycles_thresh` depending on
            // requested clock speed.
            n_cycles_thresh = info->requested_clock_speed / 60.0f;
        }

        // handle paused execution and breakpoints
        if (info->execution_paused) {
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
            if (!info->step_execution)
                continue;
        }
        if (info->breakpoints_enabled && !info->step_execution) {
            for (u16 bp : info->breakpoints) {
                if (system->cpu->PC == bp) {
                    std::cout << "Hit breakpoint: " << std::hex << bp << std::dec << std::endl;
                    info->execution_paused = true;
                    break;
                }
            }
            if (info->execution_paused)
                continue;
        }
        info->step_execution = false;

        // handle manual (imgui) reset
        if (info->reset_cpu) {
            system->cpu->reset();
            info->reset_cpu = false;
        }

        // run and time this fetch decode execute cycle
        u8 elapsed_cycles = system->cpu->fetch_execute_next();
        system->timer1->step((u16) elapsed_cycles);
        system->timer2->step((u16) elapsed_cycles);
        total_n_cycles += elapsed_cycles;
        
        // If in turbo mode, we execute as fast as possible (quite unlike
        // what the turbo button did on some 90s computers). Don't sleep.
        if (info->turbo_mode)
            continue;

        // sleep for however long we need to sleep to match the clock speed
        // we're supposed to run at.
        ns_to_sleep += elapsed_cycles * (1000000000.0f / info->requested_clock_speed);
        ns_to_sleep = Util::precise_sleep(ns_to_sleep);
        
        // To account for real cpu time usage, we measure the time it takes to run a set number of
        // cycles (say `N`). The difference between the measured time and the expected time is 
        // subtracted from the time that will be slept for the next `N` cycles.
        if (info->requested_clock_speed > 1000.0f &&
                total_n_cycles > n_cycles_thresh) {
            int64_t measured_ns = duration_cast<nanoseconds>(high_resolution_clock::now() - t_start).count();
            int64_t expected_ns = n_cycles_thresh * (1000000000.0f / info->requested_clock_speed);

            // compensate for extra mesaured cycles above `n_cycles_thresh`. We barely lose or gain
            // any precision but why not.
            int64_t extra_cycles = total_n_cycles - n_cycles_thresh;
            measured_ns -= extra_cycles * (1000000000.0f / info->requested_clock_speed);

            // compensate for the time subracted from previous measurements
            expected_ns -= info->changed ? 0 : last_diff_ns;

            // subtract difference from `ns_to_sleep`
            last_diff_ns = (info->changed) ? 0 : (measured_ns - expected_ns);
            ns_to_sleep -= last_diff_ns;                                            

            // reset stuff for next measurement
            total_n_cycles = 0;
            t_start = high_resolution_clock::now();
            //first = false;
            info->changed = false;
           
            // measured clock speed 
            info->measured_clock_speed = (1000000000.0f * n_cycles_thresh) / measured_ns;
        }
        
        
    }

}

int main(int argc, char *argv[]) 
{
    if (argc != 2) {
        printf("%s", usage);
        return 1;
    }

    System system;

    signal(SIGINT, sigint_handler);

    // Create memory
    Memory mem;

    // Create Cpu and provide it with a reference to mem
    Cpu cpu(mem); 

    // Create display and provide references to cpu and memory
    Display display(mem);
    
    // -------------- DEBUG ---------------
    // put garbage in vga_text_buffer
    for (int i = Layout::VGA_TEXT_BUF_LOW; i < Layout::VGA_TEXT_BUF_HIGH; i++) {
        mem[i] = (i/2) % 256;
        if (i % 2 == 1)
            mem[i] = 0x07;
    }

    mem[Layout::VGA_TEXT_BUF_LOW + 4] = 0x7f;
    mem[Layout::VGA_TEXT_BUF_LOW + 5] = 0x87;
    // ------------------------------------

    // create a Disassembler object (used by ImguiLayer).
    Disassembler disassembler(mem);

    // Create an imgui layer and attach it to the OpenGL context of `display`
    UiInfo info;
    ImguiLayer imgui_layer(cpu, mem, &disassembler, &info);
    display.attach_imgui_layer(&imgui_layer);
    
    // Create a keyboard and attach it to the GLFW context of `display`
    Keyboard keyboard(&cpu, &mem, Layout::KEYBOARD_IO_PORT);
    display.attach_keyboard(&keyboard);
    
    // Create a mouse and attach it to the GLFW context of `display`
    Mouse mouse(&cpu, &mem, Layout::MOUSE_IO_PORT);
    display.attach_mouse(&mouse);

    // Create timers and provide references to cpu and memory
    Timer timer1(cpu, mem, Layout::TIMER1_CTRL, Layout::TIMER1_DATA);
    Timer timer2(cpu, mem, Layout::TIMER2_CTRL, Layout::TIMER2_DATA);

    // load program into memory
    if (mem.load_from_file(Layout::FREE_ROM_LOW, argv[1]) < 0)
        return 1;
   
    // load VGA text buffer splash screen into memory
    if (mem.load_from_file(Layout::VGA_TEXT_BUF_LOW, "extra/6502burken_splash_screen_converted.bin") < 0)
        return 1;

    // load VGA color palette into memory
    if (mem.load_from_file(Layout::VGA_COLOR_BUF_LOW, "extra/6502burken_color_lut.bin") < 0)
        return 1;

    // load VGA charset into memory
    if (mem.load_from_file(Layout::VGA_CHAR_BUF_LOW, "extra/6502burken_charset_v2.bin") < 0)
        return 1;
    
    // debug
    if (mem.load_from_file(Layout::VGA_SPRITE_BUF_LOW + 0x00, "extra/mouse_cursor_sprite.bin") < 0)
        return 1;
   
   
    // bundle everything together
    system.cpu     = &cpu;
    system.mem     = &mem;
    system.timer1  = &timer1;
    system.timer2  = &timer2;
    system.display = &display;

    // run simulation
    run_simulation(&system, &info);

}

#endif
