#ifndef MEMORY_H
#define MEMORY_H

#include "typedefs.h"

#include <string>

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
