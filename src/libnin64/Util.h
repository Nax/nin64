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

inline static constexpr std::uint64_t swap(std::uint64_t v)
{
    std::uint64_t out{};

    out |= ((v << 56) & 0xff00000000000000ULL);
    out |= ((v << 40) & 0x00ff000000000000ULL);
    out |= ((v << 24) & 0x0000ff0000000000ULL);
    out |= ((v << 8)  & 0x000000ff00000000ULL);
    out |= ((v >> 8)  & 0x00000000ff000000ULL);
    out |= ((v >> 24) & 0x0000000000ff0000ULL);
    out |= ((v >> 40) & 0x000000000000ff00ULL);
    out |= ((v >> 56) & 0x00000000000000ffULL);

    return out;
}

inline static constexpr std::uint16_t swap16(std::uint16_t v) { return swap(v); }
inline static constexpr std::uint32_t swap32(std::uint32_t v) { return swap(v); }
inline static constexpr std::uint64_t swap64(std::uint64_t v) { return swap(v); }

}

#endif
