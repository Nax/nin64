#ifndef RDRAM_INTERFACE_H
#define RDRAM_INTERFACE_H

#include <cstdint>
#include <libnin64/NonCopyable.h>

namespace libnin64
{

class RDRAMInterface : private NonCopyable
{
public:
    RDRAMInterface();
    ~RDRAMInterface();

    std::uint32_t read(std::uint32_t reg);
    void          write(std::uint32_t reg, std::uint32_t value);

private:
    std::uint32_t _values[5];
};

} // namespace libnin64

#endif
