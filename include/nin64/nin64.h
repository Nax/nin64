#ifndef NIN64_H
#define NIN64_H

#include <nin64/_abi.h>

typedef struct Nin64State Nin64State;
typedef enum
{
    NIN64_OK = 0,
} Nin64Err;

NIN64_API Nin64Err nin64CreateState(Nin64State** dst, const char* romPath);
NIN64_API Nin64Err nin64DestroyState(Nin64State* state);

#endif
