#ifndef INCLUDED_CPU_H
#define INCLUDED_CPU_H

#include <cstddef>
#include <cstdint>
#include <libnin64/NonCopyable.h>

namespace libnin64
{

class Bus;
class CPU : private NonCopyable
{
public:
    CPU(Bus& bus);
    ~CPU();

    void tick(std::size_t count);
    void tick();

private:
    std::uint32_t   cop0Read(std::uint8_t reg);
    void            cop0Write(std::uint8_t reg, std::uint32_t value);

    union Reg
    {
        std::uint64_t   u64;
        std::int64_t    i64;
        std::uint32_t   u32;
        std::int32_t    i32;
        std::uint16_t   u16;
        std::int16_t    i16;
        std::uint8_t    u8;
        std::int8_t     i8;
    };

    Bus& _bus;

    std::uint64_t   _pc;
    std::uint64_t   _pcNext;
    Reg             _regs[32];
    Reg             _lo;
    Reg             _hi;
    std::uint32_t   _llAddr;
    bool            _llBit:1;
};

}

#endif
