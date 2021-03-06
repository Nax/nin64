#ifndef INCLUDED_PERIPHERAL_INTERFACE_H
#define INCLUDED_PERIPHERAL_INTERFACE_H

#include <cstddef>
#include <cstdint>
#include <libnin64/NonCopyable.h>

namespace libnin64
{

class Cart;
class Memory;
class MIPSInterface;
class PeripheralInterface : private NonCopyable
{
public:
    PeripheralInterface(MIPSInterface& mi, Memory& memory, Cart& cart);
    ~PeripheralInterface();

    std::uint32_t read(std::uint32_t reg);
    void          write(std::uint32_t reg, std::uint32_t value);

private:
    MIPSInterface& _mi;
    Memory&        _memory;
    Cart&          _cart;

    std::uint32_t _dramAddr;
    std::uint32_t _cartAddr;
};

} // namespace libnin64

#endif
