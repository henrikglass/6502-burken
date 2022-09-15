#include "memory.h"

#include <iostream>
#include <string.h>

Memory::Memory()
{
    data = new u8[Layout::MEM_SIZE];
    memset(data, 0, Layout::MEM_SIZE * sizeof(u8)); // Not really neccessary.
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
 * Loads file into memory at specified offset `dest`.
 */
int Memory::load_from_file(u16 dest, const std::string &path)
{
    FILE *f = fopen(path.c_str(), "r");

    if(f == nullptr) {
        printf("Error opening file <%s>", path.c_str());
        return -1;
    }

    u32 max_n_bytes = Layout::MEM_SIZE - dest; 
    u32 n_read = fread(&(this->data[dest]), sizeof(u8), max_n_bytes, f);
    fclose(f);
    
    printf("%d bytes read from <%s>\n", n_read, path.c_str());

    return 0;
}
