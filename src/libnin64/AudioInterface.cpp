#include <cstdio>
#include <cstdlib>
#include <libnin64/AudioInterface.h>
#include <libnin64/MIPSInterface.h>
#include <libnin64/Memory.h>
#include <libnin64/Util.h>

#define AI_DRAM_ADDR_REG 0x04500000
#define AI_LEN_REG       0x04500004
#define AI_CONTROL_REG   0x04500008
#define AI_STATUS_REG    0x0450000c
#define AI_DACRATE_REG   0x04500010
#define AI_BITRATE_REG   0x04500014

#define TICKS_PER_SAMPLE (93750000 / 32000)

using namespace libnin64;

static void dummyAudioCallback(const std::uint16_t* samples, std::size_t sampleCount, void* arg)
{
    (void)samples;
    (void)sampleCount;
    (void)arg;
}

AudioInterface::AudioInterface(MIPSInterface& mi, Memory& memory)
: _mi{mi}
, _memory{memory}
, _callback{&dummyAudioCallback}
, _callbackArg{}
, _acc{}
, _addr{}
, _len{}
, _bufCount{}
, _buffer{}
{
}

AudioInterface::~AudioInterface()
{
}

void AudioInterface::setCallback(Nin64AudioCallback callback, void* callbackArg)
{
    _callback    = callback;
    _callbackArg = callbackArg;
}

void AudioInterface::tick(std::uint32_t count)
{
    if (_bufCount == 0)
        return;

    _acc += count;
    while (_acc >= TICKS_PER_SAMPLE)
    {
        _acc -= TICKS_PER_SAMPLE;
        _len[0]--;
        if (_len[0] == 0)
        {
            _bufCount--;
            if (_bufCount)
            {
                _mi.setInterrupt(MI_INTR_AI);
                _len[0] = _len[1];
            }
            else
            {
                _acc = 0;
                break;
            }
        }
    }
}

std::uint32_t AudioInterface::read(std::uint32_t reg)
{
    std::uint32_t value{};

    switch (reg)
    {
    case AI_DRAM_ADDR_REG:
        std::printf("AI Read: AI_DRAM_ADDR_REG\n");
        break;
    case AI_LEN_REG:
        std::printf("AI Read: AI_LEN_REG\n");
        value = _len[0];
        break;
    case AI_CONTROL_REG:
        std::printf("AI Read: AI_CONTROL_REG\n");
        break;
    case AI_STATUS_REG:
        std::printf("AI Read: AI_STATUS_REG\n");
        if (_bufCount == 2) value |= 0x80000001;
        if (_bufCount) value |= 0x40000000;
        break;
    case AI_DACRATE_REG:
        std::printf("AI Read: AI_DACRATE_REG\n");
        break;
    case AI_BITRATE_REG:
        std::printf("AI Read: AI_BITRATE_REG\n");
        break;
    }

    return value;
}

void AudioInterface::write(std::uint32_t reg, std::uint32_t value)
{
    switch (reg)
    {
    case AI_DRAM_ADDR_REG:
        std::printf("AI Write: AI_DRAM_ADDR_REG 0x%08x\n", value);
        //std::printf("Audio DMA ADDR: 0x%08x\n", value);
        _addr = value & 0x00fffff8;
        break;
    case AI_LEN_REG:
        std::printf("AI Write: AI_LEN_REG 0x%08x\n", value);
        //std::printf("Audio DMA: 0x%08x\n", value);
        //std::printf("AI Write: AI_LEN_REG\n");
        dma(value & 0x3fff8);
        break;
    case AI_CONTROL_REG:
        std::printf("AI Write: AI_CONTROL_REG 0x%08x\n", value);
        // TODO: Implement this
        break;
    case AI_STATUS_REG:
        std::printf("AI Write: AI_STATUS_REG 0x%08x\n", value);
        _mi.clearInterrupt(MI_INTR_AI);
        break;
    case AI_DACRATE_REG:
        std::printf("AI Write: AI_DACRATE_REG 0x%08x\n", value);
        // TODO: Implement this
        //std::printf("DAC Rate: 0x%08x\n", (48681812 / (value + 1)));
        //std::printf("AI Write: AI_DACRATE_REG\n");
        //std::exit(1);
        break;
    case AI_BITRATE_REG:
        std::printf("AI Write: AI_BITRATE_REG 0x%08x\n", value);
        // TODO: Implement this
        break;
    }
}

void AudioInterface::dma(std::uint32_t size)
{
    std::uint32_t dstSize;

    if (_bufCount == 2)
        return;
    _len[_bufCount++] = size;
    dstSize           = (size * 3) / 2;
    /* Swap and convert from 32kHz to 48kHz */
    for (std::uint32_t i = 0; i < size / 4; ++i)
    {
        _buffer[i * 6 + 0] = swap(*((std::uint16_t*)_memory.ram + _addr + i * 4 + 0));
        _buffer[i * 6 + 1] = swap(*((std::uint16_t*)_memory.ram + _addr + i * 4 + 1));
        _buffer[i * 6 + 2] = swap(*((std::uint16_t*)_memory.ram + _addr + i * 4 + 2));
        _buffer[i * 6 + 3] = swap(*((std::uint16_t*)_memory.ram + _addr + i * 4 + 3));
        _buffer[i * 6 + 4] = _buffer[i * 6 + 2];
        _buffer[i * 6 + 5] = _buffer[i * 6 + 3];
    }

    (*_callback)(_buffer, dstSize, _callbackArg);
}
