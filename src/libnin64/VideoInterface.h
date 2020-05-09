#ifndef INCLUDED_VIDEO_INTERFACE_H
#define INCLUDED_VIDEO_INTERFACE_H

#include <cstddef>
#include <cstdint>
#include <libnin64/NonCopyable.h>

namespace libnin64
{

class MIPSInterface;
class VideoInterface : private NonCopyable
{
public:
    VideoInterface(MIPSInterface& mi);
    ~VideoInterface();

    void          setVBlank();
    std::uint32_t read(std::uint32_t reg);
    void          write(std::uint32_t reg, std::uint32_t value);

private:
    MIPSInterface& _mi;
};

} // namespace libnin64

#endif
