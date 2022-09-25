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
    (*this->mem)[this->io_port_address] = 0x80 | (key_code & 0x7f);
    std::cout << "keypress " << (u32)key_code << std::endl;
    std::cout << "keypress mods" << (u32)mods << std::endl;
    //this->cpu->irq();
}
