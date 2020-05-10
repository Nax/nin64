#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <libnin64/MIPSInterface.h>
#include <libnin64/Memory.h>
#include <libnin64/RSP.h>
#include <libnin64/Util.h>

#define SP_MEM_ADDR_REG  0x04040000
#define SP_DRAM_ADDR_REG 0x04040004
#define SP_RD_LEN_REG    0x04040008
#define SP_WR_LEN_REG    0x0404000c
#define SP_STATUS_REG    0x04040010
#define SP_DMA_FULL_REG  0x04040014
#define SP_DMA_BUSY_REG  0x04040018
#define SP_SEMAPHORE_REG 0x0404001c
#define SP_PC_REG        0x04080000
#define SP_IBIST_REG     0x04080004

#define NOT_IMPLEMENTED()                                                                            \
    {                                                                                                \
        std::printf("%s:%d PC: 0x%04x Not implemented: OP:%02o RS:%02o RT:%02o RD:%02o %02o %02o\n", \
                    __FILE__,                                                                        \
                    __LINE__,                                                                        \
                    _pc,                                                                             \
                    (op >> 26),                                                                      \
                    ((op >> 21) & 0x1f),                                                             \
                    ((op >> 16) & 0x1f),                                                             \
                    ((op >> 11) & 0x1f),                                                             \
                    ((op >> 06) & 0x1f),                                                             \
                    ((op >> 00) & 0x3f));                                                            \
        std::exit(1);                                                                                \
    }

#define RS          ((std::uint8_t)((op >> 21) & 0x1f))
#define RT          ((std::uint8_t)((op >> 16) & 0x1f))
#define RD          ((std::uint8_t)((op >> 11) & 0x1f))
#define SA          ((std::uint8_t)((op >> 6) & 0x1f))
#define IMM         ((std::uint16_t)op)
#define SIMM        ((std::int16_t)op)
#define JUMP_TARGET ((std::uint32_t)op & 0x3ffffff)

using namespace libnin64;

RSP::RSP(Memory& memory, MIPSInterface& mi)
: _memory{memory}
, _mi{mi}
, _halt{true}
, _semaphore{}
, _spAddr{}
, _dramAddr{}
, _regs{}
, _pc{0}
, _pcNext{4}
{
}

RSP::~RSP()
{
}

void RSP::tick(std::size_t count)
{
    if (_halt) return;

    while (count--)
    {
        tick();
    }
}

