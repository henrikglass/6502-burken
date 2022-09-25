#ifndef MEMORY_H
#define MEMORY_H

#include "typedefs.h"

#include <string>

namespace Layout
{
    const u16 ZERO_PAGE_LOW      = 0x0000;
    const u16 ZERO_PAGE_HIGH     = 0x00FF;
    const u16 STACK_PAGE_LOW     = 0x0100;
    const u16 STACK_PAGE_HIGH    = 0x01FF;
    const u16 VGA_TEXT_BUF_LOW   = 0x0200;
    const u16 VGA_TEXT_BUF_HIGH  = 0x119F;
    const u16 VGA_COLOR_BUF_LOW  = 0x11A0;
    const u16 VGA_COLOR_BUF_HIGH = 0x11CF;
    const u16 VGA_CHAR_BUF_LOW   = 0x1200;
    const u16 VGA_CHAR_BUF_HIGH  = 0x15FF;
    const u16 IO_PAGE_LOW        = 0x1600;
    const u16 TIMER1_CTRL        = 0x1600; // 1 Byte
    const u16 TIMER1_DATA        = 0x1601; // 2 Bytes
    const u16 TIMER2_CTRL        = 0x1603; // 1 Byte
    const u16 TIMER2_DATA        = 0x1604; // 2 Bytes
    const u16 VGA_CTRL           = 0x1606; // 1 Bytes
    const u16 IO_PAGE_HIGH       = 0x16FF;
    const u16 FREE_RAM_LOW       = 0x1700;
    const u16 FREE_RAM_HIGH      = 0x3FFF;
    const u16 IO_MEM_LOW         = 0x4000;
    const u16 IO_MEM_HIGH        = 0x7FFF;
    const u16 FREE_ROM_LOW       = 0x8000; // Note: not actually protected from writes in any way.
    const u16 FREE_ROM_HIGH      = 0xFFF9;
    const u16 NMI_VECTOR         = 0xFFFA; // 2 bytes
    const u16 RESET_VECTOR       = 0xFFFC; // 2 bytes
    const u16 IRQ_BRK_VECTOR     = 0xFFFE; // 2 bytes

    const u16 N_PAGES            = 0xFF;
    const u32 PAGE_SIZE          = 0x100;
    const u32 MEM_SIZE           = 0x10000;
    const u16 VGA_CHAR_BUF_SIZE  = VGA_CHAR_BUF_HIGH - VGA_CHAR_BUF_LOW + 1;
    const u16 VGA_TEXT_BUF_SIZE  = VGA_TEXT_BUF_HIGH - VGA_TEXT_BUF_LOW + 1;
    const u16 VGA_COLOR_BUF_SIZE = VGA_COLOR_BUF_HIGH - VGA_COLOR_BUF_LOW + 1;
}

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
    int load_from_file(u16 dest, const std::string &path);
};

#endif
