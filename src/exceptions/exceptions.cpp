#include <stdexcept>

#include "exceptions.h"


namespace png_decoder::exceptions {

std::string getErrorMessage(std::string file, int line, std::string message) {
    return file + ":" + std::to_string(line) + ": '" + message + "'";
}

DecodingException::DecodingException(const std::string& message) : std::runtime_error(message) {}

InvalidSignatureException::InvalidSignatureException() : DecodingException("Invalid signature") {}

InvalidIHDRChunkException::InvalidIHDRChunkException() : DecodingException("Invalid IHDR chunk") {}

InvalidPLTEChunkException::InvalidPLTEChunkException() : DecodingException("Invalid PLTE chunk") {}

InvalidIENDChunkException::InvalidIENDChunkException() : DecodingException("Invalid IEND chunk") {}

InvalidCRCException::InvalidCRCException(const std::string& message) : DecodingException(message) {}

InvalidColorTypeChunkException::InvalidColorTypeChunkException(const std::string& message) : DecodingException(message) {}

CriticalChunkTypeChunkException::CriticalChunkTypeChunkException(const std::string& message) : DecodingException(message) {}

InvalidStreamException::InvalidStreamException(const std::string& message) : DecodingException(message) {}

// zlib exceptions
ZlibInvalidCompressionLevelException::ZlibInvalidCompressionLevelException() : DecodingException("zlib: invalid compression level") {}
ZlibInvalidDeflateDataException::ZlibInvalidDeflateDataException() : DecodingException("zlib: invalid or incomplete deflate data") {}
ZlibOutOfMemoryException::ZlibOutOfMemoryException() : DecodingException("zlib: out of memory") {}
ZlibVersionMismatchException::ZlibVersionMismatchException() : DecodingException("zlib: zlib version mismatch") {}

} // namespace png_decoder::exceptions