void RSP::tick()
{
    std::uint32_t op;

    if (_halt) return;

    op      = swap(*(std::uint32_t*)(_memory.spImem + (_pc & 0xfff)));
    _pc     = _pcNext;
    _pcNext = _pcNext + 4;

    switch ((op >> 26) & 077)
    {
    case 000: // SPECIAL
        switch (op & 0x3f)
        {
        case 000: // SLL (Shift Left Logical)
            NOT_IMPLEMENTED();
            break;
        case 002: // SRL (Shift Right Logical)
            NOT_IMPLEMENTED();
            break;
        case 003: // SRA (Shift Right Arithmetic)
            NOT_IMPLEMENTED();
            break;
        case 004: // SLLV (Shift Left Logical Variable)
            NOT_IMPLEMENTED();
            break;
        case 006: // SRLV (Shift Right Logical Variable)
            NOT_IMPLEMENTED();
            break;
        case 007: // SRAV (Shift Right Arithmetic Variable)
            NOT_IMPLEMENTED();
            break;
        case 010: // JR (Jump Register)
            NOT_IMPLEMENTED();
            break;
        case 011: // JALR (Jump And Link Register)
            NOT_IMPLEMENTED();
            break;
        case 015: // BREAK (Breakpoint)
            NOT_IMPLEMENTED();
            break;
        case 040: // ADD (Add)
            NOT_IMPLEMENTED();
            break;
        case 041: // ADDU (Add Unsigned)
            NOT_IMPLEMENTED();
            break;
        case 042: // SUB (Subtract)
            NOT_IMPLEMENTED();
            break;
        case 043: // SUBU (Subtract Unsigned)
            NOT_IMPLEMENTED();
            break;
        case 044: // AND
            NOT_IMPLEMENTED();
            break;
        case 045: // OR
            NOT_IMPLEMENTED();
            break;
        case 046: // XOR
            NOT_IMPLEMENTED();
            break;
        case 047: // NOR
            NOT_IMPLEMENTED();
            break;
        case 052: // SLT (Set On Less Than)
            NOT_IMPLEMENTED();
            break;
        case 053: // SLTU (Set On Less Than Unsigned)
            NOT_IMPLEMENTED();
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
            NOT_IMPLEMENTED();
            break;
        case 001: // BGEZ
            NOT_IMPLEMENTED();
            break;
        case 020: // BLTZAL (Branch On Less Than Zero And Link Likely)
            NOT_IMPLEMENTED();
            break;
        case 021: // BGEZAL (Branch On Greater Than Or Equal To Zero And Link Likely)
            NOT_IMPLEMENTED();
            break;
        default:
            NOT_IMPLEMENTED();
            break;
        }
        break;
    case 002: // J (Jump)
        NOT_IMPLEMENTED();
        break;
    case 003: // JAL (Jump And Link)
        NOT_IMPLEMENTED();
        break;
    case 004: // BEQ
        NOT_IMPLEMENTED();
        break;
    case 005: // BNE
        NOT_IMPLEMENTED();
        break;
    case 006: // BLEZ
        NOT_IMPLEMENTED();
        break;
    case 007: // BGTZ
        NOT_IMPLEMENTED();
        break;
    case 010: // ADDI (Add Immediate)
        NOT_IMPLEMENTED();
        break;
    case 011: // ADDIU (Add Immediate Unsigned)
        NOT_IMPLEMENTED();
        break;
    case 012: // SLTI (Set On Less Than Immediate)
        NOT_IMPLEMENTED();
        break;
    case 013: // SLTIU (Set On Less Than Immediate Unsigned)
        NOT_IMPLEMENTED();
        break;
    case 014: // ANDI (And Immediate)
        NOT_IMPLEMENTED();
        break;
    case 015: // ORI (Or Immediate)
        NOT_IMPLEMENTED();
        break;
    case 016: // XORI (Exclusive Or Immediate)
        NOT_IMPLEMENTED();
        break;
    case 017: // LUI (Load Upper Immediate)
        NOT_IMPLEMENTED();
        break;
    case 020: // COP0 (Coprocessor 0)
        NOT_IMPLEMENTED();
        break;
    case 021: // COP1 (Coprocessor 1)
        NOT_IMPLEMENTED();
        break;
    case 022: // COP2 (Coprocessor 2)
        NOT_IMPLEMENTED();
        break;
    case 040: // LB (Load Byte)
        NOT_IMPLEMENTED();
        break;
    case 041: // LH (Load Halfword)
        NOT_IMPLEMENTED();
        break;
    case 043: // LW (Load Word)
        NOT_IMPLEMENTED();
        break;
    case 044: // LBU (Load Byte Unsigned)
        NOT_IMPLEMENTED();
        break;
    case 045: // LHU (Load Halfword Unsigned)
        NOT_IMPLEMENTED();
        break;
    case 050: // SB (Store Byte)
        NOT_IMPLEMENTED();
        break;
    case 051: // SH (Store Halfword)
        NOT_IMPLEMENTED();
        break;
    case 053: // SW (Store Word)
        NOT_IMPLEMENTED();
        break;
    case 057: // CACHE
        NOT_IMPLEMENTED();
        break;
    case 061: // LWC1 (Load Word to FPU)
        NOT_IMPLEMENTED();
        break;
    case 062: // LWC2
        NOT_IMPLEMENTED();
        break;
    case 071: // SWC1 (Store Word From FPU)
        NOT_IMPLEMENTED();
        break;
    case 072: // SWC2
        NOT_IMPLEMENTED();
        break;
    default:
        NOT_IMPLEMENTED();
        break;
    }

    _regs[0].u32 = 0;
}

