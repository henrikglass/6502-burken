#ifndef MOUSE_H
#define MOUSE_H

#include "memory.h"
#include "m6502.h"
#include "typedefs.h"

class Mouse
{
public:
    // references to cpu & memory. Cpu is only used for access to irq & nmi "pins".
    Memory *mem;
    Cpu *cpu;

    // access to keyboard registers
    u16 io_port_address; // The mouse device has three single byte registers:
                         // One general purpose register and two registers
                         // containing information about how the mouse has moved
                         // since last being polled.
                         //
                         // the general purpose register (offset 0) looks like this:
                         //
                         //     bit 0: Indicates the left mouse button is pressed
                         //     bit 1: Indicates the right mouse button is pressed
                         //     bit 7: The write/ack bit.
                         //
                         // Like the keyboard device, the write/ack bit is set to 1
                         // when a key is pressed. In the ISR, the cpu should 
                         // acknowledge this by writing a 0 to the write/ack bit.
                         //
                         // The move registers (offsets 1 & 2) are contain signed
                         // 8 bit values indicating the aggregate movement of the 
                         // mouse in the x- and y-axes respectibely. These registers
                         // should be continuously polled by the 6502 application
                         // and manually set to 0 to avoid overflows.

public:
    Mouse(Cpu *cpu, Memory *mem, u16 io_port_address);
    void poll(double xpos, double ypos);
};

#endif
