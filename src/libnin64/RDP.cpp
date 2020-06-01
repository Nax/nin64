#include <cstdio>
#include <cstdlib>
#include <libnin64/RDP.h>

using namespace libnin64;

RDP::RDP(Memory& memory, MIPSInterface& mi)
: _memory{memory}
, _mi{mi}
{
}

RDP::~RDP()
{
}

std::uint32_t RDP::read(std::uint32_t reg)
{
    std::uint32_t value{};

    switch (reg)
    {
    case DPC_START_REG:
        std::printf("RDP Reg Read DPC_START_REG\n");
        std::exit(1);
        break;
    case DPC_END_REG:
        std::printf("RDP Reg Read DPC_END_REG\n");
        std::exit(1);
        break;
    case DPC_CURRENT_REG:
        std::printf("RDP Reg Read DPC_CURRENT_REG\n");
        std::exit(1);
        break;
    case DPC_STATUS_REG:
        //std::printf("RDP Reg Read DPC_STATUS_REG\n");
        //std::getchar();
        //std::exit(1);
        break;
    case DPC_CLOCK_REG:
        std::printf("RDP Reg Read DPC_CLOCK_REG\n");
        std::exit(1);
        break;
    case DPC_BUFBUSY_REG:
        std::printf("RDP Reg Read DPC_BUFBUSY_REG\n");
        std::exit(1);
        break;
    case DPC_PIPEBUSY_REG:
        std::printf("RDP Reg Read DPC_PIPEBUSY_REG\n");
        std::exit(1);
        break;
    case DPC_TMEM_REG:
        std::printf("RDP Reg Read DPC_TMEM_REG\n");
        std::exit(1);
        break;
    case DPS_TBIST_REG:
        std::printf("RDP Reg Read DPS_TBIST_REG\n");
        std::exit(1);
        break;
    case DPS_TEST_MODE_REG:
        std::printf("RDP Reg Read DPS_TEST_MODE_REG\n");
        std::exit(1);
        break;
    case DPS_BUFTEST_ADDR_REG:
        std::printf("RDP Reg Read DPS_BUFTEST_ADDR_REG\n");
        std::exit(1);
        break;
    case DPS_BUFTEST_DATA_REG:
        std::printf("RDP Reg Read DPS_BUFTEST_DATA_REG\n");
        std::exit(1);
        break;
    }

    return value;
}

void RDP::write(std::uint32_t reg, std::uint32_t value)
{
    switch (reg)
    {
    case DPC_START_REG:
        std::printf("RDP Reg Write DPC_START_REG\n");
        std::exit(1);
        break;
    case DPC_END_REG:
        std::printf("RDP Reg Write DPC_END_REG\n");
        std::exit(1);
        break;
    case DPC_CURRENT_REG:
        std::printf("RDP Reg Write DPC_CURRENT_REG\n");
        std::exit(1);
        break;
    case DPC_STATUS_REG:
        std::printf("RDP Reg Write DPC_STATUS_REG\n");
        std::exit(1);
        break;
    case DPC_CLOCK_REG:
        std::printf("RDP Reg Write DPC_CLOCK_REG\n");
        std::exit(1);
        break;
    case DPC_BUFBUSY_REG:
        std::printf("RDP Reg Write DPC_BUFBUSY_REG\n");
        std::exit(1);
        break;
    case DPC_PIPEBUSY_REG:
        std::printf("RDP Reg Write DPC_PIPEBUSY_REG\n");
        std::exit(1);
        break;
    case DPC_TMEM_REG:
        std::printf("RDP Reg Write DPC_TMEM_REG\n");
        std::exit(1);
        break;
    case DPS_TBIST_REG:
        std::printf("RDP Reg Write DPS_TBIST_REG\n");
        std::exit(1);
        break;
    case DPS_TEST_MODE_REG:
        std::printf("RDP Reg Write DPS_TEST_MODE_REG\n");
        std::exit(1);
        break;
    case DPS_BUFTEST_ADDR_REG:
        std::printf("RDP Reg Write DPS_BUFTEST_ADDR_REG\n");
        std::exit(1);
        break;
    case DPS_BUFTEST_DATA_REG:
        std::printf("RDP Reg Write DPS_BUFTEST_DATA_REG\n");
        std::exit(1);
        break;
    }
}
