#pragma once

#include <memory>

#include "misc/structs.h"
#include "image.h"


namespace png_decoder::scanline_reader {


class PixelStrategy {
public:
    PixelStrategy(uint8_t bitDepth, PLTE plte);
    virtual ~PixelStrategy() = default;

    uint32_t bpp() const;
    uint32_t sampleSizeBits() const noexcept;

    virtual RGB pixelAt(const Scanline& scanline, size_t index) const = 0;
    virtual uint32_t samplesCount() const noexcept = 0;

public:
    static std::unique_ptr<PixelStrategy> create(uint8_t colorType, uint8_t bitDepth, PLTE plte);

private:
    static constexpr uint8_t PIXEL_GRAYSCALE_COLOR_TYPE = 0;
    static constexpr uint8_t PIXEL_RGB_COLOR_TYPE = 2;
    static constexpr uint8_t PIXEL_PALETTE_INDEX_COLOR_TYPE = 3;
    static constexpr uint8_t PIXEL_GRAYSCALE_ALPHA_COLOR_TYPE = 4;
    static constexpr uint8_t PIXEL_RGB_ALPHA_COLOR_TYPE = 6;

protected:
    uint8_t m_bitDepth;
    PLTE m_plte;
};


class PixelBitsMixin {
protected:
    /*
    * Extract bits of pixel with bitDepth <= 8 from scanline data.
    * This is valid because grayscale color type has exactly one sample
    * which leads to the fact of pixel being equivalent to the sample.
    */
    uint8_t getPixelBits(const Scanline& scanline, size_t index, uint32_t sampleSizeBits) const;
};


// PixelGrayscaleStrategy
class PixelGrayscaleStrategy : public PixelStrategy, private PixelBitsMixin {
public:
    PixelGrayscaleStrategy(uint8_t bitDepth, PLTE plte);
    RGB pixelAt(const Scanline& scanline, size_t index) const override;
    uint32_t samplesCount() const noexcept override;
};


// PixelRGBStrategy
class PixelRGBStrategy : public PixelStrategy {
public:
    PixelRGBStrategy(uint8_t bitDepth, PLTE plte);
    RGB pixelAt(const Scanline& scanline, size_t index) const override;
    uint32_t samplesCount() const noexcept override;
};


// PixelPaletteIndexStrategy
class PixelPaletteIndexStrategy : public PixelStrategy, private PixelBitsMixin {
public:
    PixelPaletteIndexStrategy(uint8_t bitDepth, PLTE plte);
    RGB pixelAt(const Scanline& scanline, size_t index) const override;
    uint32_t samplesCount() const noexcept override;
};


// PixelGrayscaleAlphaStrategy
class PixelGrayscaleAlphaStrategy : public PixelStrategy {
public:
    PixelGrayscaleAlphaStrategy(uint8_t bitDepth, PLTE plte);
    RGB pixelAt(const Scanline& scanline, size_t index) const override;
    uint32_t samplesCount() const noexcept override;
};


// PixelRGBAlphaStrategy
class PixelRGBAlphaStrategy : public PixelStrategy {
public:
    PixelRGBAlphaStrategy(uint8_t bitDepth, PLTE plte);
    RGB pixelAt(const Scanline& scanline, size_t index) const override;
    uint32_t samplesCount() const noexcept override;
};



} // namespace png_decoder::scanline_reader
