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

#define RS          ((std::uint8_t)((op >> 21) & 0x1f))
#define BASE        RS
#define E           ((std::uint8_t)((op >> 21) & 0xf))
#define RT          ((std::uint8_t)((op >> 16) & 0x1f))
#define VT          RT
#define RD          ((std::uint8_t)((op >> 11) & 0x1f))
#define OPCODE      RD
#define SA          ((std::uint8_t)((op >> 6) & 0x1f))
#define ELEMENT     ((std::uint8_t)((op >> 6) & 0xf))
#define IMM         ((std::uint16_t)op)
#define SIMM        ((std::int16_t)op)
#define OFFSET      ((std::uint16_t)(op & ((1 << 7) - 1)))
#define FUNC        ((std::uint16_t)(op & ((1 << 6) - 1)))
#define JUMP_TARGET ((std::uint32_t)op & 0x3ffffff)

#define NOT_IMPLEMENTED()                                                                                      \
    {                                                                                                          \
        std::printf("%s:%d PC: 0x%04x Not implemented: OP:%02o RS:%02o RT:%02o RD:%02o %02o %02o FUNC:%02x\n", \
                    __FILE__,                                                                                  \
                    __LINE__,                                                                                  \
                    _pc,                                                                                       \
                    (op >> 26),                                                                                \
                    ((op >> 21) & 0x1f),                                                                       \
                    ((op >> 16) & 0x1f),                                                                       \
                    ((op >> 11) & 0x1f),                                                                       \
                    ((op >> 06) & 0x1f),                                                                       \
                    ((op >> 00) & 0x3f),                                                                       \
                    FUNC);                                                                                     \
        std::exit(1);                                                                                          \
    }

using namespace libnin64;

