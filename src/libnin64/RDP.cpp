#include <cstdio>
#include <cstdlib>
#include <libnin64/Memory.h>
#include <libnin64/MipsInterface.h>
#include <libnin64/RDP.h>
#include <libnin64/Util.h>

#define NOT_IMPLEMENTED()                                                                    \
    do                                                                                       \
    {                                                                                        \
        std::printf("%s:%d: Unknown RDP Command: 0x%016llx\n", __FILE__, __LINE__, command); \
        std::exit(1);                                                                        \
    } while (0)

using namespace libnin64;

RDP::RDP(Memory& memory, MIPSInterface& mi)
: _memory{memory}
, _mi{mi}
, _xbus{}
, _cmdStart{}
, _cmdEnd{}
, _cmdCurrent{}
{
}

RDP::~RDP()
{
}

std::uint32_t RDP::read(std::uint32_t reg)
{
    std::uint32_t value{};

    switch (reg)
    {
    case DPC_START_REG:
        std::printf("DP Read: DPC_START_REG\n");
        value = _cmdStart;
        break;
    case DPC_END_REG:
        std::printf("DP Read: DPC_END_REG\n");
        value = _cmdEnd;
        break;
    case DPC_CURRENT_REG:
        std::printf("DP Read: DPC_CURRENT_REG\n");
        value = _cmdCurrent;
        break;
    case DPC_STATUS_REG:
        std::printf("DP Read: DPC_STATUS_REG\n");
        if (_xbus)
            value |= 0x01;
        value |= 0x00000080;
        break;
    case DPC_CLOCK_REG:
        std::printf("DP Read: DPC_CLOCK_REG\n");
        std::exit(1);
        break;
    case DPC_BUFBUSY_REG:
        std::printf("DP Read: DPC_BUFBUSY_REG\n");
        std::exit(1);
        break;
    case DPC_PIPEBUSY_REG:
        std::printf("DP Read: DPC_PIPEBUSY_REG\n");
        std::exit(1);
        break;
    case DPC_TMEM_REG:
        std::printf("DP Read: DPC_TMEM_REG\n");
        std::exit(1);
        break;
    case DPS_TBIST_REG:
        std::printf("DP Read: DPS_TBIST_REG\n");
        std::exit(1);
        break;
    case DPS_TEST_MODE_REG:
        std::printf("DP Read: DPS_TEST_MODE_REG\n");
        std::exit(1);
        break;
    case DPS_BUFTEST_ADDR_REG:
        std::printf("DP Read: DPS_BUFTEST_ADDR_REG\n");
        std::exit(1);
        break;
    case DPS_BUFTEST_DATA_REG:
        std::printf("DP Read: DPS_BUFTEST_DATA_REG\n");
        std::exit(1);
        break;
    default:
        std::printf("RDP Unknown reg\n");
        std::exit(1);
        break;
    }

    return value;
}

void RDP::write(std::uint32_t reg, std::uint32_t value)
{
    switch (reg)
    {
    case DPC_START_REG:
        std::printf("DP Write DPC_START_REG: %08x\n", value);
        _cmdStart   = value & 0xffffff;
        _cmdCurrent = _cmdStart;
        break;
    case DPC_END_REG:
        std::printf("DP Write DPC_END_REG: %08x\n", value);
        _cmdEnd = value & 0xffffff;
        dma();
        break;
    case DPC_CURRENT_REG:
        std::printf("DP Write DPC_CURRENT_REG: %08x\n", value);
        std::exit(1);
        break;
    case DPC_STATUS_REG:
        std::printf("DP Write DPC_STATUS_REG: %08x\n", value);
        if (value & 0x01) _xbus = false;
        if (value & 0x02) _xbus = true;
        break;
    case DPC_CLOCK_REG:
        std::printf("DP Write DPC_CLOCK_REG: %08x\n", value);
        std::exit(1);
        break;
    case DPC_BUFBUSY_REG:
        std::printf("DP Write DPC_BUFBUSY_REG: %08x\n", value);
        std::exit(1);
        break;
    case DPC_PIPEBUSY_REG:
        std::printf("DP Write DPC_PIPEBUSY_REG: %08x\n", value);
        std::exit(1);
        break;
    case DPC_TMEM_REG:
        std::printf("DP Write DPC_TMEM_REG: %08x\n", value);
        std::exit(1);
        break;
    case DPS_TBIST_REG:
        std::printf("DP Write DPS_TBIST_REG: %08x\n", value);
        std::exit(1);
        break;
    case DPS_TEST_MODE_REG:
        std::printf("DP Write DPS_TEST_MODE_REG: %08x\n", value);
        std::exit(1);
        break;
    case DPS_BUFTEST_ADDR_REG:
        std::printf("DP Write DPS_BUFTEST_ADDR_REG: %08x\n", value);
        std::exit(1);
        break;
    case DPS_BUFTEST_DATA_REG:
        std::printf("DP Write DPS_BUFTEST_DATA_REG: %08x\n", value);
        std::exit(1);
        break;
    default:
        std::printf("RDP Unknown reg\n");
        std::exit(1);
        break;
    }
}

