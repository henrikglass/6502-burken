#include "memory.h"

#include <iostream>
#include <string.h>

Memory::Memory()
{
    data = new u8[MEM_SIZE];
    memset(data, 0, MEM_SIZE * sizeof(u8)); // Not really neccessary.
    //data[RESET_VECTOR]     =  FREE_ROM_LOW & 0x00FF; // by default entry point is at 0x8000
    //data[RESET_VECTOR + 1] = (FREE_ROM_LOW & 0xFF00) >> 8; 
}

Memory::~Memory()
{
    delete[] data;
}

u8 Memory::operator[](u16 address) const 
{
    return data[address];
}

u8 &Memory::operator[](u16 address) 
{
    return data[address];
}

/*
 * Loads memory from file at specified offset.
 */
int Memory::load_from_file(const std::string &path, u16 offset)
{
    FILE *f = fopen(path.c_str(), "r");

    if(f == nullptr) {
        printf("Error opening file <%s>", path.c_str());
        return 1;
    }

    u32 max_n_bytes = MEM_SIZE - offset; 
    u32 n_read = fread(&(this->data[offset]), sizeof(u8), max_n_bytes, f);
    fclose(f);
    
    printf("%d bytes read from <%s>\n", n_read, path.c_str());

    return 0;
}
