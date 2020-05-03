#include <nin64/nin64.h>

int main(int argc, char** argv)
{
    Nin64State* state;

    nin64CreateState(&state, argv[1]);
    for (;;) { nin64RunCycles(state, 1024); }
    nin64DestroyState(state);

    return 0;
}
