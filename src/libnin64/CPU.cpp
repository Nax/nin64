#include <cstdio>
#include <cstdlib>
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

#define NOT_IMPLEMENTED()                                                                         \
    {                                                                                             \
        std::printf("PC: 0x%016llx Not implemented: OP:%02o RS:%02o RT:%02o RD:%02o %02o %02o\n", \
                    _pc,                                                                          \
                    (op >> 26),                                                                   \
                    ((op >> 21) & 0x1f),                                                          \
                    ((op >> 16) & 0x1f),                                                          \
                    ((op >> 11) & 0x1f),                                                          \
                    ((op >> 06) & 0x1f),                                                          \
                    ((op >> 00) & 0x3f));                                                         \
        exit(1);                                                                                  \
    }

#define RS          ((std::uint8_t)((op >> 21) & 0x1f))
#define RT          ((std::uint8_t)((op >> 16) & 0x1f))
#define RD          ((std::uint8_t)((op >> 11) & 0x1f))
#define FS          RD
#define SA          ((std::uint8_t)((op >> 6) & 0x1f))
#define IMM         ((std::uint16_t)op)
#define SIMM        ((std::int16_t)op)
#define JUMP_TARGET ((std::uint32_t)op & 0x3ffffff)

#define INT_TIMER 0x80

using namespace libnin64;

CPU::CPU(Bus& bus, MIPSInterface& mi)
: _bus{bus}
, _mi{mi}
, _pc{0xffffffffa4000040ull}
, _pcNext{_pc + 4}
, _regs{}
, _llAddr{}
, _epc{}
, _errorEpc{}
, _ip{}
, _im{}
, _erl{true}
, _exl{}
, _ie{}
, _llBit{}
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
    std::printf("PC: %016llx\n", _pc);
}

