#ifndef INCLUDED_RSP_H
#define INCLUDED_RSP_H

#include <cstddef>
#include <cstdint>
#include <libnin64/NonCopyable.h>

namespace libnin64
{

class Memory;
class MIPSInterface;
class RSP : private NonCopyable
{
public:
    RSP(Memory& memory, MIPSInterface& mi);
    ~RSP();

    std::uint32_t read(std::uint32_t reg);
    void          write(std::uint32_t reg, std::uint32_t value);

private:
    void dmaRead(std::uint16_t length, std::uint16_t count, std::uint16_t skip);
    void dmaWrite(std::uint16_t length, std::uint16_t count, std::uint16_t skip);

    Memory&        _memory;
    MIPSInterface& _mi;

    bool          _halt : 1;
    bool          _semaphore : 1;
    std::uint16_t _spAddr;
    std::uint32_t _dramAddr;
    std::uint16_t _pc;
    std::uint16_t _pcNext;
};

} // namespace libnin64

#endif
