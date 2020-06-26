#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <libnin64/Bus.h>
#include <libnin64/CPU.h>
#include <libnin64/MIPSInterface.h>
#include <libnin64/Util.h>

#define COP0_REG_INDEX    0
#define COP0_REG_RANDOM   1
#define COP0_REG_ENTRYLO0 2
#define COP0_REG_ENTRYLO1 3
#define COP0_REG_CONTEXT  4
#define COP0_REG_PAGEMASK 5
#define COP0_REG_WIRED    6
#define COP0_REG_BADVADDR 8
#define COP0_REG_COUNT    9
#define COP0_REG_ENTRYHI  10
#define COP0_REG_COMPARE  11
#define COP0_REG_SR       12
#define COP0_REG_CAUSE    13
#define COP0_REG_EPC      14
#define COP0_REG_PRID     15
#define COP0_REG_CONFIG   16
#define COP0_REG_LLADDR   17
#define COP0_REG_WATCHLO  18
#define COP0_REG_WATCHHI  19
#define COP0_REG_XCONTEXT 20
#define COP0_REG_PERR     26
#define COP0_REG_CACHEERR 27
#define COP0_REG_TAGLO    28
#define COP0_REG_TAGHI    29
#define COP0_REG_ERROREPC 30

#define FCR_REVISION 0
#define FCR_CONTROL  31

#define NOT_IMPLEMENTED()                                                                                \
    {                                                                                                    \
        std::printf("%s:%d: PC: 0x%016llx Not implemented: OP:%02o RS:%02o RT:%02o RD:%02o %02o %02o\n", \
                    __FILE__,                                                                            \
                    __LINE__,                                                                            \
                    _pc,                                                                                 \
                    (op >> 26),                                                                          \
                    ((op >> 21) & 0x1f),                                                                 \
                    ((op >> 16) & 0x1f),                                                                 \
                    ((op >> 11) & 0x1f),                                                                 \
                    ((op >> 06) & 0x1f),                                                                 \
                    ((op >> 00) & 0x3f));                                                                \
        exit(1);                                                                                         \
    }

#define RS          ((std::uint8_t)((op >> 21) & 0x1f))
#define BASE        RS
#define FMT         RS
#define RT          ((std::uint8_t)((op >> 16) & 0x1f))
#define FT          RT
#define RD          ((std::uint8_t)((op >> 11) & 0x1f))
#define FS          RD
#define SA          ((std::uint8_t)((op >> 6) & 0x1f))
#define FD          SA
#define IMM         ((std::uint16_t)op)
#define SIMM        ((std::int16_t)op)
#define JUMP_TARGET ((std::uint32_t)op & 0x3ffffff)

#define FMT_S (16 << 6)
#define FMT_D (17 << 6)
#define FMT_W (20 << 6)
#define FMT_L (21 << 6)

#define INT_TIMER 0x80

using namespace libnin64;

static std::uint32_t StoW(float v)
{
    return (((std::uint32_t)std::fabs(v)) & 0x7fffffff) | (v < 0.f ? 0x80000000 : 0x00000000);
}

static std::uint64_t StoL(float v)
{
    return (((std::uint64_t)std::fabs(v)) & 0x7fffffffffffffffull) | (v < 0.f ? 0x8000000000000000ull : 0x0000000000000000ull);
}

static std::uint32_t DtoW(double v)
{
    return (((std::uint32_t)std::abs(v)) & 0x7fffffff) | (v < 0.0 ? 0x80000000 : 0x00000000);
}

static std::uint64_t DtoL(double v)
{
    return (((std::uint64_t)std::abs(v)) & 0x7fffffffffffffffull) | (v < 0.0 ? 0x8000000000000000ull : 0x0000000000000000ull);
}

static float WtoS(std::uint32_t v)
{
    return (float)(v & 0x7fffffff) * ((v & 0x80000000) ? -1.f : 1.f);
}

static float LtoS(std::uint64_t v)
{
    return (float)(v & 0x7fffffffffffffffull) * ((v & 0x8000000000000000ull) ? -1.f : 1.f);
}

static double WtoD(std::uint32_t v)
{
    return (double)(v & 0x7fffffff) * ((v & 0x80000000) ? -1.0 : 1.0);
}

static double LtoD(std::uint64_t v)
{
    return (double)(v & 0x7fffffffffffffffull) * ((v & 0x8000000000000000ull) ? -1.0 : 1.0);
}

CPU::CPU(Bus& bus, MIPSInterface& mi)
: _bus{bus}
, _mi{mi}
, _pc{0xffffffffa4000040ull}
, _pcNext{_pc + 4}
, _regs{}
, _fpuRegs{}
, _llAddr{}
, _epc{}
, _errorEpc{}
, _ip{}
, _im{}
, _erl{true}
, _exl{}
, _ie{}
, _llBit{}
, _fpCompare{}
, _branchDelay{}
, _bd{}
, _fr{}
, _count{}
, _compare{}
{
    _regs[0].u64  = 0;
    _regs[1].u64  = 0x1;
    _regs[2].u64  = 0x0ebda536;
    _regs[3].u64  = 0x0ebda536;
    _regs[4].u64  = 0x0000a536;
    _regs[5].u64  = 0;
    _regs[6].u64  = 0xffffffffa4001f0c;
    _regs[7].u64  = 0xffffffffa4001f08;
    _regs[8].u64  = 0x00000000000000C0;
    _regs[9].u64  = 0x0000000000000000;
    _regs[10].u64 = 0x0000000000000040;
    _regs[11].u64 = 0xffffffffa4000040;
    _regs[12].u64 = 0xffffffffed10d0b3;
    _regs[13].u64 = 0x000000001402a4cc;
    _regs[14].u64 = 0x0;
    _regs[15].u64 = 0x3103e121;
    _regs[16].u64 = 0x0000000000000000;
    _regs[17].u64 = 0x0000000000000000;
    _regs[18].u64 = 0x0000000000000000;
    _regs[19].u64 = 0x0000000000000000;
    _regs[20].u64 = 0x1;
    _regs[21].u64 = 0x0;
    _regs[22].u64 = 0x3f;
    _regs[23].u64 = 0x0;
    _regs[24].u64 = 0x3;
    _regs[25].u64 = 0xffffffff9debb54f;
    _regs[26].u64 = 0x0;
    _regs[27].u64 = 0x0;
    _regs[28].u64 = 0x0;
    _regs[29].u64 = 0xa4001ff0;
    _regs[30].u64 = 0x0;
    _regs[31].u64 = 0xffffffffa4001550;
}

CPU::~CPU()
{
}

