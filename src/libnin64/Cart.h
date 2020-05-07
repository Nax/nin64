#ifndef INCLUDED_CART_H
#define INCLUDED_CART_H

#include <cstdint>
#include <nin64/nin64.h>
#include <libnin64/CIC.h>
#include <libnin64/NonCopyable.h>

namespace libnin64
{

class Cart : private NonCopyable
{
public:
    Cart();
    ~Cart();

    CIC         cic() const;
    void        read(std::uint8_t* dst, std::uint32_t offset, std::uint32_t size);
    Nin64Err    load(const char* path);

private:
    std::uint8_t* _data;
    std::uint32_t   _size;
};

}

#endif
