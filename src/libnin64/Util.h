#ifndef UTIL_H
#define UTIL_H

#include <cstdint>

namespace libnin64
{

    inline static constexpr std::uint8_t swap(std::uint8_t v)
    {
        return v;
    }

    inline static constexpr std::uint16_t swap(std::uint16_t v)
    {
        return (v << 8) | (v >> 8);
    }

    inline static constexpr std::uint32_t swap(std::uint32_t v)
    {
        return (v << 24) | ((v & 0xff00) << 8) | ((v & 0xff0000) >> 8) | (v >> 24);
    }

    inline static constexpr std::uint16_t swap16(std::uint16_t v) { return swap(v); }
    inline static constexpr std::uint32_t swap32(std::uint32_t v) { return swap(v); }

}

#endif
