#pragma once

#include <arpa/inet.h>
#include <string>

#include "exceptions/exceptions.h"


namespace png_decoder::utils {

inline uint64_t convertFromBigEndianToHostEndianness(uint64_t x) {
    static bool areEndiannessesSame = (1 == ntohl(1));

    return (
        areEndiannessesSame
        ? x
        : ((uint64_t)ntohl((x) & 0xFFFFFFFF) << 32) | ntohl((x) >> 32)
    );
}

inline uint32_t convertFromBigEndianToHostEndianness(uint32_t x) {
    static bool areEndiannessesSame = (1 == ntohl(1));

    return (
        areEndiannessesSame
        ? x
        : ((uint32_t)ntohs((x) & 0xFFFFFFFF) << 16) | ntohs((x) >> 16)
    );
}

inline uint32_t convertFromBigEndianToHostEndianness(uint16_t x) {
    static bool areEndiannessesSame = (1 == ntohl(1));

    return (
        areEndiannessesSame
        ? (x)
        : (x << 8) | (x >> 8)
    );
}

inline std::string stringifyChunkType(uint32_t chunkType) {
    // `size + 1` to create '\0' terminated string
    char bytes[sizeof(chunkType) + 1] = {0};
    bytes[0] = (chunkType >> 24) & 0xFF;
    bytes[1] = (chunkType >> 16) & 0xFF;
    bytes[2] = (chunkType >> 8) & 0xFF;
    bytes[3] = chunkType & 0xFF;

    return std::string(bytes);
}

// read bytes from stream as big-endian and convert the value from big-endian to the endianess of the host machine
template <class T>
inline void readFromBigEndianAndConvertToHostEndianess(std::istream& stream, T* destination, size_t bytesCount, const std::string& errorMessage) {
    if (!stream.read(reinterpret_cast<char*>(destination), bytesCount)) {
        throw exceptions::InvalidStreamException(errorMessage);
    }

    *destination = convertFromBigEndianToHostEndianness(*destination);
}

} // namespace png_decoder::utils
