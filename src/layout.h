#ifndef LAYOUT_H
#define LAYOUT_H

#include "typedefs.h"

const u32 MEM_SIZE = 0x10000;

const u16 ZERO_PAGE_LOW      = 0x0000;
const u16 ZERO_PAGE_HIGH     = 0x00FF;
const u16 STACK_PAGE_LOW     = 0x0100;
const u16 STACK_PAGE_HIGH    = 0x01FF;
const u16 DISPLAY_DATA_LOW   = 0x0200;
const u16 DISPLAY_DATA_HIGH  = 0x03FF;
const u16 DISPLAY_TILES_LOW  = 0x0400;
const u16 DISPLAY_TILES_HIGH = 0x04FF;
const u16 IO_PAGE_LOW        = 0x0500;
const u16 TIMER_CTRL         = 0x0500;
const u16 TIMER_DATA         = 0x0501;
const u16 IO_PAGE_HIGH       = 0x05FF;
const u16 FREE_RAM_LOW       = 0x0600;
const u16 FREE_RAM_HIGH      = 0x3FFF;
const u16 IO_MEM_LOW         = 0x4000;
const u16 IO_MEM_HIGH        = 0x7FFF;
const u16 FREE_ROM_LOW       = 0x8000;
const u16 FREE_ROM_HIGH      = 0xFFF9;
const u16 NMI_VECTOR         = 0xFFFA; 
const u16 RESET_VECTOR       = 0xFFFC; // TODO implement at 0xFCE2;
const u16 IRQ_BRK_VECTOR     = 0xFFFE;

#endif
