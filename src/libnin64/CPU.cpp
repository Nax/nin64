#include <libnin64/Bus.h>
#include <libnin64/CPU.h>

#define RS              ((std::uint8_t)((op >> 21) & 0x1f))
#define RT              ((std::uint8_t)((op >> 16) & 0x1f))
#define RD              ((std::uint8_t)((op >> 11) & 0x1f))
#define IMM             ((std::uint16_t)op)
#define SIMM            ((std::int16_t)op)
#define JUMP_TARGET     ((std::uint32_t)op & 0x3fffff)

using namespace libnin64;

CPU::CPU(Bus& bus)
: _bus{bus}
, _pc{0xa4000040}
, _pcNext{_pc + 4}
, _regs{}
{
}

CPU::~CPU()
{
}

#include <cstdio>

void CPU::tick()
{
    std::printf("PC: 0x%016x\n", _pc);
    std::uint32_t op;

    op = _bus.read32((std::uint32_t)_pc);
    _pc = _pcNext;
    _pcNext += 4;

    switch (op >> 26)
    {
    case 000: // SPECIAL
        break;
    case 001: // REGIMM
        break;
    case 002: // J (Jump)
        _pcNext = ((std::uint64_t)JUMP_TARGET << 2) | (_pc & 0xfffffffff0000000ULL);
        break;
    case 003: // JAL (Jump And Link)
        _pcNext = ((std::uint64_t)JUMP_TARGET << 2) | (_pc & 0xfffffffff0000000ULL);
        _regs[31].u64 = _pc + 4;
        break;
    case 004: // BEQ
        break;
    case 005: // BNE
        break;
    case 006: // BLEZ
        break;
    case 007: // BGTZ
        break;
    case 010: // ADDI (Add Immediate)
        _regs[RT].i64 = _regs[RS].i32 + SIMM;
        break;
    case 011: // ADDIU (Add Immediate Unsigned)
        _regs[RT].i64 = _regs[RS].i32 + SIMM;
        break;
    case 012: // SLTI (Set On Less Than Immediate)
        break;
    case 013: // SLTIU (Set On Less Than Immediate Unsigned)
        break;
    case 014: // ANDI (And Immediate)
        _regs[RT].u64 = _regs[RS].u64 & IMM;
        break;
    case 015: // ORI (Or Immediate)
        _regs[RT].u64 = _regs[RS].u64 | IMM;
        break;
    case 016: // XORI (Exclusive Or Immediate)
        _regs[RT].u64 = _regs[RS].u64 ^ IMM;
        break;
    case 017: // LUI (Load Upper immediate)
        _regs[RT].i64 = ((std::int32_t)SIMM << 16);
        break;
    case 020: // COP0 (Coprocessor 0)
        break;
    case 021: // COP1 (Coprocessor 1)
        break;
    case 022: // COP2 (Coprocessor 2)
        break;
    case 024: // BEQL
        break;
    case 025: // BNEL
        break;
    case 026: // BLEZL
        break;
    case 027: // BGTZL (Branch On Greater Than Zero Likely)
        break;
    case 030: // DADDI (Doubleword Add Immediate)
        _regs[RT].i64 = _regs[RS].i64 + SIMM;
        break;
    case 031: // DADDIU (Doubleword Add Immediate Unsigned)
        _regs[RT].i64 = _regs[RS].i64 + SIMM;
        break;
    case 032: // LDL
        break;
    case 033: // LDR
        break;
    case 040: // LB (Load Byte)
        _regs[RT].i64 = (std::int8_t)_bus.read8(_regs[RS].u32 + SIMM);
        break;
    case 041: // LH (Load Halfword)
        _regs[RT].i64 = (std::int16_t)_bus.read16(_regs[RS].u32 + SIMM);
        break;
    case 042: // LWL
        break;
    case 043: // LW (Load Word)
        _regs[RT].i64 = (std::int32_t)_bus.read32(_regs[RS].u32 + SIMM);
        break;
    case 044: // LBU (Load Byte Unsigned)
        _regs[RT].u64 = _bus.read8(_regs[RS].u32 + SIMM);
        break;
    case 045: // LHU (Load Halfword Unsigned)
        _regs[RT].u64 = _bus.read16(_regs[RS].u32 + SIMM);
        break;
    case 046: // LWR
        break;
    case 047: // LWU
        _regs[RT].u64 = _bus.read32(_regs[RS].u32 + SIMM);
        break;
    case 050: // SB (Store Byte)
        _bus.write8(_regs[RS].u32 + SIMM, _regs[RT].u8);
        break;
    case 051: // SH (Store Halfword)
        _bus.write16(_regs[RS].u32 + SIMM, _regs[RT].u16);
        break;
    case 052: // SWL (Store Word Left)
        break;
    case 053: // SW (Store Word)
        _bus.write32(_regs[RS].u32 + SIMM, _regs[RT].u32);
        break;
    case 054: // SDL (Store Doubleword Left)
        break;
    case 055: // SDR (Store Doubleword Right)
        break;
    case 056: // SWR (Store Word Right)
        break;
    case 057: // CACHE
        break;
    case 060: // LL (Load Linked)
        _regs[RT].i64 = (std::int32_t)_bus.read32(_regs[RS].u32 + SIMM);
        // TODO: More shit...
        break;
    case 061: // LWC1
        break;
    case 062: // LWC2
        break;
    case 064: // LLD
        break;
    case 065: // LDC1
        break;
    case 066: // LDC2
        break;
    case 067: // LD (Load Doubleword)
        _regs[RT].u64 = _bus.read64(_regs[RS].u32 + SIMM);
        break;
    case 070: // SC (Store Conditional)
        break;
    case 071: // SWC1
        break;
    case 072: // SWC2
        break;
    case 074: // SCD (Store Conditional Doubleword)
        break;
    case 075: // SDC1 (Store Doubleword From Coprocessor 1)
        break;
    case 076: // SDC2 (Store Doubleword From Coprocessor 2)
        break;
    case 077: // SD (Store Doubleword)
        _bus.write64(_regs[RS].u32 + SIMM, _regs[RT].u64);
        break;
    }

    _regs[0].u64 = 0;
}






