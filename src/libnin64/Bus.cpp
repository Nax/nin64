#include <cstdio>
#include <cstdlib>
#include <libnin64/Bus.h>
#include <libnin64/Cart.h>
#include <libnin64/Memory.h>
#include <libnin64/PeripheralInterface.h>
#include <libnin64/Util.h>

using namespace libnin64;

// https://raw.githubusercontent.com/mikeryan/n64dev/master/docs/n64ops/n64ops%23h.txt

// if (addr <= 0x03EFFFFF) //  RDRAM Memory
// {
// }
// else if (addr <= 0x03FFFFFF) //  RDRAM Registers
// {
// }
// else if (addr <= 0x040FFFFF) //  SP Registers
// {
// }
// else if (addr <= 0x041FFFFF) //  DP Command Registers
// {
// }
// else if (addr <= 0x042FFFFF) //  DP Span Registers
// {
// }
// else if (addr <= 0x043FFFFF) //  MIPS Interface (MI) Registers
// {
// }
// else if (addr <= 0x044FFFFF) //  Video Interface (VI) Registers
// {
// }
// else if (addr <= 0x045FFFFF) //  Audio Interface (AI) Registers
// {
// }
// else if (addr <= 0x046FFFFF) //  Peripheral Interface (PI) Registers
// {
// }
// else if (addr <= 0x047FFFFF) //  RDRAM Interface (RI) Registers
// {
// }
// else if (addr <= 0x048FFFFF) //  Serial Interface (SI) Registers
// {
// }
// else if (addr <= 0x04FFFFFF) //  Unused
// {
// }
// else if (addr <= 0x05FFFFFF) //  Cartridge Domain 2 Address 1
// {
// }
// else if (addr <= 0x07FFFFFF) //  Cartridge Domain 1 Address 1
// {
// }
// else if (addr <= 0x0FFFFFFF) //  Cartridge Domain 2 Address 2
// {
// }
// else if (addr <= 0x1FBFFFFF) //  Cartridge Domain 1 Address 2
// {
// }
// else if (addr <= 0x1FC007BF) //  PIF Boot ROM
// {
// }
// else if (addr <= 0x1FC007FF) //  PIF RAM
// {
// }
// else if (addr <= 0x1FCFFFFF) //  Reserved
// {
// }
// else if (addr <= 0x7FFFFFFF) //  Cartridge Domain 1 Address 3
// {
// }
// else if (addr <= 0xFFFFFFFF) //  External SysAD Device
// {
// }

Bus::Bus(Memory& memory, Cart& cart, PeripheralInterface& pi)
: _memory{memory}
, _cart{cart}
, _pi{pi}
{
}

template <typename T> T Bus::read(std::uint32_t addr)
{
    T value;

    addr &= 0x1fffffff;

    if (addr < 0x03f00000)
    {
        value = swap(*(T*)(_memory.ram + addr));
    }
    else if (addr < 0x03ffffff)
        value = 0;
    else if (addr < 0x04001000)
        value = swap(*(T*)(_memory.spDmem + (addr & 0xfff)));
    else if (addr < 0x04002000)
        value = swap(*(T*)(_memory.spImem + (addr & 0xfff)));
    else if (addr >= 0x04600000 && addr <= 0x046fffff) // Peripheral Interface (PI) Registers
        value = T(_pi.read(addr));
    else if (addr >= 0x10000000 && addr <= 0x1fbfffff) // Cart Domain 1 Address 2
    {
        _cart.read((std::uint8_t*)&value, addr - 0x10000000, sizeof(T));
        value = swap(value);
    }
    else
    {
        value = 0;
        std::printf("WARN: Read: Accessing not mapped memory zone: 0x%08x.\n", addr);
    }

    return value;
}

template <typename T> void Bus::write(std::uint32_t addr, T value)
{
    addr &= 0x1fffffff;

    if (addr < 0x03f00000)
        *(T*)(_memory.ram + addr) = swap(value);
    else if (addr < 0x03ffffff)
        return;
    else if (addr < 0x04001000)
        *(T*)(_memory.spDmem + (addr & 0xfff)) = swap(value);
    else if (addr < 0x04002000)
        *(T*)(_memory.spImem + (addr & 0xfff)) = swap(value);
    else if (addr >= 0x04600000 && addr <= 0x046fffff) //  Peripheral Interface (PI) Registers
        _pi.write(addr, (std::uint32_t)value);
    else
    {
        std::printf("WARN: Write: Accessing not mapped memory zone: 0x%08x.\n", addr);
        return;
    }
}

template std::uint8_t  Bus::read<std::uint8_t>(std::uint32_t);
template std::uint16_t Bus::read<std::uint16_t>(std::uint32_t);
template std::uint32_t Bus::read<std::uint32_t>(std::uint32_t);
template std::uint64_t Bus::read<std::uint64_t>(std::uint32_t);

template void Bus::write<std::uint8_t>(std::uint32_t, std::uint8_t);
template void Bus::write<std::uint16_t>(std::uint32_t, std::uint16_t);
template void Bus::write<std::uint32_t>(std::uint32_t, std::uint32_t);
template void Bus::write<std::uint64_t>(std::uint32_t, std::uint64_t);
