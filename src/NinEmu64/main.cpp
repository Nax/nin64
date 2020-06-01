#include <AL/al.h>
#include <AL/alc.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <nin64/nin64.h>
#include <vector>

ALCcontext*         audioCtx;
ALCdevice*          audioDevice;
ALuint              audioSource;
std::vector<ALuint> audioBuffers;

std::FILE* audioOut;

static void displayError(Nin64Err err)
{
    switch (err)
    {
    case NIN64_ERROR_IO:
        std::puts("IO Error");
        break;
    case NIN64_ERROR_OUTOFMEMORY:
        std::puts("Memory Error");
        break;
    case NIN64_ERROR_BADROM:
        std::puts("Bad Rom");
        break;
    default:
        std::puts("Unknown Error");
        break;
    }
}

static void audioCallback(const std::uint16_t* samples, std::size_t count, void* arg)
{
    ALuint buffer;
    ALint  attr;

    alGetSourcei(audioSource, AL_BUFFERS_PROCESSED, &attr);
    for (int i = 0; i < attr; ++i)
    {
        //std::printf("GUI: Audio Done\n");
        alSourceUnqueueBuffers(audioSource, 1, &buffer);
        audioBuffers.push_back(buffer);
    }

    if (audioBuffers.empty())
    {
        //std::printf("GUI: Audio Full\n");
        return;
    }

    buffer = audioBuffers.back();
    audioBuffers.pop_back();

    alBufferData(buffer, AL_FORMAT_STEREO16, samples, count * 2, 48000);
    alSourceQueueBuffers(audioSource, 1, &buffer);
    alGetSourcei(audioSource, AL_SOURCE_STATE, &attr);
    if (attr != AL_PLAYING)
    {
        //std::printf("Not playing 0x%08x\n", attr);
        alSourcePlay(audioSource);
    }

    std::fwrite(samples, count * 2, 1, audioOut);
}

int main(int argc, char** argv)
{
    Nin64State*   state;
    Nin64Err      err;
    std::uint64_t count{};
    ALuint        buffers[4];

    audioDevice = alcOpenDevice(nullptr);
    audioCtx    = alcCreateContext(audioDevice, nullptr);
    alcMakeContextCurrent(audioCtx);

    audioOut = std::fopen("audio.bin", "wb");

    alGenSources(1, &audioSource);
    alGenBuffers(sizeof(buffers) / sizeof(buffers[0]), buffers);
    for (int i = 0; i < sizeof(buffers) / sizeof(buffers[0]); ++i)
    {
        audioBuffers.push_back(buffers[i]);
    }
    err = nin64CreateState(&state, argv[1]);
    nin64SetAudioCallback(state, &audioCallback, nullptr);
    if (err)
    {
        displayError(err);
        std::exit(1);
    }
    for (;;)
    {
        // printf("=================\n");
        // printf("CYCLES: %016llu\n", count);
        // printf("=================\n");
        //nin64RunCycles(state, 1024 * 127);
        //count += 1024 * 127;
        nin64RunFrame(state);
    }
    nin64DestroyState(state);

    return 0;
}
