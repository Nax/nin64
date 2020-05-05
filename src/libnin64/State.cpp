#include <libnin64/State.h>

using namespace libnin64;

State::State()
: cart{}
, memory{}
, pi{memory, cart}
, bus{memory, pi}
, cpu{bus}
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
    return NIN64_OK;
}
