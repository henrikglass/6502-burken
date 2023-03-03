#ifndef UTIL_H
#define UTIL_H

#include "typedefs.h"

#include <stdint.h>
#include <string>
#include <sstream>
#include <iomanip>

namespace Util
{
    int64_t precise_sleep(int64_t nanoseconds);

    template<typename T>
    std::string int_to_hex(T i)
    {
      std::stringstream stream;
      stream << "0x"
             << std::setfill ('0') << std::setw(sizeof(T)*2)
             << std::hex << i;
      return stream.str();
    }

    u16 hex_to_u16(const std::string &str);

    u32 crc32(const u8 *data, size_t len);
};

#endif
