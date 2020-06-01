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
, _sync{}
{
}

VideoInterface::~VideoInterface()
{
}

void VideoInterface::setVBlank()
{
    if (_sync)
    {
        std::printf("VI set!\n");
        _mi.setInterrupt(MI_INTR_VI);
    }
}

std::uint32_t VideoInterface::read(std::uint32_t reg)
{
    std::uint32_t value{};

    switch (reg)
    {
    case VI_STATUS_REG:
        std::printf("VI Read: VI_STATUS_REG\n");
        break;
    case VI_ORIGIN_REG:
        std::printf("VI Read: VI_ORIGIN_REG\n");
        break;
    case VI_WIDTH_REG:
        std::printf("VI Read: VI_WIDTH_REG\n");
        break;
    case VI_INTR_REG:
        std::printf("VI Read: VI_INTR_REG\n");
        break;
    case VI_CURRENT_REG:
        std::printf("VI Read: VI_CURRENT_REG\n");
        break;
    case VI_BURST_REG:
        std::printf("VI Read: VI_BURST_REG\n");
        break;
    case VI_V_SYNC_REG:
        std::printf("VI Read: VI_V_SYNC_REG\n");
        break;
    case VI_H_SYNC_REG:
        std::printf("VI Read: VI_H_SYNC_REG\n");
        break;
    case VI_LEAP_REG:
        std::printf("VI Read: VI_LEAP_REG\n");
        break;
    case VI_H_START_REG:
        std::printf("VI Read: VI_H_START_REG\n");
        break;
    case VI_V_START_REG:
        std::printf("VI Read: VI_V_START_REG\n");
        break;
    case VI_V_BURST_REG:
        std::printf("VI Read: VI_V_BURST_REG\n");
        break;
    case VI_X_SCALE_REG:
        std::printf("VI Read: VI_X_SCALE_REG\n");
        break;
    case VI_Y_SCALE_REG:
        std::printf("VI Read: VI_Y_SCALE_REG\n");
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
        _sync = !!(value & 0x03);
        std::printf("VI Write: VI_STATUS_REG: 0x%08x\n", value);
        break;
    case VI_ORIGIN_REG:
        std::printf("VI Write: VI_ORIGIN_REG: 0x%08x\n", value);
        break;
    case VI_WIDTH_REG:
        std::printf("VI Write: VI_WIDTH_REG: 0x%08x\n", value);
        break;
    case VI_INTR_REG:
        std::printf("VI Write: VI_INTR_REG: 0x%08x\n", value);
        break;
    case VI_CURRENT_REG:
        _mi.clearInterrupt(MI_INTR_VI);
        std::printf("VI Write: VI_CURRENT_REG: 0x%08x\n", value);
        std::printf("VI clear!\n");
        break;
    case VI_BURST_REG:
        std::printf("VI Write: VI_BURST_REG: 0x%08x\n", value);
        break;
    case VI_V_SYNC_REG:
        std::printf("VI Write: VI_V_SYNC_REG: 0x%08x\n", value);
        break;
    case VI_H_SYNC_REG:
        std::printf("VI Write: VI_H_SYNC_REG: 0x%08x\n", value);
        break;
    case VI_LEAP_REG:
        std::printf("VI Write: VI_LEAP_REG: 0x%08x\n", value);
        break;
    case VI_H_START_REG:
        std::printf("VI Write: VI_H_START_REG: 0x%08x\n", value);
        break;
    case VI_V_START_REG:
        std::printf("VI Write: VI_V_START_REG: 0x%08x\n", value);
        break;
    case VI_V_BURST_REG:
        std::printf("VI Write: VI_V_BURST_REG: 0x%08x\n", value);
        break;
    case VI_X_SCALE_REG:
        std::printf("VI Write: VI_X_SCALE_REG: 0x%08x\n", value);
        break;
    case VI_Y_SCALE_REG:
        std::printf("VI Write: VI_Y_SCALE_REG: 0x%08x\n", value);
        break;
    default:
        break;
    }
}
