#include <string_view>
#include <cassert>
#include <iostream>
#include <istream>
#include <fstream>
#include <cstring>

#include "png_decoder.h"
#include "exceptions/exceptions.h"
#include "scanline-reader/scanline_reader.h"
#include "utils/utils.h"
#include "inflate/inflate.h"

// misc
#include "misc/crc.h"
#include "image.h"



namespace png_decoder {

PNGDecoder::PNGDecoder(std::istream& stream) {
    // reading signature
    uint64_t signature;

    utils::readFromBigEndianAndConvertToHostEndianess(
        stream, &signature, sizeof(signature), PNG_DECODER_ERROR_MESSAGE("Cannot read signature"));

    validateSignature(signature);

    // reading IHDR
    Chunk ihdrChunk = readChunk(stream);
    storeIHDR(ihdrChunk);

    // reading other chunks
    bool stop = false;
    while (!stop) {
        Chunk chunk = readChunk(stream);

        if (isIEND(chunk.type)) {
            if (stream.peek() != std::ifstream::traits_type::eof()) {
                throw exceptions::InvalidIENDChunkException();
            }
            stop = true;
        }
        else if (isIDAT(chunk.type)) {
            storeIDAT(chunk);
        }
        else if (isPLTE(chunk.type)) {
            storePLTE(chunk);
        }
        else {
            // TODO: throw exceptions::CriticalChunkTypeChunkException if critical chunk type
            // unsupported chunk type
            // throw exceptions::CriticalChunkTypeChunkException(
            //     PNG_DECODER_ERROR_MESSAGE("Unsupported critical chunk with type " + utils::stringifyChunkType(chunk.type) + std::to_string(chunk.type)));
        }
    }

    inflate::Inflate inflateWrapper{};
    // replacing with inflated data
    m_data = inflateWrapper.doInflate(m_data);
}

Image PNGDecoder::createImage() const {
    Image image(m_ihdr.height, m_ihdr.width);

    if (m_ihdr.interlaceMethod == NULL_INTERLACING_METHOD) {
        fillImageNullInterlace(image);
    }
    else if (m_ihdr.interlaceMethod == ADAM7_INTERLACING_METHOD) {
        fillImageAdam7Interlace(image);
    }
    else {
        throw exceptions::DecodingException(
            PNG_DECODER_ERROR_MESSAGE("Invalid interlace method: " + std::to_string(m_ihdr.interlaceMethod)));
    }

    return image;
}

// methods
void PNGDecoder::fillImageAdam7Interlace(Image& image) const {
    // define the starting column, starting row, column increment, and row increment for each pass
    uint32_t starting_col[]  = {0, 4, 0, 2, 0, 1, 0};
    uint32_t starting_row[]  = {0, 0, 4, 0, 2, 0, 1};
    uint32_t col_increment[] = {8, 8, 4, 4, 2, 2, 1};
    uint32_t row_increment[] = {8, 8, 8, 4, 4, 2, 2};

    // calculate the size and position of each pass
    const size_t PASSES_COUNT = 7;

    uint32_t pass_width[PASSES_COUNT] = {0};
    uint32_t pass_height[PASSES_COUNT] = {0};

    for (size_t i = 0; i < PASSES_COUNT; i++) {
        pass_width[i]  = (m_ihdr.width  + col_increment[i] - starting_col[i] - 1) / col_increment[i];
        pass_height[i] = (m_ihdr.height + row_increment[i] - starting_row[i] - 1) / row_increment[i];
    }

    std::vector<std::vector<unsigned char>> passes(PASSES_COUNT);

    uint32_t offset = 0;
    for (size_t i = 0; i < PASSES_COUNT; ++i) {
        uint32_t width = pass_width[i];
        uint32_t height = pass_height[i];

        // creating reader to determine scanline size
        scanline_reader::ScanlineReader reader(width, height, m_ihdr.colorType, m_ihdr.bitDepth, m_plte, passes[i]);
        uint32_t length = (1 + reader.getScanlineSize()) * height;
        passes[i].resize(length);

        // reading reduced image
        std::memcpy(passes[i].data(), m_data.data() + offset, length);
        offset += length;
    }

    for (int i = 0; i < 7; ++i) {
        uint32_t width = pass_width[i];
        uint32_t height = pass_height[i];

        /*
        * See: http://www.libpng.org/pub/png/spec/1.2/PNG-DataRep.html#DR.Image-layout
        * If the image contains fewer than five columns or fewer than five rows,
        * some passes will be entirely empty.
        */
        if (width == 0 || height == 0) {
            continue;
        }

        scanline_reader::ScanlineReader reader(width, height, m_ihdr.colorType, m_ihdr.bitDepth, m_plte, passes[i]);

        size_t row = 0;
        while(reader.hasNext()) {
            size_t fullRow = row * row_increment[i] + starting_row[i];
            std::vector<RGB> pixels = reader.read();

            for (size_t col = 0; col < pixels.size(); ++col) {
                // calculate the position of this pixel in the full image
                size_t fullCol = col * col_increment[i] + starting_col[i];
                image(fullRow, fullCol) = pixels[col];
            }
            ++row;
        }
    }
}


void PNGDecoder::fillImageNullInterlace(Image& image) const {
    scanline_reader::ScanlineReader reader(m_ihdr.width, m_ihdr.height, m_ihdr.colorType, m_ihdr.bitDepth, m_plte, m_data);
    size_t row = 0;
    while(reader.hasNext()) {
        std::vector<RGB> pixels = reader.read();
        for (size_t col = 0; col < pixels.size(); ++col) {
            image(row, col) = pixels[col];
        }
        ++row;
    }
}


void PNGDecoder::storeIHDR(const Chunk& ihdrChunk) {
    validateIHDR(ihdrChunk.type);

    // copying fields
    assert(ihdrChunk.length == sizeof(m_ihdr));
    std::memcpy(&m_ihdr, ihdrChunk.data.data(), ihdrChunk.length);
    m_ihdr.width = utils::convertFromBigEndianToHostEndianness(m_ihdr.width);
    m_ihdr.height = utils::convertFromBigEndianToHostEndianness(m_ihdr.height);
}

void PNGDecoder::storeIDAT(const Chunk& idatChunk) {
    m_data.reserve(m_data.size() + idatChunk.length);
    for (auto byte : idatChunk.data) {
        m_data.push_back(byte);
    }
}

void PNGDecoder::storePLTE(const Chunk& plteChunk) {
    // copying fields
    if (!(plteChunk.data.size() % 3 == 0 && plteChunk.length <= 256)) {
        throw exceptions::InvalidPLTEChunkException();
    }

    m_plte.length = plteChunk.length;
    for (size_t i = 0; i < plteChunk.length; i += 3) {
        PLTE::rgb rgb{};

        std::memcpy(&rgb.red, &plteChunk.data[i], sizeof(rgb.red));
        std::memcpy(&rgb.green, &plteChunk.data[i + 1], sizeof(rgb.green));
        std::memcpy(&rgb.blue, &plteChunk.data[i + 2], sizeof(rgb.blue));

        m_plte.palette.push_back(std::move(rgb));
    }
}


// static methods
void PNGDecoder::validateSignature(uint64_t signature) {
    if (signature != PNGDecoder::PNG_SIGNATURE) {
        throw exceptions::InvalidSignatureException();
    }
}


void PNGDecoder::validateIHDR(std::uint32_t chunkType) {
    if (chunkType != PNGDecoder::IHDR_CHUNK_TYPE) {
        throw exceptions::InvalidIHDRChunkException();
    }
}

void PNGDecoder::validateCRC(uint32_t actual, uint32_t expected, uint32_t chunkType) {
    if (actual != expected) {
        std::string message = "Invalid CRC chunk type '" + utils::stringifyChunkType(chunkType) +
                "': actual=" + std::to_string(actual) + ", expected=" + std::to_string(expected);
        throw exceptions::InvalidCRCException(PNG_DECODER_ERROR_MESSAGE(message));
    }
}


Chunk PNGDecoder::readChunk(std::istream& stream) {
    Chunk chunk;

    // reading length
    utils::readFromBigEndianAndConvertToHostEndianess(
        stream,
        &chunk.length,
        sizeof(chunk.length),
        PNG_DECODER_ERROR_MESSAGE("Cannot read chunk length")
    );

    // reading type and data for following crc checking
    uint32_t computedCRC = 0;
    {
        size_t bufferSize = sizeof(chunk.type) + chunk.length;
        std::vector<char> buffer(bufferSize);
        if (!stream.read(buffer.data(), buffer.size())) {
            throw exceptions::InvalidStreamException(
                PNG_DECODER_ERROR_MESSAGE("Cannot read chunk type and data into buffer"));
        }

        // computing crc
        computedCRC = crc::computeCRCFrom(buffer);

        // moving stream back
        stream.seekg(-1UL * bufferSize, stream.cur);
    }

    // reading type
    utils::readFromBigEndianAndConvertToHostEndianess(
        stream,
        &chunk.type,
        sizeof(chunk.type),
        PNG_DECODER_ERROR_MESSAGE("Cannot read chunk type")
    );

    // reading chunk data
    chunk.data.resize(chunk.length);
    for (size_t i = 0; i < chunk.length; ++i) {
        if (!stream.read(reinterpret_cast<char*>(&chunk.data[i]), sizeof(chunk.data[i]))) {
            throw exceptions::InvalidStreamException(
                PNG_DECODER_ERROR_MESSAGE("Cannot read chunk data at position " + std::to_string(i)));
        }
    }

    // reading crc
    utils::readFromBigEndianAndConvertToHostEndianess(
        stream,
        &chunk.crc,
        sizeof(chunk.crc),
        PNG_DECODER_ERROR_MESSAGE("Cannot read chunk crc")
    );

    // validating actual crc against chunk crc
    validateCRC(computedCRC, chunk.crc, chunk.type);

    return chunk;
}


bool PNGDecoder::isIEND(uint32_t chunkType) noexcept {
    return chunkType == PNGDecoder::IEND_CHUNK_TYPE;
}

bool PNGDecoder::isIDAT(uint32_t chunkType) noexcept {
    return chunkType == PNGDecoder::IDAT_CHUNK_TYPE;
}

bool PNGDecoder::isPLTE(uint32_t chunkType) noexcept {
    return chunkType == PNGDecoder::PLTE_CHUNK_TYPE;
}


} // namespace png_decoder


Image ReadPng(std::string_view filename) {
    std::string path(filename);
    std::fstream file(path);

    png_decoder::PNGDecoder decoder(file);
    return decoder.createImage();
}