void CPU::tick()
{
    std::uint32_t op;
    std::uint64_t tmp;
    std::uint64_t tmp2;

    if (_ie && !_erl && !_exl && (_im & (_ip | _mi.ip())))
    {
        std::printf("INTERRUPT !!!\n");
        std::exit(3);
    }
    // For current tick
    op = _bus.read32((std::uint32_t)_pc);
    //std::printf("PC: 0x%016llx OP: 0x%08x Details:%02o %02o %02o %02o %02o %02o\n", _pc,  op, (op >> 26), ((op >> 21) & 0x1f), ((op >> 16) & 0x1f), ((op >> 11) & 0x1f), ((op >> 06) & 0x1f), ((op >> 00) & 0x3f));
    //for (int i = 0; i < 32; ++i) { std::printf("  REG %02d: 0x%016llx\n", i, _regs[i].u64); }

    // For next tick
    _pc = _pcNext;
    _pcNext += 4;

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
            NOT_IMPLEMENTED();
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
            NOT_IMPLEMENTED();
            break;
        case 055: // DADDU
            NOT_IMPLEMENTED();
            break;
        case 056: // DSUB
            NOT_IMPLEMENTED();
            break;
        case 057: // DSUBU
            NOT_IMPLEMENTED();
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
                _pcNext = _pc + ((std::int64_t)SIMM << 2);
            }
            break;
        case 001: // BGEZ
            if (_regs[RS].i64 >= 0)
            {
                _pcNext = _pc + ((std::int64_t)SIMM << 2);
            }
            break;
        case 002: // BLTZL
            if (_regs[RS].i64 < 0)
            {
                _pcNext = _pc + ((std::int64_t)SIMM << 2);
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
                _pcNext = _pc + ((std::int64_t)SIMM << 2);
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
                _pcNext = _pc + ((std::int64_t)SIMM << 2);
            }
            break;
        case 021: // BGEZAL (Branch On Greater Than Or Equal To Zero And Link Likely)
            _regs[31].u64 = _pc + 4;
            if (_regs[RS].i64 >= 0)
            {
                _pcNext = _pc + ((std::int64_t)SIMM << 2);
            }
            break;
        case 022: // BLTZALL
            _regs[31].u64 = _pc + 4;
            if (_regs[RS].i64 < 0)
            {
                _pcNext = _pc + ((std::int64_t)SIMM << 2);
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
                _pcNext = _pc + ((std::int64_t)SIMM << 2);
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
        _pcNext = ((std::uint64_t)JUMP_TARGET << 2) | (_pc & 0xfffffffff0000000ULL);
        break;
    case 003: // JAL (Jump And Link)
        _pcNext       = ((std::uint64_t)JUMP_TARGET << 2) | (_pc & 0xfffffffff0000000ULL);
        _regs[31].u64 = _pc + 4;
        break;
    case 004: // BEQ
        if (_regs[RS].u64 == _regs[RT].u64)
        {
            _pcNext = _pc + ((std::int64_t)SIMM << 2);
        }
        break;
    case 005: // BNE
        if (_regs[RS].u64 != _regs[RT].u64)
        {
            _pcNext = _pc + ((std::int64_t)SIMM << 2);
        }
        break;
    case 006: // BLEZ
        if (_regs[RS].i64 <= 0)
        {
            _pcNext = _pc + ((std::int64_t)SIMM << 2);
        }
        break;
    case 007: // BGTZ
        if (_regs[RS].i64 > 0)
        {
            _pcNext = _pc + ((std::int64_t)SIMM << 2);
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
                }
                else
                {
                    _pc  = (std::int64_t)((std::int32_t)_epc);
                    _exl = false;
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
        case 000: // MF
            NOT_IMPLEMENTED();
            break;
        case 001: // DMF
            NOT_IMPLEMENTED();
            break;
        case 002: // CF (Move Control From FPU)
            _regs[RT].i64 = (std::int32_t)fcrRead(FS);
            break;
        case 004: // MT
            NOT_IMPLEMENTED();
            break;
        case 005: // DMT
            NOT_IMPLEMENTED();
            break;
        case 006: // CT (Move Control Word To FPU)
            fcrWrite(FS, _regs[RT].u32);
            break;
        case 010: // BC
            switch (RT)
            {
            case 000: // BCF
                break;
            case 001: // BCT
                break;
            case 002: // BCFL
                break;
            case 003: // BCTL
                break;
            }
            NOT_IMPLEMENTED();
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
            _pcNext = _pc + ((std::int64_t)SIMM << 2);
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
            _pcNext = _pc + ((std::int64_t)SIMM << 2);
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
            _pcNext = _pc + ((std::int64_t)SIMM << 2);
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
            _pcNext = _pc + ((std::int64_t)SIMM << 2);
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
        _regs[RT].i64 = (std::int8_t)_bus.read8(_regs[RS].u32 + SIMM);
        break;
    case 041: // LH (Load Halfword)
        _regs[RT].i64 = (std::int16_t)_bus.read16(_regs[RS].u32 + SIMM);
        break;
    case 042: // LWL (Load Word Left)
        NOT_IMPLEMENTED();
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
        NOT_IMPLEMENTED();
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
        NOT_IMPLEMENTED();
        break;
    case 053: // SW (Store Word)
        _bus.write32(_regs[RS].u32 + SIMM, _regs[RT].u32);
        break;
    case 054: // SDL (Store Doubleword Left)
        NOT_IMPLEMENTED();
        break;
    case 055: // SDR (Store Doubleword Right)
        tmp  = _regs[RS].u64 + SIMM;
        tmp2 = _bus.read64(tmp & 0xfffffff8); // Mask the offset
        switch (tmp & 0x7)
        {
        case 0x0:
            _bus.write64((tmp & 0xfffffff8), (_regs[RT].u64 << 56) | (tmp2 & 0x00ffffffffffffffull));
            break;
        case 0x1:
            _bus.write64((tmp & 0xfffffff8), (_regs[RT].u64 << 48) | (tmp2 & 0x0000ffffffffffffull));
            break;
        case 0x2:
            _bus.write64((tmp & 0xfffffff8), (_regs[RT].u64 << 40) | (tmp2 & 0x000000ffffffffffull));
            break;
        case 0x3:
            _bus.write64((tmp & 0xfffffff8), (_regs[RT].u64 << 32) | (tmp2 & 0x00000000ffffffffull));
            break;
        case 0x4:
            _bus.write64((tmp & 0xfffffff8), (_regs[RT].u64 << 24) | (tmp2 & 0x0000000000ffffffull));
            break;
        case 0x5:
            _bus.write64((tmp & 0xfffffff8), (_regs[RT].u64 << 16) | (tmp2 & 0x000000000000ffffull));
            break;
        case 0x6:
            _bus.write64((tmp & 0xfffffff8), (_regs[RT].u64 << 8) | (tmp2 & 0x00000000000000ffull));
            break;
        case 0x7:
            _bus.write64((tmp & 0xfffffff8), _regs[RT].u64);
            break;
        }
        break;
    case 056: // SWR (Store Word Right)
        NOT_IMPLEMENTED();
        break;
    case 057: // CACHE
        break;
    case 060: // LL (Load Linked)
        tmp           = _regs[RS].u32 + SIMM;
        _regs[RT].i64 = (std::int32_t)_bus.read32(tmp);
        _llAddr       = tmp;
        _llBit        = true;
        break;
    case 061: // LWC1
        NOT_IMPLEMENTED();
        break;
    case 062: // LWC2
        NOT_IMPLEMENTED();
        break;
    case 064: // LLD
        NOT_IMPLEMENTED();
        break;
    case 065: // LDC1
        NOT_IMPLEMENTED();
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
    case 071: // SWC1
        NOT_IMPLEMENTED();
        break;
    case 072: // SWC2
        NOT_IMPLEMENTED();
        break;
    case 074: // SCD (Store Conditional Doubleword)
        NOT_IMPLEMENTED();
        break;
    case 075: // SDC1 (Store Doubleword From Coprocessor 1)
        NOT_IMPLEMENTED();
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
    if (_count == _compare)
    {
        _ip |= INT_TIMER;
        std::printf("COUNT == COMPARE!\n");
        std::exit(4);
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
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_RANDOM:
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_ENTRYLO0:
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_ENTRYLO1:
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_CONTEXT:
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_PAGEMASK:
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_WIRED:
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_BADVADDR:
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_COUNT:
        value = _count;
        break;
    case COP0_REG_ENTRYHI:
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_COMPARE:
        value = _compare;
        break;
    case COP0_REG_SR:
        if (_ie) value |= 0x00000001;
        if (_exl) value |= 0x00000002;
        if (_erl) value |= 0x00000004;
        value |= ((std::uint32_t)_im << 8);
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_CAUSE:
        value |= (std::uint32_t)(_ip | _mi.ip()) << 8;
        break;
    case COP0_REG_EPC:
        value = _epc;
        break;
    case COP0_REG_PRID:
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_CONFIG:
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_LLADDR:
        value = _llAddr;
        break;
    case COP0_REG_WATCHLO:
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_WATCHHI:
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_XCONTEXT:
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_PERR:
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_CACHEERR:
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_TAGLO:
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_TAGHI:
        // COP0_NOT_IMPLEMENTED(false);
        break;
    case COP0_REG_ERROREPC:
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
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_RANDOM:
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_ENTRYLO0:
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_ENTRYLO1:
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_CONTEXT:
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_PAGEMASK:
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_WIRED:
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_BADVADDR:
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_COUNT:
        std::printf("COUNT: %u\n", value);
        _count = value;
        break;
    case COP0_REG_ENTRYHI:
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_COMPARE:
        std::printf("COMPARE: %u\n", value);
        _compare = value;
        _ip &= ~INT_TIMER;
        break;
    case COP0_REG_SR:
        _ie  = !!(value & 0x00000001);
        _exl = !!(value & 0x00000002);
        _erl = !!(value & 0x00000004);
        _im  = (value >> 8) & 0xff;
        break;
    case COP0_REG_CAUSE:
        _ip = (_ip & 0xfc) | ((value >> 8) & 0x03);
        break;
    case COP0_REG_EPC:
        _epc = value;
        break;
    case COP0_REG_PRID:
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_CONFIG:
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_LLADDR:
        _llAddr = value;
        break;
    case COP0_REG_WATCHLO:
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_WATCHHI:
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_XCONTEXT:
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_PERR:
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_CACHEERR:
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_TAGLO:
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_TAGHI:
        // COP0_NOT_IMPLEMENTED(true);
        break;
    case COP0_REG_ERROREPC:
        _errorEpc = value;
        break;
    }
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