std::uint32_t RSP::read(std::uint32_t reg)
{
    std::uint32_t value{};

    switch (reg)
    {
    case SP_MEM_ADDR_REG:
        value = _spAddr;
        break;
    case SP_DRAM_ADDR_REG:
        value = _dramAddr;
        break;
    case SP_RD_LEN_REG:
        std::puts("READ::SP_RD_LEN_REG NOT IMPLEMENTED");
        exit(42);
        break;
    case SP_WR_LEN_REG:
        std::puts("READ::SP_WR_LEN_REG NOT IMPLEMENTED");
        exit(42);
        break;
    case SP_STATUS_REG:
        if (_halt) value |= 0x00000001;
        break;
    case SP_DMA_FULL_REG:
        std::puts("READ::SP_DMA_FULL_REG NOT IMPLEMENTED");
        exit(42);
        break;
    case SP_DMA_BUSY_REG:
        std::puts("READ::SP_DMA_BUSY_REG NOT IMPLEMENTED");
        exit(42);
        break;
    case SP_SEMAPHORE_REG:
        if (_semaphore) value |= 0x01;
        _semaphore = true;
        break;
    case SP_PC_REG:
        value = _pc;
        break;
    case SP_IBIST_REG:
        std::puts("READ::SP_IBIST_REG NOT IMPLEMENTED");
        exit(42);
        break;
    default:
        break;
    }

    return value;
}

void RSP::write(std::uint32_t reg, std::uint32_t value)
{
    switch (reg)
    {
    case SP_MEM_ADDR_REG:
        _spAddr = value & 0x1fff;
        break;
    case SP_DRAM_ADDR_REG:
        _dramAddr = value & 0xffffff;
        break;
    case SP_RD_LEN_REG:
        dmaRead((value & 0xfff) + 1, ((value >> 12) & 0xff) + 1, value >> 20);
        break;
    case SP_WR_LEN_REG:
        std::puts("WRITE::SP_WR_LEN_REG NOT IMPLEMENTED");
        exit(42);
        break;
    case SP_STATUS_REG:
        if (value & (1 << 0))
        {
            _halt = false;
            std::printf("RSP START!!!\n");
        }
        if (value & (1 << 1)) _halt = true;
        break;
    case SP_DMA_FULL_REG:
        std::puts("WRITE::SP_DMA_FULL_REG NOT IMPLEMENTED");
        exit(42);
        break;
    case SP_DMA_BUSY_REG:
        std::puts("WRITE::SP_DMA_BUSY_REG NOT IMPLEMENTED");
        exit(42);
        break;
    case SP_SEMAPHORE_REG:
        _semaphore = false;
        break;
    case SP_PC_REG:
        _pc     = (value & 0xfff);
        _pcNext = (_pc + 4) & 0xfff;
        break;
    case SP_IBIST_REG:
        std::puts("WRITE::SP_IBIST_REG NOT IMPLEMENTED");
        exit(42);
        break;
    default:
        break;
    }
}

void RSP::dmaRead(std::uint16_t length, std::uint16_t count, std::uint16_t skip)
{
    std::uint8_t* src;
    std::uint8_t* dst;

    std::printf("SP DMA (Read)!\n");

    src = _memory.ram + _dramAddr;

    if (_spAddr & 0x1000)
        dst = _memory.spImem + (_spAddr & 0xfff);
    else
        dst = _memory.spDmem + _spAddr;

    if (skip == 0)
    {
        std::memcpy(dst, src, length * count);
    }
    else
    {
        for (int i = 0; i < count; ++i)
        {
            std::memcpy(dst + i * length, src + i * (length + skip), length);
        }
    }
}

void RSP::dmaWrite(std::uint16_t length, std::uint16_t count, std::uint16_t skip)
{
    std::printf("SP DMA (Write)!\n");
}

template <typename T> T RSP::dRead(std::uint16_t addr)
{
    addr &= 0xfff;

    return swap(*(T*)(_memory.spDmem + addr));
}

template <typename T> void RSP::dWrite(std::uint16_t addr, T value)
{
    addr &= 0xfff;

    *(T*)(_memory.spDmem + addr) = swap(value);
}
