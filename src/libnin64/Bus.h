#ifndef INCLUDED_BUS_H
#define INCLUDED_BUS_H

#include <cstdint>
#include <libnin64/NonCopyable.h>

namespace libnin64
{

class Memory;
class Cart;
class MIPSInterface;
class PeripheralInterface;
class Bus : private NonCopyable
{
public:
    Bus(Memory& memory, Cart& cart, MIPSInterface& mi, PeripheralInterface& pi);

    template <typename T> T    read(std::uint32_t addr);
    template <typename T> void write(std::uint32_t addr, T value);

    auto read8(std::uint32_t addr) { return read<std::uint8_t>(addr); }
    auto read16(std::uint32_t addr) { return read<std::uint16_t>(addr); }
    auto read32(std::uint32_t addr) { return read<std::uint32_t>(addr); }
    auto read64(std::uint32_t addr) { return read<std::uint64_t>(addr); }

    void write8(std::uint32_t addr, std::uint8_t value) { write<std::uint8_t>(addr, value); }
    void write16(std::uint32_t addr, std::uint16_t value) { write<std::uint16_t>(addr, value); }
    void write32(std::uint32_t addr, std::uint32_t value) { write<std::uint32_t>(addr, value); }
    void write64(std::uint32_t addr, std::uint64_t value) { write<std::uint64_t>(addr, value); }

private:
    Memory&              _memory;
    Cart&                _cart;
    MIPSInterface&       _mi;
    PeripheralInterface& _pi;
};

} // namespace libnin64

#endif
