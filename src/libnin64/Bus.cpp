#include <libnin64/Bus.h>
#include <libnin64/Memory.h>
#include <libnin64/Util.h>

using namespace libnin64;

Bus::Bus(Memory& memory)
: _memory{memory}
{

}

template <typename T> T Bus::read(std::uint32_t addr)
{
    T value;

    addr &= 0x3fffffff;

    if (addr < 0x03f00000)
        value = *(T*)(_memory.ram + addr);
    else if (addr < 0x3fffffff)
        value = 0;
    else if (addr < 0x40001000)
        value = *(T*)(_memory.spDmem + (addr & 0xfff));
    else if (addr < 0x40002000)
        value = *(T*)(_memory.spImem + (addr & 0xfff));
    else
        value = 0;

    return swap(value);
}

template <typename T> void Bus::write(std::uint32_t addr, T value)
{
    addr &= 0x3fffffff;
    value = swap(value);

    if (addr < 0x03f00000)
        *(T*)(_memory.ram + addr) = value;
    else if (addr < 0x3fffffff)
        return;
    else if (addr < 0x40001000)
        *(T*)(_memory.spDmem + (addr & 0xfff)) = value;
    else if (addr < 0x40002000)
        *(T*)(_memory.spImem + (addr & 0xfff)) = value;
    else
        return;
}
