#include <cstdio>
#include <cstdlib>
#include <libnin64/Bus.h>
#include <libnin64/Cart.h>
#include <libnin64/MIPSInterface.h>
#include <libnin64/Memory.h>
#include <libnin64/PeripheralInterface.h>
#include <libnin64/RSP.h>
#include <libnin64/SerialInterface.h>
#include <libnin64/Util.h>
#include <libnin64/VideoInterface.h>

using namespace libnin64;

// https://raw.githubusercontent.com/mikeryan/n64dev/master/docs/n64ops/n64ops%23h.txt
Bus::Bus(Memory& memory, Cart& cart, MIPSInterface& mi, PeripheralInterface& pi, SerialInterface& si, VideoInterface& vi, RSP& rsp)
: _memory{memory}
, _cart{cart}
, _mi{mi}
, _pi{pi}
, _si{si}
, _vi{vi}
, _rsp{rsp}
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
    else if (addr >= 0x04040000 && addr <= 0x0404ffff)
        value = T(_rsp.read(addr));
    else if (addr >= 0x04300000 && addr <= 0x043fffff) //  MIPS Interface (MI) Registers
        value = T(_mi.read(addr));
    else if (addr >= 0x04400000 && addr <= 0x044fffff) // Video Interface (VI) Registers
        value = T(_vi.read(addr));
    else if (addr >= 0x04600000 && addr <= 0x046fffff) // Peripheral Interface (PI) Registers
        value = T(_pi.read(addr));
    else if (addr >= 0x04800000 && addr <= 0x048fffff) // Serial Interface (SI) Registers
        value = T(_si.read(addr));
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
    else if (addr >= 0x04040000 && addr <= 0x0408ffff)
        _rsp.write(addr, (std::uint32_t)value);
    else if (addr >= 0x04300000 && addr <= 0x043fffff) //  MIPS Interface (MI) Registers
        _mi.write(addr, (std::uint32_t)value);
    else if (addr >= 0x04400000 && addr <= 0x044fffff) //  Video Interface (VI) Registers
        _vi.write(addr, (std::uint32_t)value);
    else if (addr >= 0x04600000 && addr <= 0x046fffff) //  Peripheral Interface (PI) Registers
        _pi.write(addr, (std::uint32_t)value);
    else if (addr >= 0x04800000 && addr <= 0x048fffff) //  Serial Interface (SI) Registers
        _si.write(addr, (std::uint32_t)value);
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
