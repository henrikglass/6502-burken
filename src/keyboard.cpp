#include "keyboard.h"

#include <iostream>
    
Keyboard::Keyboard(Cpu *cpu, Memory *mem, u16 io_port_address)
{
    this->cpu = cpu;
    this->mem = mem;
    this->io_port_address = io_port_address;
}

void Keyboard::press(int key_code, int mods)
{
    //std::cout << key_code << " " << mods << std::endl;

    // translate keycode + mods combination to 6502-burken keycode byte
    //
    // TODO implement as map instead of logic
    u8 key_byte;
    if      (key_code >= 48 && key_code <= 57)              key_byte = key_code;        // digits 0-9
    else if (key_code >= 65 && key_code <= 90 && mods == 0) key_byte = key_code + 0x20; // characters a-z
    else if (key_code >= 65 && key_code <= 90 && mods == 1) key_byte = key_code;        // characters A-Z
    else if (key_code == 32)  key_byte = 0x00;                                          // space
    else if (key_code == 259) key_byte = 0x01;                                          // backspace
    else if (key_code == 257) key_byte = 0x02;                                          // enter
    else if (key_code == 265) key_byte = 0x03;                                          // up
    else if (key_code == 264) key_byte = 0x04;                                          // down
    else if (key_code == 263) key_byte = 0x05;                                          // left
    else if (key_code == 262) key_byte = 0x06;                                          // right
    else return;    

    // write byte with write/ack bit set
    std::cout << this->io_port_address << std::endl;
    (*this->mem)[this->io_port_address] = 0x80 | (key_byte & 0x7f);

    // generate an interrupt
    this->cpu->irq();
}
