#ifndef INCLUDED_NON_COPYABLE_H
#define INCLUDED_NON_COPYABLE_H

namespace libnin64
{

class NonCopyable
{
public:
    NonCopyable() {}
    ~NonCopyable() {}

    NonCopyable(const NonCopyable& rhs) = delete;
    NonCopyable& operator=(const NonCopyable& rhs) = delete;
};

}

#endif
