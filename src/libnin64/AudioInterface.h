#ifndef INCLUDED_AUDIO_INTERFACE_H
#define INCLUDED_AUDIO_INTERFACE_H

#include <cstddef>
#include <cstdint>
#include <libnin64/NonCopyable.h>
#include <nin64/nin64.h>

namespace libnin64
{

class Memory;
class MIPSInterface;
class AudioInterface : private NonCopyable
{
public:
    AudioInterface(MIPSInterface& mi, Memory& memory);
    ~AudioInterface();

    void setCallback(Nin64AudioCallback callback, void* callbackArg);

    void          tick(std::uint32_t count);
    std::uint32_t read(std::uint32_t reg);
    void          write(std::uint32_t reg, std::uint32_t value);

private:
    void dma(std::uint32_t size);

    MIPSInterface& _mi;
    Memory&        _memory;

    Nin64AudioCallback _callback;
    void*              _callbackArg;

    std::uint32_t _acc;
    std::uint32_t _addr;
    std::uint32_t _len[2];
    std::uint8_t  _bufCount;
    std::uint16_t _buffer[0x60000];
};

} // namespace libnin64

#endif
