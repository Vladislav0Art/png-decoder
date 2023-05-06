# PNG Decoder

## Supported features:

---

The decoder conforms to the requirements of decoders mentioned in [standard PNG specification](http://www.libpng.org/pub/png/spec/1.2/PNG-Contents.html) supporting all the required **chunks** of the PNG format (i.e. `IHDR Image header`, `PLTE Palette`, `IDAT Image data`, and `IEND Image trailer`).

### Supported images:

- Grayscale images
- Regular RGB images
- Indexed images (i.e. with a fixed palette of colors)
- Images containing alpha-channel (transparency)

**Note:** `bit depth <= 8` supported. The entry point is `Image ReadPng(std::string_view filename)` function in `png_decoder.h`.



## Project details:

---

### The pipeline of decoding:

1. Read signature bytes and validate it.
1. Read chunks and validate its `CRC` until `IEND` chunk encountered.
1. Save the information provided by `IHDR` and `PLTE` chunks for future decoding use.
1. Concatenate the content of all `IDAT` chunks into a single byte vector (lets call it `V`).
1. Once `IEND` chunk reached, decoding of `V` starts: deflate to decompress `V` into another byte vector `D`.
1. Process `D` by **scanlines**, applying specified **filters**.
1. In case of interlaced image use [**Adam7 algorithm**](http://www.libpng.org/pub/png/spec/1.2/PNG-DataRep.html#DR.Image-layout) to decode the image.

### External libraries:

1. **zlib1g-dev (zlib)** - deflate algorithm for decompressing the image data.
1. **Boost** - CRC calculation.

### Implementation details:

1. Decoder is implemented as a class, receiving `std::istream&` (assuming binary mode) in it's constructor.
1. Deflate logic is completely separated from the decoder. Since **zlib1g-dev (zlib)** is a C libraries, there is a **RAII wrapper** written
around the library functionality (see [`Inflate`](./src/inflate/inflate.h)).
1. Error handling:
    1. CRC validation for ancillary chunks.
    1. Checking of EOF when reading from the input stream.
    1. Exceptions throwing for invalid png images.
1. In case of bit depth being less than 8 bits **bits reading** functionality is used which takes a sequence of bytes and reads it bitwise.



## Implementation:

---

### Defiltering data scanlines:

There are 3 main classes that are responsible for decoding the image after deflate algorithm is used: `ScanlineReader`, `Defilter`, and `PixelStrategy`.

`ScanlineReader` applies [defilters]((http://www.libpng.org/pub/png/spec/1.2/PNG-Filters.html)) to the scanlines to remove additional encoding layer from the actual image data, after that depending on the pixel storing format (which is stored inside `IHDR` chunk) a concrete implementation of abstract `PixelStrategy` used to convert scanline bytes into image pixels.

[Defilters](./src/defilter/defilter.h) are concrete implementations of abstact class `Defilter` that comform to **Applicable Design Pattern**: the caller (i.e. `ScanlineReader`) provides every scanline to the method `Defilter::applicable` which tells whether the currect defilter can be used in order to apply defiltering (it can be determined by a header stored in scanlines), in case of success the caller invokes `Defilter::apply`:

```cpp
// ScanlineReader workflow:
for (const auto& defilter : m_defilters) {
    if (defilter->applicable(scanline)) {
        defilter->apply(scanline, m_previousScanline, bpp);
    }
}
```


### Pixel recovery:

PNG format supports 3 main image formats: grayscale, RGB, and color pallete images, where the former two may also contain alpha-channel, which leads to having 5 completely different image formats.

In order to uniformly treat all the image formats the **Strategy Design Pattern** is applied: factory method [`PixelStrategy::create`](./src/scanline-reader/strategy/strategy.h) determines which image format is used in the current image and returns the concrete implementor of the image format, i.e. one of `PixelGrayscaleStrategy`, `PixelRGBStrategy`, `PixelPaletteIndexStrategy`, `PixelGrayscaleAlphaStrategy`, and `PixelRGBAlphaStrategy`:

```cpp
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
```