void CPU::init(CIC cic)
{
    switch (cic)
    {
    case CIC::Unknown:
    case CIC::NUS_6102:
        _regs[1].u64  = 0x0000000000000001;
        _regs[2].u64  = 0x000000000ebda536;
        _regs[3].u64  = 0x000000000ebda536;
        _regs[4].u64  = 0x000000000000a536;
        _regs[5].u64  = 0xffffffffc95973d5;
        _regs[12].u64 = 0xffffffffed10d0b3;
        _regs[13].u64 = 0x000000001402a4cc;
        _regs[14].u64 = 0x000000002449a366;
        _regs[15].u64 = 0x000000003103e121;
        _regs[22].u64 = 0x000000000000003f;
        _regs[25].u64 = 0xffffffff9debb54f;
        break;
    case CIC::NUS_6103:
        _regs[1].u64  = 0x0000000000000001;
        _regs[2].u64  = 0x0000000049a5ee96;
        _regs[3].u64  = 0x0000000049a5ee96;
        _regs[4].u64  = 0x000000000000ee96;
        _regs[5].u64  = 0xffffffff95315a28;
        _regs[12].u64 = 0xffffffffce9dfbf7;
        _regs[13].u64 = 0xffffffffce9dfbf7;
        _regs[14].u64 = 0x000000005baca1df;
        _regs[15].u64 = 0x0000000018b63d28;
        _regs[22].u64 = 0x0000000000000078;
        _regs[25].u64 = 0xffffffff825b21c9;
    case CIC::NUS_6105:
        _regs[1].u64  = 0x0000000000000000;
        _regs[2].u64  = 0xfffffffff58b0fbf;
        _regs[3].u64  = 0xfffffffff58b0fbf;
        _regs[4].u64  = 0x0000000000000fbf;
        _regs[5].u64  = 0x000000005493fb9a;
        _regs[12].u64 = 0xffffffff9651f81e;
        _regs[13].u64 = 0x000000002d42aac5;
        _regs[14].u64 = 0xffffffffc2c20384;
        _regs[15].u64 = 0x0000000056584d60;
        _regs[22].u64 = 0x0000000000000091;
        _regs[25].u64 = 0xffffffffcdce565f;
        break;
    }
}

void CPU::tick(std::size_t count)
{
    while (count--)
    {
        tick();
    }
    //std::printf("PC: %016llx\n", _pc);
}

