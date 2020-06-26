#ifndef INCLUDED_SERIAL_INTERFACE_H
#define INCLUDED_SERIAL_INTERFACE_H

#include <cstddef>
#include <cstdint>
#include <libnin64/NonCopyable.h>

namespace libnin64
{

class MIPSInterface;
class Memory;
class SerialInterface : private NonCopyable
{
public:
    SerialInterface(MIPSInterface& mi, Memory& memory);
    ~SerialInterface();

    std::uint32_t read(std::uint32_t reg);
    void          write(std::uint32_t reg, std::uint32_t value);
    void          pifUpdate();

private:
    void dmaRead();
    void dmaWrite();

    MIPSInterface& _mi;
    Memory&        _memory;

    std::uint32_t _addr;
};

} // namespace libnin64

#endif
