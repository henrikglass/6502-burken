#ifndef MEMORY_H
#define MEMORY_H

#include <iostream>

/*
 * Layout proposed by Rockwell and implemented by most: 
 * <https://www.csh.rit.edu/~moffitt/docs/6502.html#MEM_MAP>
 */
const u16 ZERO_PAGE_LOW   = 0x0000;
const u16 ZERO_PAGE_HIGH  = 0x00FF;
const u16 STACK_PAGE_LOW  = 0x0100;
const u16 STACK_PAGE_HIGH = 0x01FF;
const u16 FREE_RAM_LOW    = 0x0200;
const u16 FREE_RAM_HIGH   = 0x3FFF;
const u16 IO_MEM_LOW      = 0x4000;
const u16 IO_MEM_HIGH     = 0x7FFF;
const u16 FREE_ROM_LOW    = 0x8000;
const u16 FREE_ROM_HIGH   = 0xFFF9;
const u16 NMI_VECTOR      = 0xFFFA; 
const u16 RESET_VECTOR    = 0xFFFC; // TODO implement at 0xFCE2;
const u16 IRQ_BRK_VECTOR  = 0xFFFE;

/*
 * A general purpose 64 KiB memory.
 */
struct Memory 
{
    
    Memory()
    {
        printf("MEM init.\n");
        data = new u8[0xFFFF + 1];
        memset(data, 0, (0xFFFF + 1) * sizeof(u8)); // Not really neccessary.
        data[RESET_VECTOR]     =  FREE_ROM_LOW & 0x00FF; // TODO temporary (start at 0x8000)
        data[RESET_VECTOR + 1] = (FREE_ROM_LOW & 0xFF00) >> 8; 
    }

    ~Memory()
    {
        delete[] data;
    }

    u8 *data;

    u8  operator[](u16 address) const 
    {
        return data[address];
    }

    u8 &operator[](u16 address) 
    {
        return data[address];
    }

    /*
     * Loads memory from file at specified offset.
     */
    void load_from_file(char *path, u16 offset);
};

#endif
