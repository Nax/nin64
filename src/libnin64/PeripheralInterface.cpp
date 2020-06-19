#include <cstdio>
#include <libnin64/Cart.h>
#include <libnin64/MIPSInterface.h>
#include <libnin64/Memory.h>
#include <libnin64/PeripheralInterface.h>

#define PI_DRAM_ADDR_REG    0x04600000
#define PI_CART_ADDR_REG    0x04600004
#define PI_RD_LEN_REG       0x04600008
#define PI_WR_LEN_REG       0x0460000c
#define PI_STATUS_REG       0x04600010
#define PI_BSD_DOM1_LAT_REG 0x04600014
#define PI_BSD_DOM1_PWD_REG 0x04600018
#define PI_BSD_DOM1_PGS_REG 0x0460001c
#define PI_BSD_DOM1_RLS_REG 0x04600020
#define PI_BSD_DOM2_LAT_REG 0x04600024
#define PI_BSD_DOM2_PWD_REG 0x04600028
#define PI_BSD_DOM2_PGS_REG 0x0460002c
#define PI_BSD_DOM2_RLS_REG 0x04600030

using namespace libnin64;

PeripheralInterface::PeripheralInterface(MIPSInterface& mi, Memory& memory, Cart& cart)
: _mi{mi}
, _memory{memory}
, _cart{cart}
, _dramAddr{}
, _cartAddr{}
{
}

PeripheralInterface::~PeripheralInterface()
{
}

std::uint32_t PeripheralInterface::read(std::uint32_t reg)
{
    std::uint32_t value{};

    switch (reg)
    {
    case PI_DRAM_ADDR_REG:
        std::puts("READ :: PI_DRAM_ADDR_REG");
        value = _dramAddr;
        break;
    case PI_CART_ADDR_REG:
        std::puts("READ :: PI_CART_ADDR_REG");
        value = _cartAddr;
        break;
    case PI_RD_LEN_REG:
        std::puts("READ :: PI_RD_LEN_REG");
        break;
    case PI_WR_LEN_REG:
        std::puts("READ :: PI_WR_LEN_REG");
        break;
    case PI_STATUS_REG:
        std::puts("READ :: PI_STATUS_REG");
        break;
    case PI_BSD_DOM1_LAT_REG:
        std::puts("READ :: PI_BSD_DOM1_LAT_REG");
        break;
    case PI_BSD_DOM1_PWD_REG:
        std::puts("READ :: PI_BSD_DOM1_PWD_REG");
        break;
    case PI_BSD_DOM1_PGS_REG:
        std::puts("READ :: PI_BSD_DOM1_PGS_REG");
        break;
    case PI_BSD_DOM1_RLS_REG:
        std::puts("READ :: PI_BSD_DOM1_RLS_REG");
        break;
    case PI_BSD_DOM2_LAT_REG:
        std::puts("READ :: PI_BSD_DOM2_LAT_REG");
        break;
    case PI_BSD_DOM2_PWD_REG:
        std::puts("READ :: PI_BSD_DOM2_PWD_REG");
        break;
    case PI_BSD_DOM2_PGS_REG:
        std::puts("READ :: PI_BSD_DOM2_PGS_REG");
        break;
    case PI_BSD_DOM2_RLS_REG:
        std::puts("READ :: PI_BSD_DOM2_RLS_REG");
        break;
    default:
        break;
    }

    return value;
}

void PeripheralInterface::write(std::uint32_t reg, std::uint32_t value)
{
    switch (reg)
    {
    case PI_DRAM_ADDR_REG:
        std::puts("WRITE :: PI_DRAM_ADDR_REG");
        _dramAddr = value & (0xffffff);
        break;
    case PI_CART_ADDR_REG:
        std::puts("WRITE :: PI_CART_ADDR_REG");
        _cartAddr = value;
        break;
    case PI_RD_LEN_REG:
        std::puts("WRITE :: PI_RD_LEN_REG");
        _mi.setInterrupt(MI_INTR_PI);
        break;
    case PI_WR_LEN_REG:
        std::puts("WRITE :: PI_WR_LEN_REG");
        std::printf("0x%08x 0x%08x 0x%08x\n", _dramAddr, _cartAddr, value);
        _cart.read(_memory.ram + _dramAddr, _cartAddr & 0x0fffffff, (value & 0xffffff) + 1);
        _mi.setInterrupt(MI_INTR_PI);
        break;
    case PI_STATUS_REG:
        std::puts("WRITE :: PI_STATUS_REG");
        if (value & (1 << 1)) _mi.clearInterrupt(MI_INTR_PI);
        break;
    case PI_BSD_DOM1_LAT_REG:
        std::puts("WRITE :: PI_BSD_DOM1_LAT_REG");
        break;
    case PI_BSD_DOM1_PWD_REG:
        std::puts("WRITE :: PI_BSD_DOM1_PWD_REG");
        break;
    case PI_BSD_DOM1_PGS_REG:
        std::puts("WRITE :: PI_BSD_DOM1_PGS_REG");
        break;
    case PI_BSD_DOM1_RLS_REG:
        std::puts("WRITE :: PI_BSD_DOM1_RLS_REG");
        break;
    case PI_BSD_DOM2_LAT_REG:
        std::puts("WRITE :: PI_BSD_DOM2_LAT_REG");
        break;
    case PI_BSD_DOM2_PWD_REG:
        std::puts("WRITE :: PI_BSD_DOM2_PWD_REG");
        break;
    case PI_BSD_DOM2_PGS_REG:
        std::puts("WRITE :: PI_BSD_DOM2_PGS_REG");
        break;
    case PI_BSD_DOM2_RLS_REG:
        std::puts("WRITE :: PI_BSD_DOM2_RLS_REG");
        break;
    default:
        break;
    }
}