void CPU::tick()
{
    std::uint32_t op;
    std::uint64_t tmp;
    std::uint64_t tmp2;

    if (_ie && !_erl && !_exl && (_im & (_ip | _mi.ip())))
    {
        _bd          = _branchDelay;
        _branchDelay = false;
        _exl         = true;
        _epc         = (std::uint32_t)_pc - (_bd ? 4 : 0);
        _pc          = 0xffffffff80000180ull;
        _pcNext      = _pc + 4;
    }

    // For current tick
    op = _bus.read32((std::uint32_t)_pc);
    //std::printf("PC: 0x%016llx OP: 0x%08x Details:%02o %02o %02o %02o %02o %02o\n", _pc, op, (op >> 26), ((op >> 21) & 0x1f), ((op >> 16) & 0x1f), ((op >> 11) & 0x1f), ((op >> 06) & 0x1f), ((op >> 00) & 0x3f));
    //for (int i = 0; i < 32; ++i) { std::printf("  REG %02d: 0x%016llx\n", i, _regs[i].u64); }

    // For next tick
    _pc = _pcNext;
    _pcNext += 4;
    _branchDelay = false;

    switch (op >> 26)
    {
    case 000: // SPECIAL
        switch (op & 0x3f)
        {
        case 000: // SLL (Shift Left Logical)
            _regs[RD].i64 = (std::int32_t)((_regs[RT].u32 << SA) & 0xffffffff);
            break;
        case 002: // SRL (Shift Right Logical)
            _regs[RD].i64 = (std::int32_t)((_regs[RT].u32 >> SA) & 0xffffffff);
            break;
        case 003: // SRA (Shift Right Arithmetic)
            _regs[RD].i64 = (std::int32_t)((_regs[RT].i32 >> SA) & 0xffffffff);
            break;
        case 004: // SLLV (Shift Left Logical Variable)
            _regs[RD].i64 = (std::int32_t)((_regs[RT].u32 << (_regs[RS].u8 & 0x1f)) & 0xffffffff);
            break;
        case 006: // SRLV (Shift Right Logical Variable)
            _regs[RD].i64 = (std::int32_t)((_regs[RT].u32 >> (_regs[RS].u8 & 0x1f)) & 0xffffffff);
            break;
        case 007: // SRAV (Shift Right Arithmetic Variable)
            _regs[RD].i64 = (std::int32_t)((_regs[RT].i32 >> (_regs[RS].u8 & 0x1f)) & 0xffffffff);
            break;
        case 010: // JR (Jump Register)
            _pcNext = _regs[RS].u64;
            break;
        case 011: // JALR (Jump And Link Register)
            _pcNext       = _regs[RS].u64;
            _regs[RD].u64 = _pc + 4;
            break;
        case 014: // SYSCALL (System Call)
            NOT_IMPLEMENTED();
            break;
        case 015: // BREAK (Breakpoint)
            NOT_IMPLEMENTED();
            break;
        case 017: // SYNC (Synchronize)
            NOT_IMPLEMENTED();
            break;
        case 020: // MFHI (Move From HI)
            _regs[RD].u64 = _hi.u64;
            break;
        case 021: // MTHI (Move To HI)
            _hi.u64 = _regs[RS].u64;
            break;
        case 022: // MFLO (Move From LO)
            _regs[RD].u64 = _lo.u64;
            break;
        case 023: // MTLO (Move To LO)
            _lo.u64 = _regs[RS].u64;
            break;
        case 024: // DSLLV (Doubleword Shift Left Logical Variable)
            NOT_IMPLEMENTED();
            break;
        case 026: // DSRLV (Doubleword Shift Right Logical Variable)
            NOT_IMPLEMENTED();
            break;
        case 027: // DSRAV
            NOT_IMPLEMENTED();
            break;
        case 030: // MULT (Multiply)
            tmp     = (std::int64_t)_regs[RS].i32 * _regs[RT].i32;
            _lo.i64 = (std::int32_t)(tmp & 0xffffffff);
            _hi.i64 = (std::int32_t)((tmp >> 32) & 0xffffffff);
            break;
        case 031: // MULTU (Multiply Unsigned)
            tmp     = (std::uint64_t)_regs[RS].u32 * _regs[RT].u32;
            _lo.i64 = (std::int32_t)(tmp & 0xffffffff);
            _hi.i64 = (std::int32_t)((tmp >> 32) & 0xffffffff);
            break;
        case 032: // DIV
            _lo.i64 = _regs[RS].i32 / _regs[RT].i32;
            _hi.i64 = _regs[RS].i32 % _regs[RT].i32;
            break;
        case 033: // DIVU
            _lo.u64 = _regs[RS].u32 / _regs[RT].u32;
            _hi.u64 = _regs[RS].u32 % _regs[RT].u32;
            break;
        case 034: // DMULT (Doubleword Multiply)
            mul128(_regs[RS].i64, _regs[RT].i64, &_lo.i64, &_hi.i64);
            break;
        case 035: // DMULTU (Doubleword Multiply Unsigned)
            umul128(_regs[RS].u64, _regs[RT].u64, &_lo.u64, &_hi.u64);
            break;
        case 036: // DDIV (Doubleword Divide)
            _lo.i64 = _regs[RS].i64 / _regs[RT].i64;
            _hi.i64 = _regs[RS].i64 % _regs[RT].i64;
            break;
        case 037: // DDIVU (Doubleword Divide Unsigned)
            _lo.u64 = _regs[RS].u64 / _regs[RT].u64;
            _hi.u64 = _regs[RS].u64 % _regs[RT].u64;
            break;
        case 040: // ADD (Add)
            _regs[RD].i64 = (std::int32_t)(_regs[RS].i64 + _regs[RT].i64);
            break;
        case 041: // ADDU (Add Unsigned)
            _regs[RD].i64 = (std::int32_t)(_regs[RS].i64 + _regs[RT].i64);
            break;
        case 042: // SUB (Subtract)
            _regs[RD].i64 = (std::int32_t)(_regs[RS].i64 - _regs[RT].i64);
            break;
        case 043: // SUBU (Subtract Unsigned)
            _regs[RD].i64 = (std::int32_t)(_regs[RS].i64 - _regs[RT].i64);
            break;
        case 044: // AND
            _regs[RD].u64 = _regs[RS].u64 & _regs[RT].u64;
            break;
        case 045: // OR
            _regs[RD].u64 = _regs[RS].u64 | _regs[RT].u64;
            break;
        case 046: // XOR
            _regs[RD].u64 = _regs[RS].u64 ^ _regs[RT].u64;
            break;
        case 047: // NOR
            _regs[RD].u64 = ~(_regs[RS].u64 | _regs[RT].u64);
            break;
        case 052: // SLT (Set On Less Than)
            _regs[RD].u64 = !!(_regs[RS].i64 < _regs[RT].i64);
            break;
        case 053: // SLTU (Set On Less Than Unsigned)
            _regs[RD].u64 = !!(_regs[RS].u64 < _regs[RT].u64);
            break;
        case 054: // DADD
            _regs[RD].i64 = _regs[RS].i64 + _regs[RT].i64;
            break;
        case 055: // DADDU
            _regs[RD].i64 = _regs[RS].i64 + _regs[RT].i64;
            break;
        case 056: // DSUB
            _regs[RD].i64 = _regs[RS].i64 - _regs[RT].i64;
            break;
        case 057: // DSUBU
            _regs[RD].i64 = _regs[RS].i64 - _regs[RT].i64;
            break;
        case 060: // TGE
            NOT_IMPLEMENTED();
            break;
        case 061: // TGEU
            NOT_IMPLEMENTED();
            break;
        case 062: // TLT
            NOT_IMPLEMENTED();
            break;
        case 063: // TLTU
            NOT_IMPLEMENTED();
            break;
        case 064: // TEQ
            NOT_IMPLEMENTED();
            break;
        case 066: // TNE
            NOT_IMPLEMENTED();
            break;
        case 070: // DSLL (Doubleword Shift Left Logical)
            _regs[RD].u64 = _regs[RT].u64 << SA;
            break;
        case 072: // DSRL (Doubleword Shift Right Logical)
            _regs[RD].u64 = _regs[RT].u64 >> SA;
            break;
        case 073: // DSRA (Doubleword Shift Right Arithmetic)
            _regs[RD].i64 = _regs[RT].i64 >> SA;
            break;
        case 074: // DSLL32 (Doubleword Shift Left Logical + 32)
            _regs[RD].u64 = _regs[RT].u64 << (32 + SA);
            break;
        case 076: // DSRL32 (Doubleword Shift Right Logical + 32)
            _regs[RD].u64 = _regs[RT].u64 >> (32 + SA);
            break;
        case 077: // DSRA32 (Doubleword Shift Right Arithmetic + 32)
            _regs[RD].i64 = _regs[RT].i64 >> (32 + SA);
            break;
        default:
            NOT_IMPLEMENTED();
            break;
        }
        break;
    case 001: // REGIMM
        switch (RT)
        {
        case 000: // BLTZ
            if (_regs[RS].i64 < 0)
            {
                _pcNext      = _pc + ((std::int64_t)SIMM << 2);
                _branchDelay = true;
            }
            break;
        case 001: // BGEZ
            if (_regs[RS].i64 >= 0)
            {
                _pcNext      = _pc + ((std::int64_t)SIMM << 2);
                _branchDelay = true;
            }
            break;
        case 002: // BLTZL
            if (_regs[RS].i64 < 0)
            {
                _pcNext      = _pc + ((std::int64_t)SIMM << 2);
                _branchDelay = true;
            }
            else
            {
                _pc = _pcNext;
                _pcNext += 4;
            }
            break;
        case 003: // BGEZL (Branch On Greater Than Or Equal To Zero Likely)
            if (_regs[RS].i64 >= 0)
            {
                _pcNext      = _pc + ((std::int64_t)SIMM << 2);
                _branchDelay = true;
            }
            else
            {
                _pc = _pcNext;
                _pcNext += 4;
            }
            break;
        case 010: // TGEI
            NOT_IMPLEMENTED();
            break;
        case 011: // TGEIU
            NOT_IMPLEMENTED();
            break;
        case 012: // TLTI
            NOT_IMPLEMENTED();
            break;
        case 013: // TLTIU
            NOT_IMPLEMENTED();
            break;
        case 014: // TEQI
            NOT_IMPLEMENTED();
            break;
        case 016: // TNEI
            NOT_IMPLEMENTED();
            break;
        case 020: // BLTZAL (Branch On Less Than Zero And Link Likely)
            _regs[31].u64 = _pc + 4;
            if (_regs[RS].i64 < 0)
            {
                _pcNext      = _pc + ((std::int64_t)SIMM << 2);
                _branchDelay = true;
            }
            break;
        case 021: // BGEZAL (Branch On Greater Than Or Equal To Zero And Link Likely)
            _regs[31].u64 = _pc + 4;
            if (_regs[RS].i64 >= 0)
            {
                _pcNext      = _pc + ((std::int64_t)SIMM << 2);
                _branchDelay = true;
            }
            break;
        case 022: // BLTZALL
            _regs[31].u64 = _pc + 4;
            if (_regs[RS].i64 < 0)
            {
                _pcNext      = _pc + ((std::int64_t)SIMM << 2);
                _branchDelay = true;
            }
            else
            {
                _pc = _pcNext;
                _pcNext += 4;
            }
            break;
        case 023: // BGEZALL
            _regs[31].u64 = _pc + 4;
            if (_regs[RS].i64 >= 0)
            {
                _pcNext      = _pc + ((std::int64_t)SIMM << 2);
                _branchDelay = true;
            }
            else
            {
                _pc = _pcNext;
                _pcNext += 4;
            }
            break;
        default:
            NOT_IMPLEMENTED();
            break;
        }
        break;
    case 002: // J (Jump)
        _pcNext      = ((std::uint64_t)JUMP_TARGET << 2) | (_pc & 0xfffffffff0000000ULL);
        _branchDelay = true;
        break;
    case 003: // JAL (Jump And Link)
        _pcNext       = ((std::uint64_t)JUMP_TARGET << 2) | (_pc & 0xfffffffff0000000ULL);
        _branchDelay  = true;
        _regs[31].u64 = _pc + 4;
        break;
    case 004: // BEQ
        if (_regs[RS].u64 == _regs[RT].u64)
        {
            _pcNext      = _pc + ((std::int64_t)SIMM << 2);
            _branchDelay = true;
        }
        break;
    case 005: // BNE
        if (_regs[RS].u64 != _regs[RT].u64)
        {
            _pcNext      = _pc + ((std::int64_t)SIMM << 2);
            _branchDelay = true;
        }
        break;
    case 006: // BLEZ
        if (_regs[RS].i64 <= 0)
        {
            _pcNext      = _pc + ((std::int64_t)SIMM << 2);
            _branchDelay = true;
        }
        break;
    case 007: // BGTZ
        if (_regs[RS].i64 > 0)
        {
            _pcNext      = _pc + ((std::int64_t)SIMM << 2);
            _branchDelay = true;
        }
        break;
    case 010: // ADDI (Add Immediate)
        _regs[RT].i64 = (_regs[RS].i32 + SIMM);
        break;
    case 011: // ADDIU (Add Immediate Unsigned)
        _regs[RT].i64 = (_regs[RS].i32 + SIMM);
        break;
    case 012: // SLTI (Set On Less Than Immediate)
        _regs[RT].u64 = (_regs[RS].i64 < SIMM) ? 1 : 0;
        break;
    case 013: // SLTIU (Set On Less Than Immediate Unsigned)
        _regs[RT].u64 = (_regs[RS].u64 < (std::uint64_t)((std::int64_t)SIMM)) ? 1 : 0;
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
    case 017: // LUI (Load Upper Immediate)
        _regs[RT].i64 = ((std::int64_t)SIMM << 16);
        break;
    case 020: // COP0 (Coprocessor 0)
        switch (RS)
        {
        case 000: // MF
            _regs[RT].i64 = (std::int32_t)cop0Read(RD);
            break;
        case 001: // DMF
            _regs[RT].u64 = cop0Read(RD);
            break;
        case 002: // CF
            NOT_IMPLEMENTED();
            break;
        case 004: // MT
            cop0Write(RD, _regs[RT].u32);
            break;
        case 005: // DMT
            cop0Write(RD, _regs[RT].u32);
            break;
        case 006: // CT
            NOT_IMPLEMENTED();
            break;
        case 010: // BC
            NOT_IMPLEMENTED();
            break;
        case 020:
        case 021:
        case 022:
        case 023:
        case 024:
        case 025:
        case 026:
        case 027:
        case 030:
        case 031:
        case 032:
        case 033:
        case 034:
        case 035:
        case 036:
        case 037:
            switch (op & 077)
            {
            case 001: // TLBR
                // To implement when doing TLB
                break;
            case 002: // TLBWI
                // To implement when doing TLB
                break;
            case 006: // TLBWR
                // To implement when doing TLB
                break;
            case 010: // TLBP
                // To implement when doing TLB
                break;
            case 030: // ERET
                if (_erl)
                {
                    _pc  = (std::int64_t)((std::int32_t)_errorEpc);
                    _erl = false;
                    std::printf("ERET: ErrorEPC\n");
                }
                else
                {
                    _pc  = (std::int64_t)((std::int32_t)_epc);
                    _exl = false;
                    std::printf("ERET: EPC\n");
                }
                _llBit  = false;
                _pcNext = _pc + 4;
                break;
            }
            break;
        }
        break;
    case 021: // COP1 (Coprocessor 1)
        switch (RS)
        {
        case 000: // MFC1 (Move Word From FPU)
            _regs[RT].u32 = fpuReadU32(FS);
            break;
        case 001: // DMFC1 (Doubleword Move From FPU)
            _regs[RT].u64 = fpuReadU64(FS);
            break;
        case 002: // CFC1 (Move Control From FPU)
            _regs[RT].i64 = (std::int32_t)fcrRead(FS);
            break;
        case 004: // MTC1 (Move To FPU)
            fpuWriteU32(FS, _regs[RT].u32);
            break;
        case 005: // DMTC1 (Doubleword Move To FPU)
            fpuWriteU64(FS, _regs[RT].u64);
            break;
        case 006: // CTC1 (Move Control Word To FPU)
            fcrWrite(FS, _regs[RT].u32);
            break;
        case 010: // BC
            switch (RT)
            {
            case 000: // BCF
                if (!_fpCompare)
                {
                    _pcNext      = _pc + ((std::int64_t)SIMM << 2);
                    _branchDelay = true;
                }
                break;
            case 001: // BCT
                if (_fpCompare)
                {
                    _pcNext      = _pc + ((std::int64_t)SIMM << 2);
                    _branchDelay = true;
                }
                break;
            case 002: // BCFL
                if (!_fpCompare)
                {
                    _pcNext      = _pc + ((std::int64_t)SIMM << 2);
                    _branchDelay = true;
                }
                else
                {
                    _pc = _pcNext;
                    _pcNext += 4;
                }
                break;
            case 003: // BCTL
                if (_fpCompare)
                {
                    _pcNext      = _pc + ((std::int64_t)SIMM << 2);
                    _branchDelay = true;
                }
                else
                {
                    _pc = _pcNext;
                    _pcNext += 4;
                }
                break;
            default:
                NOT_IMPLEMENTED();
                break;
            }
            break;
        case 020:
        case 021:
        case 022:
        case 023:
        case 024:
        case 025:
        case 026:
        case 027:
        case 030:
        case 031:
        case 032:
        case 033:
        case 034:
        case 035:
        case 036:
        case 037: // CO
            /*
             * FMT values:
             * 0x10
             */
            switch (((std::uint16_t)FMT << 6) | (op & 0x77))
            {
            // 00: ADD.fmt
            case FMT_S | 000: // ADD.S
                fpuWriteF32(FD, fpuReadF32(FS) + fpuReadF32(FT));
                break;
            case FMT_D | 000: // ADD.D
                fpuWriteF64(FD, fpuReadF64(FS) + fpuReadF64(FT));
                break;

            // 01: SUB.fmt
            case FMT_S | 001: // SUB.S
                fpuWriteF32(FD, fpuReadF32(FS) - fpuReadF32(FT));
                break;
            case FMT_D | 001: // SUB.D
                fpuWriteF64(FD, fpuReadF64(FS) - fpuReadF64(FT));
                break;

            // 02: MUL.fmt
            case FMT_S | 002: // MUL.S
                fpuWriteF32(FD, fpuReadF32(FS) * fpuReadF32(FT));
                break;
            case FMT_D | 002: // MUL.D
                fpuWriteF64(FD, fpuReadF64(FS) * fpuReadF64(FT));
                break;

            // 03: DIV.fmt
            case FMT_S | 003: // DIV.S
                fpuWriteF32(FD, fpuReadF32(FS) / fpuReadF32(FT));
                break;
            case FMT_D | 003: // DIV.D
                fpuWriteF64(FD, fpuReadF64(FS) / fpuReadF64(FT));
                break;

            // 04: SQRT.fmt
            case FMT_S | 004: // SQRT.S
                fpuWriteF32(FD, std::sqrtf(fpuReadF32(FS)));
                break;
            case FMT_D | 004: // SQRT.D
                fpuWriteF64(FD, std::sqrt(fpuReadF64(FS)));
                break;

            // 05: ABS.fmt
            case FMT_S | 005: // ABS.S
                fpuWriteF32(FD, std::fabs(fpuReadF32(FS)));
                break;
            case FMT_D | 005: // ABS.D
                fpuWriteF64(FD, std::abs(fpuReadF64(FS)));
                break;

            // 06: MOV.fmt
            case FMT_S | 006:
                fpuWriteF32(FD, fpuReadF32(FS));
                break;
            case FMT_D | 006:
                fpuWriteF64(FD, fpuReadF64(FS));
                break;

            // 07: NEG
            case FMT_S | 007:
                fpuWriteF32(FD, -fpuReadF32(FS));
                break;
            case FMT_D | 007:
                fpuWriteF64(FD, -fpuReadF64(FS));
                break;

            // 10: ROUND.L
            case FMT_S | 010:
                fpuWriteU64(FD, StoL(std::roundf(fpuReadF32(FS))));
                break;
            case FMT_D | 010:
                fpuWriteU64(FD, DtoL(std::round(fpuReadF64(FS))));
                break;

            // 11: TRUNC.L
            case FMT_S | 011:
                fpuWriteU64(FD, StoL(std::truncf(fpuReadF32(FS))));
                break;
            case FMT_D | 011:
                fpuWriteU64(FD, DtoL(std::trunc(fpuReadF64(FS))));
                break;

            // 12: CEIL.L
            case FMT_S | 012:
                fpuWriteU64(FD, StoL(std::ceilf(fpuReadF32(FS))));
                break;
            case FMT_D | 012:
                fpuWriteU64(FD, DtoL(std::ceil(fpuReadF64(FS))));
                break;

            // 13: FLOOR.L
            case FMT_S | 013:
                fpuWriteU64(FD, StoL(std::floorf(fpuReadF32(FS))));
                break;
            case FMT_D | 013:
                fpuWriteU64(FD, DtoL(std::floor(fpuReadF64(FS))));
                break;

            // 14: ROUND.W
            case FMT_S | 014:
                fpuWriteU32(FD, StoW(std::roundf(fpuReadF32(FS))));
                break;
            case FMT_D | 014:
                fpuWriteU32(FD, DtoW(std::round(fpuReadF64(FS))));
                break;

            // 15: TRUNC.W
            case FMT_S | 015:
                fpuWriteU32(FD, StoW(std::truncf(fpuReadF32(FS))));
                break;
            case FMT_D | 015:
                fpuWriteU32(FD, DtoW(std::trunc(fpuReadF64(FS))));
                break;

            // 16: CEIL.W
            case FMT_S | 016:
                fpuWriteU32(FD, StoW(std::ceilf(fpuReadF32(FS))));
                break;
            case FMT_D | 016:
                fpuWriteU32(FD, DtoW(std::ceil(fpuReadF64(FS))));
                break;

            // 17: FLOOR.W
            case FMT_S | 017:
                fpuWriteU32(FD, StoW(std::floorf(fpuReadF32(FS))));
                break;
            case FMT_D | 017:
                fpuWriteU32(FD, DtoW(std::floor(fpuReadF64(FS))));
                break;

            // 40: CVT.S
            case FMT_D | 040:
                fpuWriteF32(FD, (float)fpuReadF64(FS));
                break;
            case FMT_W | 040:
                fpuWriteF32(FD, WtoS(fpuReadU32(FS)));
                break;
            case FMT_L | 040:
                fpuWriteF32(FD, LtoS(fpuReadU64(FS)));
                break;

            // 41: CVT.D
            case FMT_S | 041:
                fpuWriteF64(FD, (double)fpuReadF32(FS));
                break;
            case FMT_W | 041:
                fpuWriteF64(FD, WtoD(fpuReadU32(FS)));
                break;
            case FMT_L | 041:
                fpuWriteF64(FD, LtoD(fpuReadU64(FS)));
                break;

            // 44: CVT.W
            case FMT_S | 044:
                fpuWriteU32(FD, StoW(fpuReadF32(FS)));
                break;
            case FMT_D | 044:
                fpuWriteU32(FD, DtoW(fpuReadF64(FS)));
                break;

            // 45: CVT.L
            case FMT_S | 045:
                fpuWriteU64(FD, StoL(fpuReadF32(FS)));
                break;
            case FMT_D | 045:
                fpuWriteU64(FD, DtoL(fpuReadF64(FS)));
                break;

            // C.cond.fmt (Floating-point Compare)

            // 60: C.F
            case FMT_S | 060:
                _fpCompare = false;
                break;
            case FMT_D | 060:
                _fpCompare = false;
                break;

            // 61: C.UN
            case FMT_S | 061:
                _fpCompare = (std::isnan(fpuReadF32(FS)) || std::isnan(fpuReadF32(FT)));
                break;
            case FMT_D | 061:
                _fpCompare = (std::isnan(fpuReadF64(FS)) || std::isnan(fpuReadF64(FT)));
                break;

            // 62: C.EQ
            case FMT_S | 062:
                _fpCompare = (fpuReadF32(FS) == fpuReadF32(FT));
                break;
            case FMT_D | 062:
                _fpCompare = (fpuReadF64(FS) == fpuReadF64(FT));
                break;

            // 63: C.UEQ
            case FMT_S | 063:
                _fpCompare = (std::isnan(fpuReadF32(FS)) || std::isnan(fpuReadF32(FT)) || (fpuReadF32(FS) == fpuReadF32(FT)));
                break;
            case FMT_D | 063:
                _fpCompare = (std::isnan(fpuReadF64(FS)) || std::isnan(fpuReadF64(FT)) || (fpuReadF64(FS) == fpuReadF64(FT)));
                break;

            // 64: C.OLT
            case FMT_S | 064:
                _fpCompare = (fpuReadF32(FS) < fpuReadF32(FT));
                break;
            case FMT_D | 064:
                _fpCompare = (fpuReadF64(FS) < fpuReadF64(FT));
                break;

            // 65: C.ULT
            case FMT_S | 065:
                _fpCompare = (std::isnan(fpuReadF32(FS)) || std::isnan(fpuReadF32(FT)) || (fpuReadF32(FS) < fpuReadF32(FT)));
                break;
            case FMT_D | 065:
                _fpCompare = (std::isnan(fpuReadF64(FS)) || std::isnan(fpuReadF64(FT)) || (fpuReadF64(FS) < fpuReadF64(FT)));
                break;

            // 66: C.OLE
            case FMT_S | 066:
                _fpCompare = (fpuReadF32(FS) <= fpuReadF32(FT));
                break;
            case FMT_D | 066:
                _fpCompare = (fpuReadF64(FS) <= fpuReadF64(FT));
                break;

            // 67: C.ULE
            case FMT_S | 067:
                _fpCompare = (std::isnan(fpuReadF32(FS)) || std::isnan(fpuReadF32(FT)) || (fpuReadF32(FS) <= fpuReadF32(FT)));
                break;
            case FMT_D | 067:
                _fpCompare = (std::isnan(fpuReadF64(FS)) || std::isnan(fpuReadF64(FT)) || (fpuReadF64(FS) <= fpuReadF64(FT)));
                break;

            // 70: C.SF
            case FMT_S | 070:
                // TODO: Signals
                _fpCompare = false;
                break;
            case FMT_D | 070:
                // TODO: Signals
                _fpCompare = false;
                break;

            // 71: C.NGLE
            case FMT_S | 071:
                // TODO: Signals
                _fpCompare = (std::isnan(fpuReadF32(FS)) || std::isnan(fpuReadF32(FT)));
                break;
            case FMT_D | 071:
                // TODO: Signals
                _fpCompare = (std::isnan(fpuReadF64(FS)) || std::isnan(fpuReadF64(FT)));
                break;

            // 72: C.SEQ
            case FMT_S | 072:
                // TODO: Signals
                _fpCompare = (fpuReadF32(FS) == fpuReadF32(FT));
                break;
            case FMT_D | 072:
                // TODO: Signals
                _fpCompare = (fpuReadF64(FS) == fpuReadF64(FT));
                break;

            // 73: C.NGL
            case FMT_S | 073:
                // TODO: Signals
                _fpCompare = (std::isnan(fpuReadF32(FS)) || std::isnan(fpuReadF32(FT)) || (fpuReadF32(FS) == fpuReadF32(FT)));
                break;
            case FMT_D | 073:
                // TODO: Signals
                _fpCompare = (std::isnan(fpuReadF64(FS)) || std::isnan(fpuReadF64(FT)) || (fpuReadF64(FS) == fpuReadF64(FT)));
                break;

            // 74: C.LT
            case FMT_S | 074:
                // TODO: Signals
                _fpCompare = (fpuReadF32(FS) < fpuReadF32(FT));
                break;
            case FMT_D | 074:
                // TODO: Signals
                _fpCompare = (fpuReadF64(FS) <= fpuReadF64(FT));
                break;

            // 75: C.NGE
            case FMT_S | 075:
                // TODO: Signals
                _fpCompare = (std::isnan(fpuReadF32(FS)) || std::isnan(fpuReadF32(FT)) || (fpuReadF32(FS) < fpuReadF32(FT)));
                break;
            case FMT_D | 075:
                // TODO: Signals
                _fpCompare = (std::isnan(fpuReadF64(FS)) || std::isnan(fpuReadF64(FT)) || (fpuReadF64(FS) < fpuReadF64(FT)));
                break;

            // 76: C.LE
            case FMT_S | 076:
                // TODO: Signals
                _fpCompare = (fpuReadF32(FS) <= fpuReadF32(FT));
                break;
            case FMT_D | 076:
                // TODO: Signals
                _fpCompare = (fpuReadF64(FS) <= fpuReadF64(FT));
                break;

            // 77: C.NGT
            case FMT_S | 077:
                // TODO: Signals
                _fpCompare = (std::isnan(fpuReadF32(FS)) || std::isnan(fpuReadF32(FT)) || (fpuReadF32(FS) <= fpuReadF32(FT)));
                break;
            case FMT_D | 077:
                // TODO: Signals
                _fpCompare = (std::isnan(fpuReadF64(FS)) || std::isnan(fpuReadF64(FT)) || (fpuReadF64(FS) <= fpuReadF64(FT)));
                break;

            default:
                NOT_IMPLEMENTED();
                break;
            }
            break;
        default:
            NOT_IMPLEMENTED();
            break;
        }
        break;
    case 022: // COP2 (Coprocessor 2)
        NOT_IMPLEMENTED();
        break;
    case 024: // BEQL
        if (_regs[RS].u64 == _regs[RT].u64)
        {
            _pcNext      = _pc + ((std::int64_t)SIMM << 2);
            _branchDelay = true;
        }
        else
        {
            _pc = _pcNext;
            _pcNext += 4;
        }
        break;
    case 025: // BNEL
        if (_regs[RS].u64 != _regs[RT].u64)
        {
            _pcNext      = _pc + ((std::int64_t)SIMM << 2);
            _branchDelay = true;
        }
        else
        {
            _pc = _pcNext;
            _pcNext += 4;
        }
        break;
    case 026: // BLEZL
        if (_regs[RS].i64 <= 0)
        {
            _pcNext      = _pc + ((std::int64_t)SIMM << 2);
            _branchDelay = true;
        }
        else
        {
            _pc = _pcNext;
            _pcNext += 4;
        }
        break;
    case 027: // BGTZL (Branch On Greater Than Zero Likely)
        if (_regs[RS].i64 > 0)
        {
            _pcNext      = _pc + ((std::int64_t)SIMM << 2);
            _branchDelay = true;
        }
        else
        {
            _pc = _pcNext;
            _pcNext += 4;
        }
        break;
    case 030: // DADDI (Doubleword Add Immediate)
        _regs[RT].i64 = _regs[RS].i64 + SIMM;
        break;
    case 031: // DADDIU (Doubleword Add Immediate Unsigned)
        _regs[RT].i64 = _regs[RS].i64 + SIMM;
        break;
    case 032: // LDL (Load Doubleword Left)
        NOT_IMPLEMENTED();
        break;
    case 033: // LDR (Load Doubleword Right)
        std::printf("LDR\n");
        tmp  = _regs[RS].u64 + SIMM;
        tmp2 = _bus.read64(tmp & 0xfffffff8); // Mask the offset
        switch (tmp & 0x7)
        {
        case 0x0:
            tmp = (tmp2 >> 56) | (_regs[RT].u64 & 0xffffffffffffff00ull);
            break;
        case 0x1:
            tmp = (tmp2 >> 48) | (_regs[RT].u64 & 0xffffffffffff0000ull);
            break;
        case 0x2:
            tmp = (tmp2 >> 40) | (_regs[RT].u64 & 0xffffffffff000000ull);
            break;
        case 0x3:
            tmp = (tmp2 >> 32) | (_regs[RT].u64 & 0xffffffff00000000ull);
            break;
        case 0x4:
            tmp = (tmp2 >> 24) | (_regs[RT].u64 & 0xffffff0000000000ull);
            break;
        case 0x5:
            tmp = (tmp2 >> 16) | (_regs[RT].u64 & 0xffff000000000000ull);
            break;
        case 0x6:
            tmp = (tmp2 >> 8) | (_regs[RT].u64 & 0xff00000000000000ull);
            break;
        case 0x7:
            tmp = tmp2;
            break;
        }
        _regs[RT].u64 = tmp;
        break;
    case 040: // LB (Load Byte)
        _regs[RT].u64 = sext(_bus.read8(_regs[RS].u32 + SIMM));
        break;
    case 041: // LH (Load Halfword)
        _regs[RT].u64 = sext(_bus.read16(_regs[RS].u32 + SIMM));
        break;
    case 042: // LWL (Load Word Left)
        tmp = _regs[RS].u32 + SIMM;
        std::printf("LWL 0x%08x (0x%016llx)\n", (uint32_t)tmp, _pc);
        tmp2 = _bus.read32(tmp & ~0x3);

        switch (tmp & 0x3)
        {
        case 0x00:
            _regs[RT].u64 = sext(tmp2);
            break;
        case 0x01:
            _regs[RT].u64 = sext((_regs[RT].u32 & 0x000000ff) | ((tmp2 & 0x00ffffff) << 8));
            break;
        case 0x02:
            _regs[RT].u64 = sext((_regs[RT].u32 & 0x0000ffff) | ((tmp2 & 0x0000ffff) << 16));
            break;
        case 0x03:
            _regs[RT].u64 = sext((_regs[RT].u32 & 0x00ffffff) | ((tmp2 & 0x000000ff) << 24));
            break;
        }
        break;
    case 043: // LW (Load Word)
        _regs[RT].u64 = sext(_bus.read32(_regs[RS].u32 + SIMM));
        break;
    case 044: // LBU (Load Byte Unsigned)
        _regs[RT].u64 = _bus.read8(_regs[RS].u32 + SIMM);
        break;
    case 045: // LHU (Load Halfword Unsigned)
        _regs[RT].u64 = _bus.read16(_regs[RS].u32 + SIMM);
        break;
    case 046: // LWR
        tmp = _regs[RS].u32 + SIMM;
        std::printf("LWR 0x%08x (0x%016llx)\n", (uint32_t)tmp, _pc);
        tmp2 = _bus.read32(tmp & ~0x3);

        switch (tmp & 0x3)
        {
        case 0x00:
            _regs[RT].u64 = sext((tmp2 >> 24) | (_regs[RT].u32 & 0xffffff00));
            break;
        case 0x01:
            _regs[RT].u64 = sext((tmp2 >> 16) | (_regs[RT].u32 & 0xffff0000));
            break;
        case 0x02:
            _regs[RT].u64 = sext((tmp2 >> 8) | (_regs[RT].u32 & 0xff000000));
            break;
        case 0x03:
            _regs[RT].u64 = sext(tmp2);
            break;
        }
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
        tmp = _regs[RS].u32 + SIMM;
        std::printf("SWL 0x%08x (0x%016llx)\n", (uint32_t)tmp, _pc);
        tmp2 = _bus.read32(tmp & ~0x3);
        switch (tmp & 0x3)
        {
        case 0x00:
            _bus.write32(tmp & ~0x3, _regs[RT].u32);
            break;
        case 0x01:
            _bus.write32(tmp & ~0x3, (_regs[RT].u32 >> 8) | (tmp2 & 0xff000000));
            break;
        case 0x02:
            _bus.write32(tmp & ~0x3, (_regs[RT].u32 >> 16) | (tmp2 & 0xffff0000));
            break;
        case 0x03:
            _bus.write32(tmp & ~0x3, (_regs[RT].u32 >> 24) | (tmp2 & 0xffffff00));
            break;
        }
        break;
    case 053: // SW (Store Word)
        _bus.write32(_regs[RS].u32 + SIMM, _regs[RT].u32);
        break;
    case 054: // SDL (Store Doubleword Left)
        NOT_IMPLEMENTED();
        break;
    case 055: // SDR (Store Doubleword Right)
        NOT_IMPLEMENTED();
        break;
    case 056: // SWR (Store Word Right)
        tmp = _regs[RS].u32 + SIMM;
        std::printf("SWR 0x%08x (0x%016llx)\n", (uint32_t)tmp, _pc);
        tmp2 = _bus.read32(tmp & ~0x3);
        switch (tmp & 0x3)
        {
        case 0x00:
            _bus.write32(tmp & ~0x3, ((_regs[RT].u32 & 0xff) << 24) | (tmp2 & 0x00ffffff));
            break;
        case 0x01:
            _bus.write32(tmp & ~0x3, ((_regs[RT].u32 & 0xffff) << 16) | (tmp2 & 0x0000ffff));
            break;
        case 0x02:
            _bus.write32(tmp & ~0x3, ((_regs[RT].u32 & 0xffffff) << 8) | (tmp2 & 0x000000ff));
            break;
        case 0x03:
            _bus.write32(tmp & ~0x3, _regs[RT].u32);
            break;
        }
        break;
    case 057: // CACHE
        break;
    case 060: // LL (Load Linked)
        tmp           = (_regs[RS].u32 + SIMM);
        _regs[RT].i64 = (std::int32_t)_bus.read32((std::uint32_t)tmp);
        _llAddr       = (std::uint32_t)tmp;
        _llBit        = true;
        break;
    case 061: // LWC1 (Load Word to FPU)
        _fpuRegs[FT].u32 = _bus.read32(_regs[BASE].u32 + SIMM);
        break;
    case 062: // LWC2
        NOT_IMPLEMENTED();
        break;
    case 064: // LLD
        NOT_IMPLEMENTED();
        break;
    case 065: // LDC1 (Load Doubleword to FPU)
        _fpuRegs[FT].u64 = _bus.read64(_regs[BASE].u32 + SIMM);
        break;
    case 066: // LDC2
        NOT_IMPLEMENTED();
        break;
    case 067: // LD (Load Doubleword)
        _regs[RT].u64 = _bus.read64(_regs[RS].u32 + SIMM);
        break;
    case 070: // SC (Store Conditional)
        NOT_IMPLEMENTED();
        break;
    case 071: // SWC1 (Store Word From FPU)
        _bus.write32(_regs[BASE].u32 + SIMM, _fpuRegs[FT].u32);
        break;
    case 072: // SWC2
        NOT_IMPLEMENTED();
        break;
    case 074: // SCD (Store Conditional Doubleword)
        NOT_IMPLEMENTED();
        break;
    case 075: // SDC1 (Store Doubleword From FPU)
        _bus.write64(_regs[BASE].u32 + SIMM, _fpuRegs[FT].u64);
        break;
    case 076: // SDC2 (Store Doubleword From Coprocessor 2)
        NOT_IMPLEMENTED();
        break;
    case 077: // SD (Store Doubleword)
        _bus.write64(_regs[RS].u32 + SIMM, _regs[RT].u64);
        break;
    default:
        NOT_IMPLEMENTED();
        break;
    }

    _regs[0].u64 = 0;
    _count++;
    if ((_count >> 1) == _compare)
    {
        _ip |= INT_TIMER;
    }
}

