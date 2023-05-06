#include <cstdint>
#include <vector>
#include <cassert>
#include <cstring>
#include <zlib.h>

#include "inflate.h"



namespace png_decoder::inflate {

Inflate::Inflate() : m_strm{} {}

Inflate::~Inflate() {
    static_cast<void>(inflateEnd(&m_strm));
}

std::vector<unsigned char> Inflate::doInflate(const std::vector<unsigned char>& source) {
    std::vector<unsigned char> result;
    int ret = inf(source, result);
    checkZlibError(ret);

    return result;
}


int Inflate::inf(const std::vector<unsigned char>& source, std::vector<unsigned char>& dest) {
    int ret;
    uint32_t have;
    size_t sourceCurrentIndex = 0;
    unsigned char in[CHUNK_SIZE];
    unsigned char out[CHUNK_SIZE];

    /* allocate inflate state */
    m_strm.zalloc = Z_NULL;
    m_strm.zfree = Z_NULL;
    m_strm.opaque = Z_NULL;
    m_strm.avail_in = 0;
    m_strm.next_in = Z_NULL;
    ret = inflateInit(&m_strm);

    if (ret != Z_OK) {
        return ret;
    }

    /* decompress until deflate stream ends or end of file */
    do {
        size_t count = readFromVector(in, CHUNK_SIZE, source, sourceCurrentIndex);
        sourceCurrentIndex += count;
        m_strm.avail_in = count;

        if (m_strm.avail_in == 0) {
            break;
        }
        m_strm.next_in = in;

        /* run inflate() on input until output buffer not full */
        do {
            m_strm.avail_out = CHUNK_SIZE;
            m_strm.next_out = out;

            ret = ::inflate(&m_strm, Z_NO_FLUSH);
            assert(ret != Z_STREAM_ERROR);  /* state not clobbered */

            switch (ret) {
                case Z_NEED_DICT:
                    ret = Z_DATA_ERROR;     /* and fall through */
                case Z_DATA_ERROR:
                case Z_MEM_ERROR:
                    return ret;
            }

            have = CHUNK_SIZE - m_strm.avail_out;
            insertInflatedBytes(out, have, dest);

        } while (m_strm.avail_out == 0);

        /* done when inflate() says it's done */
    } while (ret != Z_STREAM_END);

    return ret == Z_STREAM_END ? Z_OK : Z_DATA_ERROR;
}


void Inflate::checkZlibError(int ret) {
    switch (ret) {
    case Z_STREAM_ERROR:
        throw exceptions::ZlibInvalidCompressionLevelException();
    case Z_DATA_ERROR:
        throw exceptions::ZlibInvalidDeflateDataException();
    case Z_MEM_ERROR:
        throw exceptions::ZlibOutOfMemoryException();
    case Z_VERSION_ERROR:
        throw exceptions::ZlibVersionMismatchException();
    }
}


std::size_t Inflate::readFromVector(unsigned char* buffer, size_t bufferSize, const std::vector<unsigned char>& source, size_t start) {
    size_t left = (start <= source.size()) ? source.size() - start : 0;
    size_t available = std::min(bufferSize, left);

    std::memcpy(buffer, source.data() + start, available);
    return available;
}

void Inflate::insertInflatedBytes(const unsigned char* buffer, size_t have, std::vector<unsigned char>& dest) {
    dest.reserve(dest.size() + have);
    for (size_t i = 0; i < have; ++i) {
        dest.push_back(buffer[i]);
    }
}


} // namespace png_decoder::inflate
