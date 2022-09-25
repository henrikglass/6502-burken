#ifndef KEYBOARD_H
#define KEYBOARD_H

#include "memory.h"
#include "m6502.h"
#include "typedefs.h"

namespace Keycodes
{

};

class Keyboard
{
public:
    // references to cpu & memory
    Memory *mem;
    Cpu *cpu;

    // access to keyboard registers
    u16 io_port_address; // The keyboard io register emulates a simple 8 bit latch
                         // that can be written to by both the keyboard and the cpu:
                         //
                         //     bit 0-6: The Keycode corresponding to a key press.
                         //     bit 7: The write/ack bit. 
                         //
                         // The keyboard sets the write/ack bit to 1 when a key is pressed. 
                         // The keyboard then generates an interrupt. In the ISR, the cpu 
                         // acknowledges the keypress has been received by setting the write 
                         // bit to 0.
public:
    Keyboard(Cpu *cpu, Memory *mem, u16 io_port_address);
    void press(int key_code, int mods);
};

#endif