#define COP0_NOT_IMPLEMENTED(w)                                                        \
    {                                                                                  \
        std::printf("COP0 reg not implemented (%s): %d\n", w ? "write" : "read", reg); \
        std::exit(2);                                                                  \
    }

std::uint32_t CPU::cop0Read(std::uint8_t reg)
{
    std::uint32_t value{};

    switch (reg)
    {
    case COP0_REG_INDEX:
        std::printf("COP0 Read: COP0_REG_INDEX\n");
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_RANDOM:
        std::printf("COP0 Read: COP0_REG_RANDOM\n");
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_ENTRYLO0:
        std::printf("COP0 Read: COP0_REG_ENTRYLO0\n");
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_ENTRYLO1:
        std::printf("COP0 Read: COP0_REG_ENTRYLO1\n");
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_CONTEXT:
        std::printf("COP0 Read: COP0_REG_CONTEXT\n");
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_PAGEMASK:
        std::printf("COP0 Read: COP0_REG_PAGEMASK\n");
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_WIRED:
        std::printf("COP0 Read: COP0_REG_WIRED\n");
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_BADVADDR:
        std::printf("COP0 Read: COP0_REG_BADVADDR\n");
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_COUNT:
        std::printf("COP0 Read: COP0_REG_COUNT\n");
        value = (_count >> 1);
        break;
    case COP0_REG_ENTRYHI:
        std::printf("COP0 Read: COP0_REG_ENTRYHI\n");
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_COMPARE:
        std::printf("COP0 Read: COP0_REG_COMPARE\n");
        value = _compare;
        break;
    case COP0_REG_SR:
        std::printf("COP0 Read: COP0_REG_SR\n");
        if (_ie) value |= 0x00000001;
        if (_exl) value |= 0x00000002;
        if (_erl) value |= 0x00000004;
        if (_fr) value |= 0x04000000;
        value |= ((std::uint32_t)_im << 8);
        value |= 0x30000000;
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_CAUSE:
        std::printf("COP0 Read: COP0_REG_CAUSE\n");
        value |= (std::uint32_t)(_ip | _mi.ip()) << 8;
        if (_bd) value |= 0x80000000;
        break;
    case COP0_REG_EPC:
        std::printf("COP0 Read: COP0_REG_EPC\n");
        value = _epc;
        break;
    case COP0_REG_PRID:
        std::printf("COP0 Read: COP0_REG_PRID\n");
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_CONFIG:
        std::printf("COP0 Read: COP0_REG_CONFIG\n");
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_LLADDR:
        std::printf("COP0 Read: COP0_REG_LLADDR\n");
        value = _llAddr;
        break;
    case COP0_REG_WATCHLO:
        std::printf("COP0 Read: COP0_REG_WATCHLO\n");
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_WATCHHI:
        std::printf("COP0 Read: COP0_REG_WATCHHI\n");
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_XCONTEXT:
        std::printf("COP0 Read: COP0_REG_XCONTEXT\n");
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_PERR:
        std::printf("COP0 Read: COP0_REG_PERR\n");
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_CACHEERR:
        std::printf("COP0 Read: COP0_REG_CACHEERR\n");
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_TAGLO:
        std::printf("COP0 Read: COP0_REG_TAGLO\n");
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_TAGHI:
        std::printf("COP0 Read: COP0_REG_TAGHI\n");
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_ERROREPC:
        std::printf("COP0 Read: COP0_REG_ERROREPC\n");
        value = _errorEpc;
        break;
    default:
        COP0_NOT_IMPLEMENTED(false);
        break;
    }

    return value;
}

