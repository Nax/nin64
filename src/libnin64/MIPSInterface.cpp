#include <cstdio>
#include <libnin64/MIPSInterface.h>

#define MI_INIT_MODE_REG 0x04300000
#define MI_VERSION_REG   0x04300004
#define MI_INTR_REG      0x04300008
#define MI_INTR_MASK_REG 0x0430000c

using namespace libnin64;

MIPSInterface::MIPSInterface()
: _interrupts{}
, _interruptsMask{}
{
}

MIPSInterface::~MIPSInterface()
{
}

std::uint8_t MIPSInterface::ip() const
{
    return (_interrupts & _interruptsMask) ? 0x04 : 0x00;
}

std::uint32_t MIPSInterface::read(std::uint32_t reg)
{
    std::uint32_t value{};

    switch (reg)
    {
    case MI_INIT_MODE_REG:
        break;
    case MI_VERSION_REG:
        value = 0x01010101;
        break;
    case MI_INTR_REG:
        value = _interrupts;
        break;
    case MI_INTR_MASK_REG:
        value = _interruptsMask;
        break;
    }

    return value;
}

void MIPSInterface::write(std::uint32_t reg, std::uint32_t value)
{
    switch (reg)
    {
    case MI_INIT_MODE_REG:
        break;
    case MI_VERSION_REG:
        /* No-op */
        break;
    case MI_INTR_REG:
        /* No-op */
        break;
    case MI_INTR_MASK_REG:
        // No handling of metastable input (https://en.wikibooks.org/wiki/Digital_Circuits/Latches#SR_latch)
        // SP (Clear/Set)
        if (value & (1 << 0)) _interruptsMask &= ~MI_INTR_SP;
        if (value & (1 << 1)) _interruptsMask |= MI_INTR_SP;
        // SI (Clear/Set)
        if (value & (1 << 2)) _interruptsMask &= ~MI_INTR_SI;
        if (value & (1 << 3)) _interruptsMask |= MI_INTR_SI;
        // AI (Clear/Set)
        if (value & (1 << 4)) _interruptsMask &= ~MI_INTR_AI;
        if (value & (1 << 5)) _interruptsMask |= MI_INTR_AI;
        // VI (Clear/Set)
        if (value & (1 << 6)) _interruptsMask &= ~MI_INTR_VI;
        if (value & (1 << 7)) _interruptsMask |= MI_INTR_VI;
        // PI (Clear/Set)
        if (value & (1 << 8)) _interruptsMask &= ~MI_INTR_PI;
        if (value & (1 << 9)) _interruptsMask |= MI_INTR_PI;
        // DP (Clear/Set)
        if (value & (1 << 10)) _interruptsMask &= ~MI_INTR_DP;
        if (value & (1 << 11)) _interruptsMask |= MI_INTR_DP;
        break;
    }
}

bool MIPSInterface::checkInterrupt(std::uint8_t intr) const
{
    return !!(_interrupts & intr);
}

void MIPSInterface::setInterrupt(std::uint8_t intr)
{
    std::printf("MI Interrupt: 0x%02x\n", intr);
    _interrupts |= intr;
}

void MIPSInterface::clearInterrupt(std::uint8_t intr)
{
    _interrupts &= ~intr;
}
