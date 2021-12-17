#ifndef MEMORY_H
#define MEMORY_H

#include "typedefs.h"

#include <string>

const u32 MEM_SIZE = 0x10000;

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
    
    u8 *data;

    Memory();
    ~Memory();

    u8  operator[](u16 address) const; 
    u8 &operator[](u16 address);

    /*
     * Loads memory from file at specified offset.
     */
    int load_from_file(const std::string &path, u16 offset);
};

#endif
