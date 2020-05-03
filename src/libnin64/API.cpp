#include <nin64/nin64.h>
#include <libnin64/State.h>

using namespace libnin64;

NIN64_API Nin64Err nin64CreateState(Nin64State** dst, const char* romPath)
{
    State* state;

    state = new State;
    state->loadRom(romPath);
    *dst = (Nin64State*)state;

    return NIN64_OK;
}

NIN64_API Nin64Err nin64DestroyState(Nin64State* state)
{
    delete state;
    return NIN64_OK;
}
