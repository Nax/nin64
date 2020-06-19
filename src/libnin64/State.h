#ifndef INCLUDED_STATE_H
#define INCLUDED_STATE_H

#include <libnin64/AudioInterface.h>
#include <libnin64/Bus.h>
#include <libnin64/CPU.h>
#include <libnin64/Cart.h>
#include <libnin64/MIPSInterface.h>
#include <libnin64/Memory.h>
#include <libnin64/NonCopyable.h>
#include <libnin64/PeripheralInterface.h>
#include <libnin64/RDP.h>
#include <libnin64/RDRAMInterface.h>
#include <libnin64/RSP.h>
#include <libnin64/SerialInterface.h>
#include <libnin64/VideoInterface.h>

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
    MIPSInterface       mi;
    PeripheralInterface pi;
    SerialInterface     si;
    VideoInterface      vi;
    AudioInterface      ai;
    RDRAMInterface      ri;
    RDP                 rdp;
    RSP                 rsp;
    Bus                 bus;
    CPU                 cpu;
};

} // namespace libnin64

struct Nin64State : public libnin64::State
{
};

#endif
