#include <cstdio>
#include <cstring>
#include <libnin64/MIPSInterface.h>
#include <libnin64/Memory.h>
#include <libnin64/SerialInterface.h>

#define SI_DRAM_ADDR_REG      0x04800000
#define SI_PIF_ADDR_RD64B_REG 0x04800004
#define SI_PIF_ADDR_WR64B_REG 0x04800010
#define SI_STATUS_REG         0x04800018

using namespace libnin64;

SerialInterface::SerialInterface(MIPSInterface& mi, Memory& memory)
: _mi{mi}
, _memory{memory}
, _addr{}
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
        value = _addr;
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
        _addr = value & 0x00ffffff;
        break;
    case SI_PIF_ADDR_RD64B_REG:
        std::printf("SI Write: SI_PIF_ADDR_RD64B_REG: 0x%08x\n", value);
        std::getchar();
        dmaRead();
        break;
    case SI_PIF_ADDR_WR64B_REG:
        std::printf("SI Write: SI_PIF_ADDR_WR64B_REG: 0x%08x\n", value);
        std::getchar();
        dmaWrite();
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

void SerialInterface::pifUpdate()
{
    if (_memory.pif[63] != 1)
        return;
    std::printf("PIF Command!\n");
    for (int i = 0; i < 8; ++i)
    {
        std::printf("  %02x%02x%02x%02x - %02x%02x%02x%02x\n",
                    _memory.pif[i * 8 + 0],
                    _memory.pif[i * 8 + 1],
                    _memory.pif[i * 8 + 2],
                    _memory.pif[i * 8 + 3],
                    _memory.pif[i * 8 + 4],
                    _memory.pif[i * 8 + 5],
                    _memory.pif[i * 8 + 6],
                    _memory.pif[i * 8 + 7]);
    }
    std::getchar();

    for (int i = 0; i < 8; ++i)
    {
        if (_memory.pif[i * 8 + 0] == 0xfe)
            break;
        else if (_memory.pif[i * 8 + 0] == 0xff)
        {
            switch (_memory.pif[i * 8 + 3])
            {
            case 0xff: // Reset
                _memory.pif[i * 8 + 4] = 0;
                _memory.pif[i * 8 + 5] = 0;
                _memory.pif[i * 8 + 6] = 0;
                _memory.pif[i * 8 + 7] = 0;
                break;
            }
        }
    }
    _memory.pif[63] = 0;
}

void SerialInterface::dmaRead()
{
    std::memcpy(_memory.ram + _addr, _memory.pif, 64);
    _mi.setInterrupt(MI_INTR_SI);
}

void SerialInterface::dmaWrite()
{
    std::memcpy(_memory.pif, _memory.ram + _addr, 64);
    pifUpdate();
    _mi.setInterrupt(MI_INTR_SI);
}
