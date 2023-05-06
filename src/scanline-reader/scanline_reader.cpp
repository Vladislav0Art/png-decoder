#include <memory>

#include "scanline_reader.h"
#include "strategy/strategy.h"


namespace png_decoder::scanline_reader {

ScanlineReader::ScanlineReader(
        uint32_t width,
        uint32_t height,
        uint8_t colorType,
        uint8_t bitDepth,
        PLTE palette,
        const std::vector<unsigned char>& data)
    : m_width{width}
    , m_height{height}
    , m_palette(std::move(palette))
    , m_data{data}
    , m_row{0}
    , m_previousScanline{createEmptyScanline(width)}
    , m_defilters{}
    , m_strategy{PixelStrategy::create(colorType, bitDepth, m_palette)}
    {
        m_defilters.push_back(std::make_unique<defilter::Sub>());
        m_defilters.push_back(std::make_unique<defilter::Up>());
        m_defilters.push_back(std::make_unique<defilter::Average>());
        m_defilters.push_back(std::make_unique<defilter::Paeth>());
    }


bool ScanlineReader::hasNext() const {
    return m_row < m_height;
}


std::vector<RGB> ScanlineReader::read() {
    // reading filter method
    const uint32_t scanlineOffset = getScanlineOffset();
    Scanline scanline{};
    std::memcpy(&scanline.filterMethod, &m_data[scanlineOffset], sizeof(scanline.filterMethod));

    // reading data bytes into scanline
    const uint32_t scanlineSize = getScanlineSize();
    scanline.data.resize(scanlineSize);
    std::memcpy(scanline.data.data(), &m_data[scanlineOffset + sizeof(scanline.filterMethod)], scanlineSize);

    // defiltering scanline
    const uint32_t bpp = m_strategy->bpp();
    for (const auto& defilter : m_defilters) {
        if (defilter->applicable(scanline)) {
            defilter->apply(scanline, m_previousScanline, bpp);
        }
    }

    std::vector<RGB> pixels;

   for (size_t i = 0; i < m_width; ++i) {
        RGB pixel = m_strategy->pixelAt(scanline, i);
        pixels.push_back(pixel);
   }

    // getting to the next row
    ++m_row;
    // updating previous scanline
    m_previousScanline = scanline;

    return pixels;
}


uint32_t ScanlineReader::getScanlineOffset() const {
    uint32_t scanlineSize = getScanlineSize();
    return m_row * (scanlineSize + sizeof(Scanline::filterMethod));
}


uint32_t ScanlineReader::getScanlineSize() const {
    uint32_t sizeBits = m_strategy->samplesCount() * m_strategy->sampleSizeBits() * m_width;
    uint32_t sizeBytes = (sizeBits / 8) + (sizeBits % 8 != 0);
    return sizeBytes;
}


Scanline ScanlineReader::createEmptyScanline(uint32_t width) {
    Scanline scanline{};
    scanline.data.resize(width, 0);
    return scanline;
}


} // namespace png_decoder::scanline_reader
