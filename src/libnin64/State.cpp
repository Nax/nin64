#include <libnin64/State.h>

using namespace libnin64;

State::State()
    : cart{}
    , memory{}
{

}

State::~State()
{

}

Nin64Err State::loadRom(const char* path)
{
    return cart.load(path);
}
