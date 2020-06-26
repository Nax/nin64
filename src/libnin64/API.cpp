#include <cstdio>
#include <libnin64/State.h>
#include <nin64/nin64.h>

using namespace libnin64;

NIN64_API Nin64Err nin64CreateState(Nin64State** dst, const char* romPath)
{
    Nin64Err err;
    State*   state;

    state = new State;
    err   = state->loadRom(romPath);
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
    state->cpu.tick(count);
    return NIN64_OK;
}

NIN64_API Nin64Err nin64RunFrame(Nin64State* state)
{
    for (int i = 0; i < (93750000 / 60 / 32); ++i)
    {
        state->cpu.tick(32);
        state->rsp.tick(24);
        state->ai.tick(32);
        state->vi.tick(32);
    }
    std::printf("PC:0x%016llx\n", state->cpu.pc());
    //state->vi.setVBlank();
    return NIN64_OK;
}

NIN64_API Nin64Err nin64SetAudioCallback(Nin64State* state, Nin64AudioCallback callback, void* callbackArg)
{
    state->ai.setCallback(callback, callbackArg);
    return NIN64_OK;
}
