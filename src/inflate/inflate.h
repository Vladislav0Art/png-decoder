#pragma once

#include <cstdint>
#include <vector>
#include <zlib.h>

#include "exceptions/exceptions.h"


namespace png_decoder::inflate {

class Inflate {
public:
    Inflate();
    ~Inflate();

    std::vector<unsigned char> doInflate(const std::vector<unsigned char>& source);

private:
    /* Decompress from source to dest.
    inf() returns Z_OK on success, Z_MEM_ERROR if memory could not be
    allocated for processing, Z_DATA_ERROR if the deflate data is
    invalid or incomplete, Z_VERSION_ERROR if the version of zlib.h and
    the version of the library linked do not match. */
    int inf(const std::vector<unsigned char>& source, std::vector<unsigned char>& dest);

    /* wrap zlib error into exception */
    void checkZlibError(int ret);
    std::size_t readFromVector(unsigned char* buffer, size_t bufferSize, const std::vector<unsigned char>& source, size_t start);
    void insertInflatedBytes(const unsigned char* buffer, size_t have, std::vector<unsigned char>& dest);

private:
    static constexpr size_t CHUNK_SIZE = 16384;

private:
    z_stream m_strm;
};


} // namespace png_decoder::inflate
