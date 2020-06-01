#ifndef INCLUDED_RSP_H
#define INCLUDED_RSP_H

#include <cstddef>
#include <cstdint>
#include <libnin64/NonCopyable.h>

#if defined(_MSC_VER)
#include <intrin.h>
#else
#include <x86intrin.h>
#endif

namespace libnin64
{

class Memory;
class MIPSInterface;
class RDP;
class RSP : private NonCopyable
{
public:
    RSP(Memory& memory, MIPSInterface& mi, RDP& rdp);
    ~RSP();

    void tick(std::size_t count);
    void tick();

    std::uint32_t read(std::uint32_t reg);
    void          write(std::uint32_t reg, std::uint32_t value);

private:
    union Reg {
        std::uint8_t  u8;
        std::int8_t   i8;
        std::uint16_t u16;
        std::int16_t  i16;
        std::uint32_t u32;
        std::int32_t  i32;
    };

    union VReg {
        __m128i i;
    };

    void dmaRead(std::uint16_t length, std::uint16_t count, std::uint16_t skip);
    void dmaWrite(std::uint16_t length, std::uint16_t count, std::uint16_t skip);

    template <typename T> T    dRead(std::uint16_t addr);
    template <typename T> void dWrite(std::uint16_t addr, T value);

    std::uint8_t  dRead8(std::uint16_t addr) { return dRead<uint8_t>(addr); }
    std::uint16_t dRead16(std::uint16_t addr) { return dRead<uint16_t>(addr); }
    std::uint32_t dRead32(std::uint16_t addr) { return dRead<uint32_t>(addr); }
    std::uint64_t dRead64(std::uint16_t addr) { return dRead<uint64_t>(addr); }

    void dWrite8(std::uint16_t addr, std::uint8_t value) { dWrite<uint8_t>(addr, value); }
    void dWrite16(std::uint16_t addr, std::uint16_t value) { dWrite<uint16_t>(addr, value); }
    void dWrite32(std::uint16_t addr, std::uint32_t value) { dWrite<uint32_t>(addr, value); }
    void dWrite64(std::uint16_t addr, std::uint64_t value) { dWrite<uint64_t>(addr, value); }

    std::uint32_t cop0Read(std::uint8_t reg);
    void          cop0Write(std::uint8_t reg, std::uint32_t value);

    Memory&        _memory;
    MIPSInterface& _mi;
    RDP&           _rdp;

    bool          _halt : 1;
    bool          _broke : 1;
    bool          _semaphore : 1;
    bool          _interruptOnBreak : 1;
    std::uint8_t  _signal;
    std::uint16_t _spAddr;
    std::uint32_t _dramAddr;
    Reg           _regs[32];
    VReg          _vregs[32];
    __m128i       _acc_lo;
    __m128i       _acc_md;
    __m128i       _acc_hi;
    std::uint16_t _pc;
    std::uint16_t _pcNext;
};

} // namespace libnin64

#endif
