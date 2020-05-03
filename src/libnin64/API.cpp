#include <nin64/nin64.h>
#include <libnin64/State.h>

using namespace libnin64;

NIN64_API Nin64Err nin64CreateState(Nin64State** dst, const char* romPath)
{
    Nin64Err err;
    State* state;

    state = new State;
    err = state->loadRom(romPath);
    if (err)
    {
        delete state;
        *dst = nullptr;
        return err;
    }
    *dst = (Nin64State*)state;

    return NIN64_OK;
}

NIN64_API Nin64Err nin64DestroyState(Nin64State* state)
{
    delete state;
    return NIN64_OK;
}

NIN64_API Nin64Err nin64RunCycles(Nin64State* state, size_t count)
{
    for (size_t i = 0; i < count; ++i)
    {
        state->cpu.tick();
    }
    return NIN64_OK;
}
