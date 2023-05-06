#pragma once

#include <vector>
#include <cstdint>
#include <cstring>
#include <cassert>
#include <memory>

// misc
#include "misc/structs.h"
// defilter
#include "defilter/defilter.h"
// strategy
#include "strategy/strategy.h"

#include "image.h"



namespace png_decoder::scanline_reader {

class ScanlineReader {
public:
    ScanlineReader(uint32_t width,
                    uint32_t height,
                    uint8_t colorType,
                    uint8_t bitDepth,
                    PLTE palette,
                    const std::vector<unsigned char>& data);

    bool hasNext() const;
    std::vector<RGB> read();
    uint32_t getScanlineSize() const;

private:
    uint32_t getScanlineOffset() const;

private:
    static Scanline createEmptyScanline(uint32_t width);

private:
    static constexpr uint8_t PIXEL_GRAYSCALE_COLOR_TYPE = 0;
    static constexpr uint8_t PIXEL_RGB_COLOR_TYPE = 2;
    static constexpr uint8_t PIXEL_PALETTE_INDEX_COLOR_TYPE = 3;
    static constexpr uint8_t PIXEL_GRAYSCALE_ALPHA_COLOR_TYPE = 4;
    static constexpr uint8_t PIXEL_RGB_ALPHA_COLOR_TYPE = 6;

private:
    uint32_t m_width;
    uint32_t m_height;
    PLTE m_palette;
    const std::vector<unsigned char>& m_data;
    uint32_t m_row;
    Scanline m_previousScanline;
    std::vector<std::unique_ptr<defilter::Defilter>> m_defilters;
    std::unique_ptr<PixelStrategy> m_strategy;
};


} // namespace png_decoder::scanline_reader
