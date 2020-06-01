#ifndef NIN64_H
#define NIN64_H

#include <nin64/_abi.h>
#include <stddef.h>
#include <stdint.h>

typedef struct Nin64State Nin64State;
typedef enum
{
    NIN64_OK                = 0,
    NIN64_ERROR_OUTOFMEMORY = 1,
    NIN64_ERROR_IO          = 2,
    NIN64_ERROR_BADROM      = 3
} Nin64Err;
typedef void (*Nin64AudioCallback)(const uint16_t*, size_t, void*);

NIN64_API Nin64Err nin64CreateState(Nin64State** dst, const char* romPath);
NIN64_API Nin64Err nin64DestroyState(Nin64State* state);
NIN64_API Nin64Err nin64RunCycles(Nin64State* state, size_t count);
NIN64_API Nin64Err nin64RunFrame(Nin64State* state);
NIN64_API Nin64Err nin64SetAudioCallback(Nin64State* state, Nin64AudioCallback callback, void* callbackArg);

#endif