static __m128i vSelect(__m128i v, std::uint8_t e)
{
    __m128i tmp;

    switch (e & 0b1111)
    {
    case 0b0000: // Whole vector (01234567)
        tmp = v;
        break;
    case 0b0001: // Undefined
        tmp = _mm_setzero_si128();
        break;
    case 0b0010: // 00224466
        tmp = _mm_shufflelo_epi16(v, _MM_SHUFFLE(2, 2, 0, 0));
        tmp = _mm_shufflehi_epi16(tmp, _MM_SHUFFLE(2, 2, 0, 0));
        break;
    case 0b0011: // 11335577
        tmp = _mm_shufflelo_epi16(v, _MM_SHUFFLE(3, 3, 1, 1));
        tmp = _mm_shufflehi_epi16(tmp, _MM_SHUFFLE(3, 3, 1, 1));
        break;
    case 0b0100: // 00004444
        tmp = _mm_shufflelo_epi16(v, _MM_SHUFFLE(0, 0, 0, 0));
        tmp = _mm_shufflehi_epi16(tmp, _MM_SHUFFLE(0, 0, 0, 0));
        break;
    case 0b0101: // 11115555
        tmp = _mm_shufflelo_epi16(v, _MM_SHUFFLE(1, 1, 1, 1));
        tmp = _mm_shufflehi_epi16(tmp, _MM_SHUFFLE(1, 1, 1, 1));
        break;
    case 0b0110: // 22226666
        tmp = _mm_shufflelo_epi16(v, _MM_SHUFFLE(2, 2, 2, 2));
        tmp = _mm_shufflehi_epi16(tmp, _MM_SHUFFLE(2, 2, 2, 2));
        break;
    case 0b0111: // 33337777
        tmp = _mm_shufflelo_epi16(v, _MM_SHUFFLE(3, 3, 3, 3));
        tmp = _mm_shufflehi_epi16(tmp, _MM_SHUFFLE(3, 3, 3, 3));
        break;
    case 0b1000: // 00000000
        tmp = _mm_set1_epi16(_mm_extract_epi16(v, 0));
        break;
    case 0b1001: // 11111111
        tmp = _mm_set1_epi16(_mm_extract_epi16(v, 1));
        break;
    case 0b1010: // 22222222
        tmp = _mm_set1_epi16(_mm_extract_epi16(v, 2));
        break;
    case 0b1011: // 33333333
        tmp = _mm_set1_epi16(_mm_extract_epi16(v, 3));
        break;
    case 0b1100: // 44444444
        tmp = _mm_set1_epi16(_mm_extract_epi16(v, 4));
        break;
    case 0b1101: // 55555555
        tmp = _mm_set1_epi16(_mm_extract_epi16(v, 5));
        break;
    case 0b1110: // 66666666
        tmp = _mm_set1_epi16(_mm_extract_epi16(v, 6));
        break;
    case 0b1111: // 77777777
        tmp = _mm_set1_epi16(_mm_extract_epi16(v, 7));
        break;
    }

    return tmp;
}

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
    alignas(16) char vtmp[16];

    std::uint64_t tmp64;
    std::uint32_t op;
    std::uint32_t tmp32;
    std::uint16_t tmp16;
    std::uint16_t addr;
    std::uint8_t  tmp8;

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
            _regs[RD].u32 = _regs[RT].u32 << SA;
            break;
        case 002: // SRL (Shift Right Logical)
            _regs[RD].u32 = _regs[RT].u32 >> SA;
            break;
        case 003: // SRA (Shift Right Arithmetic)
            _regs[RD].i32 = _regs[RT].i32 >> SA;
            break;
        case 004: // SLLV (Shift Left Logical Variable)
            _regs[RD].u32 = _regs[RT].u32 << (_regs[RS].u16 & 0x1f);
            break;
        case 006: // SRLV (Shift Right Logical Variable)
            _regs[RD].u32 = _regs[RT].u32 >> (_regs[RS].u16 & 0x1f);
            break;
        case 007: // SRAV (Shift Right Arithmetic Variable)
            _regs[RD].i32 = _regs[RT].i32 >> (_regs[RS].u16 & 0x1f);
            break;
        case 010: // JR (Jump Register)
            _pcNext = _regs[RS].u16 & 0xfff;
            break;
        case 011: // JALR (Jump And Link Register)
            _pcNext       = _regs[RS].u16 & 0xfff;
            _regs[RD].u32 = (_pc + 4) & 0xfff;
            break;
        case 015: // BREAK (Halt the RSP)
            NOT_IMPLEMENTED();
            break;
        case 040: // ADD (Add)
        case 041: // ADDU (Add Unsigned)
            _regs[RD].i32 = _regs[RS].i32 + _regs[RT].i32;
            break;
        case 042: // SUB (Subtract)
        case 043: // SUBU (Subtract Unsigned)
            _regs[RD].i32 = _regs[RS].i32 - _regs[RT].i32;
            break;
        case 044: // AND
            _regs[RD].u32 = _regs[RS].u32 & _regs[RT].u32;
            break;
        case 045: // OR
            _regs[RD].u32 = _regs[RS].u32 | _regs[RT].u32;
            break;
        case 046: // XOR
            _regs[RD].u32 = _regs[RS].u32 ^ _regs[RT].u32;
            break;
        case 047: // NOR
            _regs[RD].u32 = ~(_regs[RS].u32 | _regs[RT].u32);
            break;
        case 052: // SLT (Set On Less Than)
            _regs[RD].u32 = (_regs[RS].i32 < _regs[RT].i32) ? 1 : 0;
            break;
        case 053: // SLTU (Set On Less Than Unsigned)
            _regs[RD].u32 = (_regs[RS].u32 < _regs[RT].u32) ? 1 : 0;
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
            if (_regs[RS].i32 < 0)
            {
                _pcNext = (_pc + (SIMM << 2)) & 0xfff;
            }
            break;
        case 001: // BGEZ
            if (_regs[RS].i32 >= 0)
            {
                _pcNext = (_pc + (SIMM << 2)) & 0xfff;
            }
            break;
        case 020: // BLTZAL (Branch On Less Than Zero And Link)
            if (_regs[RS].i32 < 0)
            {
                _pcNext = (_pc + (SIMM << 2)) & 0xfff;
            }
            _regs[31].u32 = (_pc + 4) & 0xfff;
            break;
        case 021: // BGEZAL (Branch On Greater Than Or Equal To Zero And Link)
            if (_regs[RS].i32 >= 0)
            {
                _pcNext = (_pc + (SIMM << 2)) & 0xfff;
            }
            _regs[31].u32 = (_pc + 4) & 0xfff;
            break;
        default:
            NOT_IMPLEMENTED();
            break;
        }
        break;
    case 002: // J (Jump)
        _pcNext = ((JUMP_TARGET << 2) & 0xfff);
        break;
    case 003: // JAL (Jump And Link)
        _regs[31].u32 = ((_pc + 4) & 0xfff);
        _pcNext       = ((JUMP_TARGET << 2) & 0xfff);
        break;
    case 004: // BEQ
        if (_regs[RS].u32 == _regs[RT].u32)
        {
            _pcNext = (_pc + (SIMM << 2)) & 0xfff;
        }
        break;
    case 005: // BNE
        if (_regs[RS].u32 != _regs[RT].u32)
        {
            _pcNext = (_pc + (SIMM << 2)) & 0xfff;
        }
        break;
    case 006: // BLEZ
        if (_regs[RS].i32 <= 0)
        {
            _pcNext = (_pc + (SIMM << 2)) & 0xfff;
        }
        break;
    case 007: // BGTZ
        if (_regs[RS].i32 > 0)
        {
            _pcNext = (_pc + (SIMM << 2)) & 0xfff;
        }
        break;
    case 010: // ADDI (Add Immediate)
    case 011: // ADDIU (Add Immediate Unsigned)
        _regs[RT].i32 = _regs[RS].i32 + SIMM;
        break;
    case 012: // SLTI (Set On Less Than Immediate)
        _regs[RT].u32 = (_regs[RS].i32 < (std::int32_t)SIMM) ? 1 : 0;
        break;
    case 013: // SLTIU (Set On Less Than Immediate Unsigned)
        _regs[RT].u32 = (_regs[RS].u32 < IMM) ? 1 : 0;
        break;
    case 014: // ANDI (And Immediate)
        _regs[RT].u32 = _regs[RS].u32 & IMM;
        break;
    case 015: // ORI (Or Immediate)
        _regs[RT].u32 = _regs[RS].u32 | IMM;
        break;
    case 016: // XORI (Exclusive Or Immediate)
        _regs[RT].u32 = _regs[RS].u32 ^ IMM;
        break;
    case 017: // LUI (Load Upper Immediate)
        _regs[RT].u32 = ((std::uint32_t)IMM) << 16;
        break;
    case 020: // COP0 (Coprocessor 0)
        switch (RS)
        {
        case 000: // MFC0
            _regs[RT].u32 = cop0Read(RD);
            break;
        case 004: // MTC0
            cop0Write(RD, _regs[RT].u32);
            break;
        default:
            NOT_IMPLEMENTED();
            break;
        }
        break;
    case 022: // COP2 (Coprocessor 2)
        switch (RS)
        {
        case 000: // MFC2
            NOT_IMPLEMENTED();
            break;
        case 002: // CFC2
            NOT_IMPLEMENTED();
            break;
        case 004: // MTC2
            _mm_store_si128((__m128i*)vtmp, _vregs[RD].i);
            *(uint16_t*)(vtmp + (ELEMENT & 0xe)) = _regs[RT].u16;
            _vregs[RD].i                         = _mm_load_si128((__m128i*)vtmp);
            break;
        case 006: // CTC2
            NOT_IMPLEMENTED();
            break;
        case 010: // BC2
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
        case 037: // CO
            switch (FUNC)
            {
            case 0b000000: // VMULF (Vector Multiply of Signed Fractions)
                NOT_IMPLEMENTED();
                break;
            case 0b000001:
                NOT_IMPLEMENTED();
                break;
            case 0b000010:
                NOT_IMPLEMENTED();
                break;
            case 0b000011:
                NOT_IMPLEMENTED();
                break;
            case 0b000100:
                NOT_IMPLEMENTED();
                break;
            case 0b000101:
                NOT_IMPLEMENTED();
                break;
            case 0b000110:
                NOT_IMPLEMENTED();
                break;
            case 0b000111:
                NOT_IMPLEMENTED();
                break;
            case 0b001000:
                NOT_IMPLEMENTED();
                break;
            case 0b001001:
                NOT_IMPLEMENTED();
                break;
            case 0b001010:
                NOT_IMPLEMENTED();
                break;
            case 0b001011:
                NOT_IMPLEMENTED();
                break;
            case 0b001100:
                NOT_IMPLEMENTED();
                break;
            case 0b001101:
                NOT_IMPLEMENTED();
                break;
            case 0b001110:
                NOT_IMPLEMENTED();
                break;
            case 0b001111:
                NOT_IMPLEMENTED();
                break;
            case 0b010000:
                NOT_IMPLEMENTED();
                break;
            case 0b010001:
                NOT_IMPLEMENTED();
                break;
            case 0b010010:
                NOT_IMPLEMENTED();
                break;
            case 0b010011:
                NOT_IMPLEMENTED();
                break;
            case 0b010100:
                NOT_IMPLEMENTED();
                break;
            case 0b010101:
                NOT_IMPLEMENTED();
                break;
            case 0b010110:
                NOT_IMPLEMENTED();
                break;
            case 0b010111:
                NOT_IMPLEMENTED();
                break;
            case 0b011000:
                NOT_IMPLEMENTED();
                break;
            case 0b011001:
                NOT_IMPLEMENTED();
                break;
            case 0b011010:
                NOT_IMPLEMENTED();
                break;
            case 0b011011:
                NOT_IMPLEMENTED();
                break;
            case 0b011100:
                NOT_IMPLEMENTED();
                break;
            case 0b011101:
                NOT_IMPLEMENTED();
                break;
            case 0b011110:
                NOT_IMPLEMENTED();
                break;
            case 0b011111:
                NOT_IMPLEMENTED();
                break;
            case 0b100000:
                NOT_IMPLEMENTED();
                break;
            case 0b100001:
                NOT_IMPLEMENTED();
                break;
            case 0b100010:
                NOT_IMPLEMENTED();
                break;
            case 0b100011:
                NOT_IMPLEMENTED();
                break;
            case 0b100100:
                NOT_IMPLEMENTED();
                break;
            case 0b100101:
                NOT_IMPLEMENTED();
                break;
            case 0b100110:
                NOT_IMPLEMENTED();
                break;
            case 0b100111:
                NOT_IMPLEMENTED();
                break;
            case 0b101000:
                NOT_IMPLEMENTED();
                break;
            case 0b101001:
                NOT_IMPLEMENTED();
                break;
            case 0b101010:
                NOT_IMPLEMENTED();
                break;
            case 0b101011:
                NOT_IMPLEMENTED();
                break;
            case 0b101100:
                NOT_IMPLEMENTED();
                break;
            case 0b101101:
                NOT_IMPLEMENTED();
                break;
            case 0b101110:
                NOT_IMPLEMENTED();
                break;
            case 0b101111:
                NOT_IMPLEMENTED();
                break;
            case 0b110000:
                NOT_IMPLEMENTED();
                break;
            case 0b110001:
                NOT_IMPLEMENTED();
                break;
            case 0b110010:
                NOT_IMPLEMENTED();
                break;
            case 0b110011:
                NOT_IMPLEMENTED();
                break;
            case 0b110100:
                NOT_IMPLEMENTED();
                break;
            case 0b110101:
                NOT_IMPLEMENTED();
                break;
            case 0b110110:
                NOT_IMPLEMENTED();
                break;
            case 0b110111:
                NOT_IMPLEMENTED();
                break;
            case 0b111000:
                NOT_IMPLEMENTED();
                break;
            case 0b111001:
                NOT_IMPLEMENTED();
                break;
            case 0b111010:
                NOT_IMPLEMENTED();
                break;
            case 0b111011:
                NOT_IMPLEMENTED();
                break;
            case 0b111100:
                NOT_IMPLEMENTED();
                break;
            case 0b111101:
                NOT_IMPLEMENTED();
                break;
            case 0b111110:
                NOT_IMPLEMENTED();
                break;
            case 0b111111:
                NOT_IMPLEMENTED();
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
    case 040: // LB (Load Byte)
        _regs[RT].i32 = (std::int8_t)dRead8(_regs[BASE].u16 + SIMM);
        break;
    case 041: // LH (Load Halfword)
        _regs[RT].i32 = (std::int16_t)dRead16(_regs[BASE].u16 + SIMM);
        break;
    case 043: // LW (Load Word)
        _regs[RT].u32 = dRead32(_regs[BASE].u16 + SIMM);
        break;
    case 044: // LBU (Load Byte Unsigned)
        _regs[RT].u32 = dRead8(_regs[BASE].u16 + SIMM);
        break;
    case 045: // LHU (Load Halfword Unsigned)
        _regs[RT].u32 = dRead16(_regs[BASE].u16 + SIMM);
        break;
    case 050: // SB (Store Byte)
        dWrite8(_regs[BASE].u16 + SIMM, _regs[RT].u8);
        break;
    case 051: // SH (Store Halfword)
        dWrite16(_regs[BASE].u16 + SIMM, _regs[RT].u16);
        break;
    case 053: // SW (Store Word)
        dWrite32(_regs[BASE].u16 + SIMM, _regs[RT].u32);
        break;
    case 062: // LWC2
        _mm_store_si128((__m128i*)vtmp, _vregs[VT].i);
        switch (OPCODE)
        {
        case 0b00000: // LBV
            NOT_IMPLEMENTED();
            break;
        case 0b00001: // LSV
            NOT_IMPLEMENTED();
            break;
        case 0b00010: // LLV
            NOT_IMPLEMENTED();
            break;
        case 0b00011: // LDV (Load Double into Vector Register)
            *(uint64_t*)(vtmp + (8 - (ELEMENT & 8))) = dRead64((_regs[BASE].u16 + (OFFSET << 3)) & 0xfff);
            break;
        case 0b00100: // LQV (Load Quad into Vector Register)
            addr = (_regs[BASE].u16 + (OFFSET << 4)) & 0xfff;
            for (int i = 0; i < 16 - (addr & 0xf); ++i)
            {
                vtmp[15 - i] = dRead8((addr & 0xff0) + i);
            }
            break;
        case 0b00101: // LRV
            NOT_IMPLEMENTED();
            break;
        case 0b00110: // LPV
            NOT_IMPLEMENTED();
            break;
        case 0b00111: // LUV
            NOT_IMPLEMENTED();
            break;
        case 0b01000: // LHV
            NOT_IMPLEMENTED();
            break;
        case 0b01001: // LFV
            NOT_IMPLEMENTED();
            break;
        case 0b01011: // LTV
            NOT_IMPLEMENTED();
            break;
        default:
            NOT_IMPLEMENTED();
            break;
        }
        _vregs[VT].i = _mm_load_si128((__m128i*)vtmp);
        break;
    case 072: // SWC2
        _mm_store_si128((__m128i*)vtmp, _vregs[VT].i);
        switch (OPCODE)
        {
        case 0b00000: // SBV
            NOT_IMPLEMENTED();
            break;
        case 0b00001: // SSV
            NOT_IMPLEMENTED();
            break;
        case 0b00010: // SLV
            NOT_IMPLEMENTED();
            break;
        case 0b00011: // SDV (Store Double into Vector Register)
            dWrite64((_regs[BASE].u16 + (OFFSET << 3)) & 0xfff, *(uint64_t*)(vtmp + (8 - (ELEMENT & 8))));
            break;
        case 0b00100: // SQV
            NOT_IMPLEMENTED();
            break;
        case 0b00101: // SRV
            NOT_IMPLEMENTED();
            break;
        case 0b00110: // SPV
            NOT_IMPLEMENTED();
            break;
        case 0b00111: // SUV
            NOT_IMPLEMENTED();
            break;
        case 0b01000: // SHV
            NOT_IMPLEMENTED();
            break;
        case 0b01001: // SFV
            NOT_IMPLEMENTED();
            break;
        case 0b01011: // STV
            NOT_IMPLEMENTED();
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
        break;
    case SP_DMA_BUSY_REG:
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

std::uint32_t RSP::cop0Read(std::uint8_t reg)
{
    std::uint32_t value{};

    switch (reg)
    {
    case 0:
        value = read(SP_MEM_ADDR_REG);
        break;
    case 1:
        value = read(SP_DRAM_ADDR_REG);
        break;
    case 2:
        value = read(SP_RD_LEN_REG);
        break;
    case 3:
        value = read(SP_WR_LEN_REG);
        break;
    case 4:
        value = read(SP_STATUS_REG);
        break;
    case 5:
        value = read(SP_DMA_FULL_REG);
        break;
    case 6:
        value = read(SP_DMA_BUSY_REG);
        break;
    case 7:
        value = read(SP_SEMAPHORE_REG);
        break;
    case 8:
        // TODO: RDP
        break;
    case 9:
        // TODO: RDP
        break;
    case 10:
        // TODO: RDP
        break;
    case 11:
        // TODO: RDP
        break;
    case 12:
        // TODO: RDP
        break;
    case 13:
        // TODO: RDP
        break;
    case 14:
        // TODO: RDP
        break;
    case 15:
        // TODO: RDP
        break;
    }

    return value;
}

void RSP::cop0Write(std::uint8_t reg, std::uint32_t value)
{
    switch (reg)
    {
    case 0:
        write(SP_MEM_ADDR_REG, value);
        break;
    case 1:
        write(SP_DRAM_ADDR_REG, value);
        break;
    case 2:
        write(SP_RD_LEN_REG, value);
        break;
    case 3:
        write(SP_WR_LEN_REG, value);
        break;
    case 4:
        write(SP_STATUS_REG, value);
        break;
    case 5:
        write(SP_DMA_FULL_REG, value);
        break;
    case 6:
        write(SP_DMA_BUSY_REG, value);
        break;
    case 7:
        write(SP_SEMAPHORE_REG, value);
        break;
    case 8:
        // TODO: RDP
        break;
    case 9:
        // TODO: RDP
        break;
    case 10:
        // TODO: RDP
        break;
    case 11:
        // TODO: RDP
        break;
    case 12:
        // TODO: RDP
        break;
    case 13:
        // TODO: RDP
        break;
    case 14:
        // TODO: RDP
        break;
    case 15:
        // TODO: RDP
        break;
    }
}
