#include "mouse.h"

#include <iostream>
#include <cstdlib>
#include <time.h>

Mouse::Mouse(Cpu *cpu, Memory *mem, u16 io_port_address) 
{
    this->cpu = cpu;
    this->mem = mem;
    this->io_port_address = io_port_address;
}

void Mouse::poll(double xpos, double ypos)
{
    static double last_x = 0;
    static double last_y = 0;

    double delta_x = xpos - last_x; // compensate for aspect ratio
    double delta_y = ypos - last_y;

    last_x = xpos;
    last_y = ypos;

    s8 x_mv_aggregate = (*this->mem)[this->io_port_address + 1];
    s8 y_mv_aggregate = (*this->mem)[this->io_port_address + 2];

    //x_mv_aggregate += (s8) (3.2*delta_x); // compensate for aspect ratio
    x_mv_aggregate += (s8) delta_x;
    y_mv_aggregate += (s8) delta_y;

    (*this->mem)[this->io_port_address + 1] = x_mv_aggregate;
    (*this->mem)[this->io_port_address + 2] = y_mv_aggregate;

}
