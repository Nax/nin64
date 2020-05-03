#ifndef INCLUDED_MEMORY_H
#define INCLUDED_MEMORY_H

#include <cstdint>
#include <libnin64/NonCopyable.h>

namespace libnin64
{

class Memory : private NonCopyable
{
public:
    Memory();
    ~Memory();

    std::uint8_t ram[8 * 1024 * 1024];
};

}

#endif
