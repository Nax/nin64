#include <cstdio>
#include <libnin64/MIPSInterface.h>
#include <libnin64/VideoInterface.h>

#define VI_STATUS_REG  0x04400000
#define VI_ORIGIN_REG  0x04400004
#define VI_WIDTH_REG   0x04400008
#define VI_INTR_REG    0x0440000c
#define VI_CURRENT_REG 0x04400010
#define VI_BURST_REG   0x04400014
#define VI_V_SYNC_REG  0x04400018
#define VI_H_SYNC_REG  0x0440001c
#define VI_LEAP_REG    0x04400020
#define VI_H_START_REG 0x04400024
#define VI_V_START_REG 0x04400028
#define VI_V_BURST_REG 0x0440002c
#define VI_X_SCALE_REG 0x04400030
#define VI_Y_SCALE_REG 0x04400034

using namespace libnin64;

VideoInterface::VideoInterface(MIPSInterface& mi)
: _mi{mi}
{
}

VideoInterface::~VideoInterface()
{
}

void VideoInterface::setVBlank()
{
    _mi.setInterrupt(MI_INTR_VI);
}

std::uint32_t VideoInterface::read(std::uint32_t reg)
{
    std::uint32_t value{};

    switch (reg)
    {
    case VI_STATUS_REG:
        break;
    case VI_ORIGIN_REG:
        break;
    case VI_WIDTH_REG:
        break;
    case VI_INTR_REG:
        break;
    case VI_CURRENT_REG:
        break;
    case VI_BURST_REG:
        break;
    case VI_V_SYNC_REG:
        break;
    case VI_H_SYNC_REG:
        break;
    case VI_LEAP_REG:
        break;
    case VI_H_START_REG:
        break;
    case VI_V_START_REG:
        break;
    case VI_V_BURST_REG:
        break;
    case VI_X_SCALE_REG:
        break;
    case VI_Y_SCALE_REG:
        break;
    default:
        break;
    }

    return value;
}

void VideoInterface::write(std::uint32_t reg, std::uint32_t value)
{
    switch (reg)
    {
    case VI_STATUS_REG:
        break;
    case VI_ORIGIN_REG:
        break;
    case VI_WIDTH_REG:
        break;
    case VI_INTR_REG:
        break;
    case VI_CURRENT_REG:
        _mi.clearInterrupt(MI_INTR_VI);
        break;
    case VI_BURST_REG:
        break;
    case VI_V_SYNC_REG:
        break;
    case VI_H_SYNC_REG:
        break;
    case VI_LEAP_REG:
        break;
    case VI_H_START_REG:
        break;
    case VI_V_START_REG:
        break;
    case VI_V_BURST_REG:
        break;
    case VI_X_SCALE_REG:
        break;
    case VI_Y_SCALE_REG:
        break;
    default:
        break;
    }
}
