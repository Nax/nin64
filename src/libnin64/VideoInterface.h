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
    void          tick(std::size_t count);

private:
    MIPSInterface& _mi;

    bool          _sync : 1;
    std::uint32_t _origin;
    std::uint16_t _scanline;
    std::uint16_t _scanlineSync;
    std::size_t   _acc;
};

} // namespace libnin64

#endif
