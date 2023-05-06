#pragma once

#include <cstdint>

#include "misc/structs.h"

namespace png_decoder::defilter {

// Refer to: http://www.libpng.org/pub/png/spec/1.2/PNG-Filters.html

class Defilter {
public:
    virtual bool applicable(const Scanline& scanline) const = 0;
    /* bpp - number of bytes per complete pixel */
    virtual void apply(Scanline& scanline, const Scanline& previousDefilteredScanline, uint32_t bpp) const = 0;
    virtual ~Defilter() = default;

protected:
    enum class FilterTypes : uint8_t {
        None = 0,
        Sub,
        Up,
        Average,
        Paeth,
    };

    static uint32_t mod(uint32_t value);
    static uint32_t prev(const Scanline& scanline, uint32_t pos, uint32_t bpp);

private:
    static constexpr uint32_t MOD = 256;
};


// defilters
class Sub : public Defilter {
public:
    bool applicable(const Scanline& scanline) const override;
    void apply(Scanline& scanline, const Scanline& previousDefilteredScanline, uint32_t bpp) const override;
};


class Up : public Defilter {
public:
    bool applicable(const Scanline& scanline) const override;
    void apply(Scanline& scanline, const Scanline& previousDefilteredScanline, uint32_t bpp) const override;
};


class Average : public Defilter {
public:
    bool applicable(const Scanline& scanline) const override;
    void apply(Scanline& scanline, const Scanline& previousDefilteredScanline, uint32_t bpp) const override;
};


class Paeth : public Defilter {
public:
    bool applicable(const Scanline& scanline) const override;
    void apply(Scanline& scanline, const Scanline& previousDefilteredScanline, uint32_t bpp) const override;

private:
    static int32_t predictor(int32_t a, int32_t b, int32_t c);
};


} // namespace png_decoder::defilter
