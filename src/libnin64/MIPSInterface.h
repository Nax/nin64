#ifndef INCLUDED_MIPS_INTERFACE_H
#define INCLUDED_MIPS_INTERFACE_H

#include <cstddef>
#include <cstdint>
#include <libnin64/NonCopyable.h>

#define MI_INTR_SP 0x01
#define MI_INTR_SI 0x02
#define MI_INTR_AI 0x04
#define MI_INTR_VI 0x08
#define MI_INTR_PI 0x10
#define MI_INTR_DP 0x20

namespace libnin64
{

class MIPSInterface : private NonCopyable
{
public:
    MIPSInterface();
    ~MIPSInterface();

    std::uint8_t ip() const;

    std::uint32_t read(std::uint32_t reg);
    void          write(std::uint32_t reg, std::uint32_t value);

    void setInterrupt(std::uint8_t intr);
    void clearInterrupt(std::uint8_t intr);

private:
    std::uint8_t _interrupts;
    std::uint8_t _interruptsMask;
};

} // namespace libnin64

#endif
