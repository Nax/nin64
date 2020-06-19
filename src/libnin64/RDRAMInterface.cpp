#include <libnin64/RDRAMInterface.h>

#define RI_MODE_REG         0x04700000
#define RI_CONFIG_REG       0x04700004
#define RI_CURRENT_LOAD_REG 0x04700008
#define RI_SELECT_REG       0x0470000c
#define RI_REFRESH_REG      0x04700010
#define RI_LATENCY_REG      0x04700014
#define RI_RERROR_REG       0x04700018
#define RI_WERROR_REG       0x0470001c

using namespace libnin64;

RDRAMInterface::RDRAMInterface()
: _values{}
{
}

RDRAMInterface::~RDRAMInterface()
{
}

std::uint32_t RDRAMInterface::read(std::uint32_t reg)
{
    std::uint32_t value{};

    switch (reg)
    {
    case RI_MODE_REG:
        value = _values[0];
        break;
    case RI_CONFIG_REG:
        value = _values[1];
        break;
    case RI_CURRENT_LOAD_REG:
        break;
    case RI_SELECT_REG:
        value = _values[2];
        break;
    case RI_REFRESH_REG:
        value = _values[3];
        break;
    case RI_LATENCY_REG:
        value = _values[4];
        break;
    case RI_RERROR_REG:
        break;
    case RI_WERROR_REG:
        break;
    }

    return value;
}

void RDRAMInterface::write(std::uint32_t reg, std::uint32_t value)
{
    switch (reg)
    {
    case RI_MODE_REG:
        _values[0] = value & 0x0f;
        break;
    case RI_CONFIG_REG:
        _values[1] = value & 0x8f;
        break;
    case RI_CURRENT_LOAD_REG:
        break;
    case RI_SELECT_REG:
        _values[2] = value & 0x07;
        break;
    case RI_REFRESH_REG:
        _values[3] = value & 0x7ffff;
        break;
    case RI_LATENCY_REG:
        _values[4] = value & 0x0f;
        break;
    case RI_RERROR_REG:
        break;
    case RI_WERROR_REG:
        break;
    }
}
