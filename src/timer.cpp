#include "timer.h"

const u16 prescaler_map[8] = {
    0,    // 0b000
    1,    // 0b001
    8,    // 0b010
    64,   // 0b011
    256,  // 0b100
    1024, // 0b101
    0,    // 0b110
    0     // 0b111
};

Timer::Timer(Cpu &cpu, const Memory &mem, u16 ctrl_address, u16 data_address) :
    mem(mem), cpu(cpu)
{
    this->ctrl_address = ctrl_address;
    this->data_address = data_address;
    this->count = 0;
}

void Timer::step(u16 n_cycles_elapsed) 
{
    u16 data;
    u16 prescaler;
    u8 ctrl = this->mem[this->ctrl_address];
    
    u8 active = ctrl & 1;
    if(!active)
        return;

    data  = this->mem[this->data_address];
    data |= this->mem[this->data_address + 1] << 8;

    u8 prescaler_bits = (ctrl >> 1) & 0b111;
    prescaler = prescaler_map[prescaler_bits];

    if (prescaler == 1) {
        // Handle prescaler == 1 as a special case not to have to deal with
        // multiple overflows
        this->count += n_cycles_elapsed;
    } else {
        // this assumes no instruction takes more than 8 cycles to complete
        this->prescaler_clock_sum += n_cycles_elapsed;

        if (this->prescaler_clock_sum > prescaler) {
            this->prescaler_clock_sum %= prescaler;            
            this->count++;
        }
    }

    printf("timer>count: %d\n", this->count);
    printf("timer>data: %d\n", data);
    printf("timer>prescaler: %d\n", prescaler);

    if (this->count < data)
        return;

    this->count %= data;
    cpu.irq(); // send interrupt request signal
}
