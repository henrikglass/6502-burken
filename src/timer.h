#ifndef TIMER_H
#define TIMER_H

#include "memory.h"
#include "m6502.h"
#include "typedefs.h"

/*
 * Implements a timer loosely based on TIMER 1 in an ATmega328P
 * microprocessor. Only supports normal mode, for now.
 */
class Timer 
{
private:
    // references to cpu & memory
    const Memory &mem;
    Cpu &cpu;

    // access to timer registers
    u16 data_address;        // The data register sets the upper limit
                             // for counting. When count hits this value,
                             // it wraps around to 0 and an interrupt 
                             // signal is sent.

    u16 ctrl_address;        // The control register works like this:
                             // bit 0: active
                             // bit 1-3: prescaler (mapping is:  001->1,
                             // 010->8, 011->64, 100->256, 101->1024).
                             // values 0b000, 0b110, and 0b111 are not 
                             // allowed (will cause floating point 
                             // exception)

    // internal state
    u16 count;               // the active count value
    u16 prescaler_clock_sum; // keep track of the number of clock cycles
                             // since last tick of count

public:
    Timer(Cpu &cpu, const Memory &mem, u16 ctrl_address, u16 data_address);

    void step(u16 n_cycles_elapsed);
};

#endif
