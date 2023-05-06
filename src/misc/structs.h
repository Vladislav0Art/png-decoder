#pragma once

#include <vector>
#include <cstdint>

namespace png_decoder {

#pragma pack(push, 1)
struct IHDR {
    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t bitDepth = 0;
    uint8_t colorType = 0;
    uint8_t compressionMethod = 0;
    uint8_t filterMethod = 0;
    uint8_t interlaceMethod = 0;
};
static_assert(sizeof(IHDR) == 13);
#pragma pack(pop)

struct PLTE {
    struct rgb {
        uint8_t red = 0;
        uint8_t green = 0;
        uint8_t blue = 0;
    };

    uint8_t length;
    std::vector<rgb> palette;
};

struct Chunk {
    uint32_t length = 0;
    uint32_t type = 0;
    std::vector<unsigned char> data{};
    uint32_t crc = 0;
};

struct Scanline {
    uint8_t filterMethod = 0;
    std::vector<unsigned char> data{};
};

} // namespace png_decoder