void RDP::dma()
{
    std::uint64_t command;

    while (_cmdCurrent < _cmdEnd)
    {
        command = swap64(*(std::uint64_t*)(_memory.ram + _cmdCurrent));
        _cmdCurrent += 8;

        switch ((command >> 56) & 0x3f)
        {
        case 0x00: // No Op
            NOT_IMPLEMENTED();
            break;
        case 0x08: // Non-ShadedTriangle
        case 0x09:
        case 0x0a:
        case 0x0b:
        case 0x0c:
        case 0x0d:
        case 0x0e:
        case 0x0f:
            NOT_IMPLEMENTED();
            break;
        case 0x25: // Texture Rectangle Flip
            NOT_IMPLEMENTED();
            break;
        case 0x26: // Unknown
            // TODO: Is this normal?
            break;
        case 0x27: // Sync Pipe
            // TODO: Add me
            break;
        case 0x28: // Sync Tile
            // TODO: Add me
            break;
        case 0x29: // Sync Full
            // TODO: Add me
            _mi.setInterrupt(MI_INTR_DP);
            break;
        case 0x2a: // Set Key GB
            NOT_IMPLEMENTED();
            break;
        case 0x2b: // Set Key R
            NOT_IMPLEMENTED();
            break;
        case 0x2c: // Set Convert
            NOT_IMPLEMENTED();
            break;
        case 0x2d: // Set Scissor
            // TODO: Add me
            break;
        case 0x2e: // Set Prim Color
            NOT_IMPLEMENTED();
            break;
        case 0x2f: // Set Other Modes
            // TODO: Add me
            break;
        case 0x30: // Load Tlut
            NOT_IMPLEMENTED();
            break;
        case 0x31: // Sync Load
            NOT_IMPLEMENTED();
            break;
        case 0x32: // Set Tile Size
            // TODO: Add me
            break;
        case 0x33: // Load Block
            // TODO: Add me
            break;
        case 0x34: // Load Tile
            NOT_IMPLEMENTED();
            break;
        case 0x35: // Set Tile
            // TODO: Add me
            break;
        case 0x36: // Fill Rectangle
            // TODO: Add me
            break;
        case 0x37: // Set Fill Color
            // TODO: Add me
            break;
        case 0x38: // Set Fog Color
            NOT_IMPLEMENTED();
            break;
        case 0x39: // Set Blend Color
            NOT_IMPLEMENTED();
            break;
        case 0x3a: // Set Prim Color
            NOT_IMPLEMENTED();
            break;
        case 0x3b: // Set Env Color
            NOT_IMPLEMENTED();
            break;
        case 0x3c: // Set Combine Mode
            // TODO: Add me
            break;
        case 0x3d: // Set Texture Image
            // TODO: Add me
            break;
        case 0x3e: // Set Z Image
            // TODO: Add me
            break;
        case 0x3f: // Set Color Image
            // TODO: Add me
            break;
        default:
            NOT_IMPLEMENTED();
            break;
        }
    }
}
