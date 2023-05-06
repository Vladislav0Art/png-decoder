#pragma once

#include <string_view>
#include <istream>
#include <string>
#include <vector>

#include "misc/structs.h"
#include "image.h"


namespace png_decoder {

class PNGDecoder {
public:
    PNGDecoder(std::istream& stream);
    Image createImage() const;

private:
    void storeIHDR(const Chunk& ihdrChunk);
    void storeIDAT(const Chunk& idatChunk);
    void storePLTE(const Chunk& plteChunk);
    void fillImageNullInterlace(Image& image) const;
    void fillImageAdam7Interlace(Image& image) const;


private:
    static void validateSignature(uint64_t signature);
    static void validateIHDR(uint32_t chunkType);
    static void validateCRC(uint32_t actual, uint32_t expected, uint32_t chunkType);
    static Chunk readChunk(std::istream& stream);
    static bool isIEND(uint32_t chunkType) noexcept;
    static bool isIDAT(uint32_t chunkType) noexcept;
    static bool isPLTE(uint32_t chunkType) noexcept;

private:
    static constexpr uint64_t PNG_SIGNATURE = 0x89504E470D0A1A0A; // 137 80 78 71 13 10 26 10
    static constexpr uint32_t IHDR_CHUNK_TYPE = 0x49484452UL; // 73 72 68 82
    static constexpr uint32_t PLTE_CHUNK_TYPE = 0x504c5445UL; // 80 76 84 69
    static constexpr uint32_t IDAT_CHUNK_TYPE = 0x49444154UL; // 73 68 65 84
    static constexpr uint32_t IEND_CHUNK_TYPE = 0x49454e44UL; // 73 69 78 68

    static constexpr uint32_t NULL_INTERLACING_METHOD = 0;
    static constexpr uint32_t ADAM7_INTERLACING_METHOD = 1;
private:
    IHDR m_ihdr;
    PLTE m_plte;
    std::vector<unsigned char> m_data;
};


}; // namespace png_decoder


Image ReadPng(std::string_view filename);