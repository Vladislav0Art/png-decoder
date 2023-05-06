#pragma once

#include <cstdint>
#include <vector>
#include <boost/crc.hpp>


namespace png_decoder::crc {

uint32_t computeCRCFrom(const std::vector<char>& bytes) {
    static constexpr uint64_t BITS_COUNT = 32;
    static constexpr uint64_t MASK = 0x4C11DB7;
    static constexpr uint64_t INIT_BITS = 0xFFFFFFFF;
    static constexpr uint64_t POST_XOR = 0xFFFFFFFF;
    static constexpr uint64_t LOWEST_TO_HIGHEST = true;
    static constexpr uint64_t REM_BEFORE_XOR = true;

    boost::crc_optimal<BITS_COUNT, MASK, INIT_BITS, POST_XOR, LOWEST_TO_HIGHEST, REM_BEFORE_XOR> crc;
    crc.process_bytes(bytes.data(), bytes.size());
    return crc.checksum();
}

} // namespace png_decoder::crc
