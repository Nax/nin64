#include <algorithm>
#include <cstdio>
#include <libnin64/Cart.h>
#include <libnin64/Util.h>

using namespace libnin64;

inline static constexpr std::uint32_t swapWords(std::uint32_t v)
{
    return swap16(v & 0xffff) | (swap16(v >> 16) << 16);
}

Cart::Cart()
: _data{}, _size{}
{
}

Cart::~Cart()
{
    delete[] _data;
}

void Cart::read(std::uint8_t *dst, std::uint32_t offset, std::uint32_t size)
{
    std::copy(_data + offset, _data + offset + size, dst);
}

Nin64Err Cart::load(const char *path)
{
    std::FILE *file;
    std::uint32_t magic{};
    bool byteSwap;
    bool wordSwap;

    /* If there was already a cart loaded, unload it properly */
    delete[] _data;
    _data = nullptr;
    _size = 0;

    file = std::fopen(path, "rb");
    if (!file)
    {
        return NIN64_ERROR_IO;
    }

    /* Detect the correct endianess */
    std::fread(&magic, 4, 1, file);
    switch (magic & 0xff)
    {
    case 0x80:
        /* Big-Endian ROM */
        byteSwap = false;
        wordSwap = false;
        break;
    case 0x37:
        /* Little-Endian ROM */
        byteSwap = false;
        wordSwap = true;
        break;
    case 0x40:
        /* Middle-Endian ROM */
        byteSwap = true;
        wordSwap = false;
        break;
    case 0x12:
        /* Alternative Middle-Endian ROM - very rare */
        byteSwap = true;
        wordSwap = true;
        break;
    default:
        std::fclose(file);
        return NIN64_ERROR_BADROM;
    }

    /* Fix up the magic */
    if (byteSwap)
    {
        magic = swap32(magic);
    }
    if (wordSwap)
    {
        magic = swapWords(magic);
    }

    /* Check the fixed up magic */
    if (magic != 0x40123780)
    {
        std::fclose(file);
        return NIN64_ERROR_BADROM;
    }

    /* Get the ROM size */
    std::fseek(file, 0, SEEK_END);
    _size = std::ftell(file);

    /* Read the ROM */
    std::fseek(file, 0, SEEK_SET);
    _data = new std::uint8_t[_size];
    std::fread(_data, _size, 1, file);
    std::fclose(file);

    /* Byte swap */
    if (byteSwap)
    {
        for (std::uint32_t i = 0; i < _size / 4; ++i)
        {
            ((std::uint32_t *)_data)[i] = swap32(((std::uint32_t *)_data)[i]);
        }
    }

    /* Word swap */
    if (wordSwap)
    {
        for (std::uint32_t i = 0; i < _size / 4; ++i)
        {
            ((std::uint32_t *)_data)[i] = swapWords(((std::uint32_t *)_data)[i]);
        }
    }

    return NIN64_OK;
}