void CPU::cop0Write(std::uint8_t reg, std::uint32_t value)
{
    switch (reg)
    {
    case COP0_REG_INDEX:
        std::printf("COP0 Write: COP0_REG_INDEX 0x%08x\n", value);
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_RANDOM:
        std::printf("COP0 Write: COP0_REG_RANDOM 0x%08x\n", value);
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_ENTRYLO0:
        std::printf("COP0 Write: COP0_REG_ENTRYLO0 0x%08x\n", value);
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_ENTRYLO1:
        std::printf("COP0 Write: COP0_REG_ENTRYLO1 0x%08x\n", value);
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_CONTEXT:
        std::printf("COP0 Write: COP0_REG_CONTEXT 0x%08x\n", value);
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_PAGEMASK:
        std::printf("COP0 Write: COP0_REG_PAGEMASK 0x%08x\n", value);
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_WIRED:
        std::printf("COP0 Write: COP0_REG_WIRED 0x%08x\n", value);
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_BADVADDR:
        std::printf("COP0 Write: COP0_REG_BADVADDR 0x%08x\n", value);
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_COUNT:
        std::printf("COP0 Write: COP0_REG_COUNT 0x%08x\n", value);
        _count = (value << 1);
        std::printf("COUNT WRITE: 0x%08x\n", value);

        break;
    case COP0_REG_ENTRYHI:
        std::printf("COP0 Write: COP0_REG_ENTRYHI 0x%08x\n", value);
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_COMPARE:
        std::printf("COP0 Write: COP0_REG_COMPARE 0x%08x\n", value);
        _compare = value;
        _ip &= ~INT_TIMER;
        std::printf("COMPARE WRITE: 0x%08x\n", value);
        break;
    case COP0_REG_SR:
        std::printf("COP0 Write: COP0_REG_SR 0x%08x\n", value);
        _ie  = !!(value & 0x00000001);
        _exl = !!(value & 0x00000002);
        _erl = !!(value & 0x00000004);
        _fr  = !!(value & 0x04000000);
        _im  = (value >> 8) & 0xff;
        break;
    case COP0_REG_CAUSE:
        std::printf("COP0 Write: COP0_REG_CAUSE 0x%08x\n", value);
        _ip = (_ip & 0xfc) | ((value >> 8) & 0x03);
        break;
    case COP0_REG_EPC:
        std::printf("COP0 Write: COP0_REG_EPC 0x%08x\n", value);
        _epc = value;
        break;
    case COP0_REG_PRID:
        std::printf("COP0 Write: COP0_REG_PRID 0x%08x\n", value);
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_CONFIG:
        std::printf("COP0 Write: COP0_REG_CONFIG 0x%08x\n", value);
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_LLADDR:
        std::printf("COP0 Write: COP0_REG_LLADDR 0x%08x\n", value);
        _llAddr = value;
        break;
    case COP0_REG_WATCHLO:
        std::printf("COP0 Write: COP0_REG_WATCHLO 0x%08x\n", value);
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_WATCHHI:
        std::printf("COP0 Write: COP0_REG_WATCHHI 0x%08x\n", value);
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_XCONTEXT:
        std::printf("COP0 Write: COP0_REG_XCONTEXT 0x%08x\n", value);
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_PERR:
        std::printf("COP0 Write: COP0_REG_PERR 0x%08x\n", value);
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_CACHEERR:
        std::printf("COP0 Write: COP0_REG_CACHEERR 0x%08x\n", value);
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_TAGLO:
        std::printf("COP0 Write: COP0_REG_TAGLO 0x%08x\n", value);
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_TAGHI:
        std::printf("COP0 Write: COP0_REG_TAGHI 0x%08x\n", value);
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_ERROREPC:
        std::printf("COP0 Write: COP0_REG_ERROREPC 0x%08x\n", value);
        _errorEpc = value;
        break;
    }
}

