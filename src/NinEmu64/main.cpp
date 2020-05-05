#include <cstdlib>
#include <cstdio>
#include <nin64/nin64.h>

static void displayError(Nin64Err err)
{
    switch (err)
    {
    case NIN64_ERROR_IO:
        std::puts("IO Error");
        break;
    case NIN64_ERROR_OUTOFMEMORY:
        std::puts("Memory Error");
        break;
    case NIN64_ERROR_BADROM:
        std::puts("Bad Rom");
        break;
    default:
        std::puts("Unknown Error");
        break;
    }
}

int main(int argc, char **argv)
{
    Nin64State *state;
    Nin64Err err;

    err = nin64CreateState(&state, argv[1]);
    if (err)
    {
        displayError(err);
        std::exit(1);
    }
    for (;;)
    {
        nin64RunCycles(state, 1024);
    }
    nin64DestroyState(state);

    return 0;
}
