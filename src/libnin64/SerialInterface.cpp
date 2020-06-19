#include <cstdio>
#include <libnin64/MIPSInterface.h>
#include <libnin64/SerialInterface.h>

#define SI_DRAM_ADDR_REG      0x04800000
#define SI_PIF_ADDR_RD64B_REG 0x04800004
#define SI_PIF_ADDR_WR64B_REG 0x04800010
#define SI_STATUS_REG         0x04800018

using namespace libnin64;

SerialInterface::SerialInterface(MIPSInterface& mi)
: _mi{mi}
{
}

SerialInterface::~SerialInterface()
{
}

std::uint32_t SerialInterface::read(std::uint32_t reg)
{
    std::uint32_t value{};

    switch (reg)
    {
    case SI_DRAM_ADDR_REG:
        std::printf("SI Read: SI_DRAM_ADDR_REG\n");
        std::getchar();
        break;
    case SI_PIF_ADDR_RD64B_REG:
        std::printf("SI Read: SI_PIF_ADDR_RD64B_REG\n");
        std::getchar();
        break;
    case SI_PIF_ADDR_WR64B_REG:
        std::printf("SI Read: SI_PIF_ADDR_WR64B_REG\n");
        std::getchar();
        break;
    case SI_STATUS_REG:
        std::printf("SI Read: SI_STATUS_REG\n");
        std::getchar();
        if (_mi.checkInterrupt(MI_INTR_SI)) value |= (1 << 12);
        break;
    default:
        break;
    }

    return value;
}

void SerialInterface::write(std::uint32_t reg, std::uint32_t value)
{
    switch (reg)
    {
    case SI_DRAM_ADDR_REG:
        std::printf("SI Write: SI_DRAM_ADDR_REG: 0x%08x\n", value);
        std::getchar();
        break;
    case SI_PIF_ADDR_RD64B_REG:
        std::printf("SI Write: SI_PIF_ADDR_RD64B_REG: 0x%08x\n", value);
        std::getchar();
        _mi.setInterrupt(MI_INTR_SI);
        break;
    case SI_PIF_ADDR_WR64B_REG:
        std::printf("SI Write: SI_PIF_ADDR_WR64B_REG: 0x%08x\n", value);
        std::getchar();
        _mi.setInterrupt(MI_INTR_SI);
        break;
    case SI_STATUS_REG:
        std::printf("SI Write: SI_STATUS_REG: 0x%08x\n", value);
        std::getchar();
        _mi.clearInterrupt(MI_INTR_SI);
        break;
    default:
        break;
    }
}
