#ifndef INCLUDED_STATE_H
#define INCLUDED_STATE_H

#include <libnin64/Bus.h>
#include <libnin64/Cart.h>
#include <libnin64/CPU.h>
#include <libnin64/Memory.h>
#include <libnin64/NonCopyable.h>
#include <libnin64/PeripheralInterface.h>

namespace libnin64
{

    class State : private NonCopyable
    {
    public:
        State();
        ~State();

        Nin64Err loadRom(const char* path);

        Cart                cart;
        Memory              memory;
        PeripheralInterface pi;
        Bus                 bus;
        CPU                 cpu;
    };

}

struct Nin64State : public libnin64::State {};

#endif
