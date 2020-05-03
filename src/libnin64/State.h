#ifndef INCLUDED_STATE_H
#define INCLUDED_STATE_H

#include <libnin64/NonCopyable.h>

namespace libnin64
{

class State : private NonCopyable
{
public:
    State();
    ~State();

    void loadRom(const char* path);
private:
};

}

struct Nin64State : public libnin64::State {};

#endif
