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
    Nin64Err err;

    if ((err = cart.load(path)))
    {
        return err;
    }
    cart.read(memory.spDmem, 0, 0x1000);
    cpu.init(cart.cic());
    return NIN64_OK;
}
