#ifndef INCLUDED_RDP_H
#define INCLUDED_RDP_H

#include <cstddef>
#include <cstdint>
#include <libnin64/NonCopyable.h>

#define DPC_START_REG        0x04100000
#define DPC_END_REG          0x04100004
#define DPC_CURRENT_REG      0x04100008
#define DPC_STATUS_REG       0x0410000c
#define DPC_CLOCK_REG        0x04100010
#define DPC_BUFBUSY_REG      0x04100014
#define DPC_PIPEBUSY_REG     0x04100018
#define DPC_TMEM_REG         0x0410001c
#define DPS_TBIST_REG        0x04200000
#define DPS_TEST_MODE_REG    0x04200004
#define DPS_BUFTEST_ADDR_REG 0x04200008
#define DPS_BUFTEST_DATA_REG 0x0420000c

namespace libnin64
{

class Memory;
class MIPSInterface;
class RDP : private NonCopyable
{
public:
    RDP(Memory& memory, MIPSInterface& mi);
    ~RDP();

    std::uint32_t read(std::uint32_t reg);
    void          write(std::uint32_t reg, std::uint32_t value);
    void          dma();

private:
    Memory&        _memory;
    MIPSInterface& _mi;

    std::uint32_t _cmdStart;
    std::uint32_t _cmdEnd;
    std::uint32_t _cmdCurrent;
    bool          _xbus : 1;
};

} // namespace libnin64

#endif
