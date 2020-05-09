#include <libnin64/State.h>

using namespace libnin64;

State::State()
: cart{}
, memory{}
, mi{}
, pi{mi, memory, cart}
, si{mi}
, vi{mi}
, rsp{memory, mi}
, bus{memory, cart, mi, pi, si, vi, rsp}
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
