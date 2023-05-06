#include <cstdint>
#include <iostream>

#include "misc/structs.h"
#include "defilter.h"


namespace png_decoder::defilter {


uint32_t Defilter::mod(uint32_t value) {
    return (value % MOD + MOD) % MOD;
}

uint32_t Defilter::prev(const Scanline& scanline, uint32_t pos, uint32_t bpp) {
    return pos < bpp ? 0 : scanline.data[pos - bpp];
}


// defilters

// Sub
bool Sub::applicable(const Scanline& scanline) const {
    return scanline.filterMethod == static_cast<uint8_t>(FilterTypes::Sub);
}

void Sub::apply(Scanline& scanline, [[maybe_unused]] const Scanline& previousDefilteredScanline, uint32_t bpp) const {
    for (size_t i = 0; i < scanline.data.size(); ++i) {
        uint32_t previous = prev(scanline, i, bpp);
        uint32_t current = scanline.data[i];

        scanline.data[i] = mod(current + previous);
    }
}


// Up
bool Up::applicable(const Scanline& scanline) const {
    return scanline.filterMethod == static_cast<uint8_t>(FilterTypes::Up);
}

void Up::apply(Scanline& scanline, const Scanline& previousDefilteredScanline, [[maybe_unused]] uint32_t bpp) const {
    for (size_t i = 0; i < scanline.data.size(); ++i) {
        uint32_t prior = previousDefilteredScanline.data[i];
        uint32_t current = scanline.data[i];

        scanline.data[i] = mod(current + prior);
    }
}


// Average
bool Average::applicable(const Scanline& scanline) const {
    return scanline.filterMethod == static_cast<uint8_t>(FilterTypes::Average);
}

void Average::apply(Scanline& scanline, const Scanline& previousDefilteredScanline, uint32_t bpp) const {
    for (size_t i = 0; i < scanline.data.size(); ++i) {
        uint32_t previous = prev(scanline, i, bpp);
        uint32_t prior = previousDefilteredScanline.data[i];
        uint32_t current = scanline.data[i];

        scanline.data[i] = mod(current + (previous + prior) / 2);
    }
}


// Paeth
bool Paeth::applicable(const Scanline& scanline) const {
    return scanline.filterMethod == static_cast<uint8_t>(FilterTypes::Paeth);
}

void Paeth::apply(Scanline& scanline, const Scanline& previousDefilteredScanline, uint32_t bpp) const {
    for (size_t i = 0; i < scanline.data.size(); ++i) {
        uint32_t previous = prev(scanline, i, bpp);
        uint32_t prior = previousDefilteredScanline.data[i];
        uint32_t priorLeft = prev(previousDefilteredScanline, i, bpp);
        uint32_t current = scanline.data[i];

        scanline.data[i] = current + mod(predictor(previous, prior, priorLeft));
    }
}

int32_t Paeth::predictor(int32_t a, int32_t b, int32_t c) {
    // a = left, b = above, c = upper left
    int32_t p = a + b - c;
    int32_t pa = std::abs(p - a);
    int32_t pb = std::abs(p - b);
    int32_t pc = std::abs(p - c);

    // return nearest of a, b, c
    // breaking ties in order a, b, c
    if (pa <= pb && pa <= pc) return a;
    if (pb <= pc) return b;
    return c;
}

} // namespace png_decoder::defilter
