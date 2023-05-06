#include <cstring>
#include <string>
#include <cassert>

#include "strategy.h"
#include "exceptions/exceptions.h"



namespace png_decoder::scanline_reader {


PixelStrategy::PixelStrategy(uint8_t bitDepth, PLTE plte)
    : m_bitDepth{bitDepth}
    , m_plte{std::move(plte)} {}


uint32_t PixelStrategy::bpp() const {
    uint32_t samples = samplesCount();
    uint32_t result = std::max<uint32_t>(1, samples * m_bitDepth / 8);
    return result;
}

uint32_t PixelStrategy::sampleSizeBits() const noexcept {
    return m_bitDepth;
}


std::unique_ptr<PixelStrategy> PixelStrategy::create(uint8_t colorType, uint8_t bitDepth, PLTE plte) {
    if (colorType == PIXEL_GRAYSCALE_COLOR_TYPE) {
        return std::make_unique<PixelGrayscaleStrategy>(bitDepth, std::move(plte));
    }
    else if (colorType == PIXEL_RGB_COLOR_TYPE) {
        return std::make_unique<PixelRGBStrategy>(bitDepth, std::move(plte));
    }
    else if (colorType == PIXEL_PALETTE_INDEX_COLOR_TYPE) {
        return std::make_unique<PixelPaletteIndexStrategy>(bitDepth, std::move(plte));
    }
    else if (colorType == PIXEL_GRAYSCALE_ALPHA_COLOR_TYPE) {
        return std::make_unique<PixelGrayscaleAlphaStrategy>(bitDepth, std::move(plte));
    }
    else if (colorType == PIXEL_RGB_ALPHA_COLOR_TYPE) {
        return std::make_unique<PixelRGBAlphaStrategy>(bitDepth, std::move(plte));
    }

    throw exceptions::InvalidColorTypeChunkException(
        PNG_DECODER_ERROR_MESSAGE("Unsupported color type in IHDR: " + std::to_string(colorType)));
}


// PixelBitsMixin
uint8_t PixelBitsMixin::getPixelBits(const Scanline& scanline, size_t index, uint32_t sampleSizeBits) const {
    uint32_t k = sampleSizeBits; // size of pixel (== sample) in bits
    uint32_t m = 8 / k; // count of pixels in single char (== block)
    assert(m > 0);
    uint32_t block = index / m; // block index of pixel
    uint32_t pos = index % m; // position of pixel in itr block
    unsigned char c = scanline.data[block]; // block bits
    uint32_t mask = (1 << k) - 1;

    /*
    * See: http://www.libpng.org/pub/png/spec/1.2/PNG-DataRep.html#DR.Image-layout
    * Pixels smaller than a byte never cross byte boundaries;
    * they are packed into bytes with the leftmost pixel in the high-order bits of a byte,
    * the rightmost in the low-order bits.
    */
   uint32_t pixelBits = (c >> (k * (m - 1 - pos))) & mask;
   assert(pixelBits < (1 << 8));
   return pixelBits;
}



// PixelGrayscaleStrategy
PixelGrayscaleStrategy::PixelGrayscaleStrategy(uint8_t bitDepth, PLTE plte) : PixelStrategy(bitDepth, std::move(plte)) {}

uint32_t PixelGrayscaleStrategy::samplesCount() const noexcept {
    return 1;
}

RGB PixelGrayscaleStrategy::pixelAt(const Scanline& scanline, size_t index) const {
    RGB pixel{};
    unsigned char color;

    // Note: sample == pixel since there is a single sample
    uint8_t pixelBits = getPixelBits(scanline, index, sampleSizeBits());
    color = pixelBits;

    pixel.r = color;
    pixel.g = color;
    pixel.b = color;
    pixel.a = 255;

    return pixel;
}


// PixelRGBStrategy
PixelRGBStrategy::PixelRGBStrategy(uint8_t bitDepth, PLTE plte) : PixelStrategy(bitDepth, std::move(plte)) {}

uint32_t PixelRGBStrategy::samplesCount() const noexcept {
    return 3;
}

RGB PixelRGBStrategy::pixelAt(const Scanline& scanline, size_t index) const {
    RGB pixel{};

    unsigned char red;
    unsigned char green;
    unsigned char blue;

    // since only bitDepth <= 8 is supported (i.e. each sample has size of one byte)
    size_t position = index * samplesCount();
    std::memcpy(&red, &scanline.data[position], sizeof(red));
    std::memcpy(&green, &scanline.data[position + sizeof(red)], sizeof(green));
    std::memcpy(&blue, &scanline.data[position + sizeof(red) + sizeof(green)], sizeof(blue));

    pixel.r = red;
    pixel.g = green;
    pixel.b = blue;
    pixel.a = 255;

    return pixel;
}


// PixelPaletteIndexStrategy
PixelPaletteIndexStrategy::PixelPaletteIndexStrategy(uint8_t bitDepth, PLTE plte) : PixelStrategy(bitDepth, std::move(plte)) {}

uint32_t PixelPaletteIndexStrategy::samplesCount() const noexcept {
    return 1;
}

RGB PixelPaletteIndexStrategy::pixelAt(const Scanline& scanline, size_t index) const {
    // since samples count is one index is already correct
    uint8_t paletteIndex = getPixelBits(scanline, index, sampleSizeBits());
    assert(paletteIndex < m_plte.palette.size());

    PLTE::rgb color = m_plte.palette[paletteIndex];
    RGB pixel{};

    pixel.r = color.red;
    pixel.g = color.green;
    pixel.b = color.blue;
    pixel.a = 255;

    return pixel;
}


// PixelGrayscaleAlphaStrategy
PixelGrayscaleAlphaStrategy::PixelGrayscaleAlphaStrategy(uint8_t bitDepth, PLTE plte) : PixelStrategy(bitDepth, std::move(plte)) {}

uint32_t PixelGrayscaleAlphaStrategy::samplesCount() const noexcept {
    return 2;
}

RGB PixelGrayscaleAlphaStrategy::pixelAt(const Scanline& scanline, size_t index) const {
    RGB pixel{};

    unsigned char grayscale;
    unsigned char alpha;

    /*
    * See: http://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html
    * Only supporting bit depth of 8 bits
    */
    size_t position = index * samplesCount();
    std::memcpy(&grayscale, &scanline.data[position], sizeof(grayscale));
    std::memcpy(&alpha, &scanline.data[position + sizeof(grayscale)], sizeof(alpha));

    pixel.r = grayscale;
    pixel.g = grayscale;
    pixel.b = grayscale;
    pixel.a = alpha;

    return pixel;
}


// PixelRGBAlphaStrategy
PixelRGBAlphaStrategy::PixelRGBAlphaStrategy(uint8_t bitDepth, PLTE plte) : PixelStrategy(bitDepth, std::move(plte)) {}

uint32_t PixelRGBAlphaStrategy::samplesCount() const noexcept {
    return 4;
}

RGB PixelRGBAlphaStrategy::pixelAt(const Scanline& scanline, size_t index) const {
    RGB pixel{};

    unsigned char red;
    unsigned char green;
    unsigned char blue;
    unsigned char alpha;

    /*
    * See: http://www.libpng.org/pub/png/spec/1.2/PNG-Chunks.html
    * Only supporting bit depth of <= 8 bits
    */
    size_t position = index * samplesCount();
    // reading colors
    std::memcpy(&red, &scanline.data[position], sizeof(red));
    std::memcpy(&green, &scanline.data[position + sizeof(red)], sizeof(green));
    std::memcpy(&blue, &scanline.data[position + sizeof(red) + sizeof(green)], sizeof(blue));

    // reading alpha
    std::memcpy(&alpha, &scanline.data[position + sizeof(red) + sizeof(green) + sizeof(blue)], sizeof(alpha));

    pixel.r = red;
    pixel.g = green;
    pixel.b = blue;
    pixel.a = alpha;

    return pixel;
}


} // namespace png_decoder::scanline_reader