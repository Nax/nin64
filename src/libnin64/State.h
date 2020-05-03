#ifndef INCLUDED_STATE_H
#define INCLUDED_STATE_H

#include <libnin64/Cart.h>
#include <libnin64/Memory.h>
#include <libnin64/NonCopyable.h>

namespace libnin64
{

    class State : private NonCopyable
    {
    public:
        State();
        ~State();

        Nin64Err loadRom(const char* path);

        Cart   cart;
        Memory memory;
    };

}

struct Nin64State : public libnin64::State {};

#endif
