#include <cstdio>
#include <libnin64/State.h>

using namespace libnin64;

State::State()
: cart{}
, memory{}
, mi{}
, pi{mi, memory, cart}
, si{mi, memory}
, vi{mi}
, ai{mi, memory}
, ri{}
, rdp{memory, mi}
, rsp{memory, mi, rdp}
, bus{memory, cart, mi, pi, si, vi, ai, ri, rsp, rdp}
, cpu{bus, mi}
{
}

State::~State()
{
}

Nin64Err State::loadRom(const char* path)
{
    Nin64Err    err;
    const char* cicName;

    if ((err = cart.load(path)))
    {
        return err;
    }
    cart.read(memory.spDmem, 0, 0x1000);
    switch (cart.cic())
    {
    case CIC::NUS_6101:
        cicName = "6101";
        break;
    case CIC::NUS_6102:
        cicName = "6102";
        break;
    case CIC::NUS_6103:
        cicName = "6103";
        break;
    case CIC::NUS_6105:
        cicName = "6105";
        break;
    case CIC::NUS_6106:
        cicName = "6106";
        break;
    default:
        cicName = "Unknown";
        break;
    }
    std::printf("CIC: %s\n", cicName);
    cpu.init(cart.cic());
    rsp.init(cart.cic());
    return NIN64_OK;
}