std::uint32_t CPU::fpuReadU32(std::uint8_t reg)
{
    reg = reg & (_fr ? 0x1f : 0x1e);
    return _fpuRegs[reg].u32;
}

std::uint64_t CPU::fpuReadU64(std::uint8_t reg)
{
    std::uint64_t tmp;

    if (_fr)
    {
        tmp = _fpuRegs[reg].u64;
    }
    else
    {
        tmp = _fpuRegs[(reg & 0x1e) + 0].u32;
        tmp |= std::uint64_t(_fpuRegs[(reg & 0x1e) + 1].u32) << 32;
    }
    return tmp;
}

float CPU::fpuReadF32(std::uint8_t reg)
{
    std::uint32_t tmp;

    tmp = fpuReadU32(reg);
    return *(float*)(&tmp);
}

double CPU::fpuReadF64(std::uint8_t reg)
{
    std::uint64_t tmp;

    tmp = fpuReadU64(reg);
    return *(double*)(&tmp);
}

void CPU::fpuWriteU32(std::uint8_t reg, std::uint32_t value)
{
    reg = reg & (_fr ? 0x1f : 0x1e);

    _fpuRegs[reg].u32 = value;
}
void CPU::fpuWriteU64(std::uint8_t reg, std::uint64_t value)
{
    if (_fr)
    {
        _fpuRegs[reg].u64 = value;
    }
    else
    {
        _fpuRegs[(reg & 0x1e) + 0].u32 = ((value >> 0) & 0xffffffff);
        _fpuRegs[(reg & 0x1e) + 1].u32 = ((value >> 8) & 0xffffffff);
    }
}

void CPU::fpuWriteF32(std::uint8_t reg, float value)
{
    fpuWriteU32(reg, *(std::uint32_t*)(&value));
}

void CPU::fpuWriteF64(std::uint8_t reg, double value)
{
    fpuWriteU64(reg, *(std::uint64_t*)(&value));
}

std::uint32_t CPU::fcrRead(std::uint8_t reg)
{
    std::uint32_t value{};

    switch (reg)
    {
    case FCR_CONTROL:
        break;
    case FCR_REVISION:
        value = 0x000B0100;
        break;
    default:
        break;
    }

    return value;
}

void CPU::fcrWrite(std::uint8_t reg, std::uint32_t value)
{
    switch (reg)
    {
    case FCR_CONTROL:
        break;
    case FCR_REVISION:
        break;
    default:
        break;
    }
}
