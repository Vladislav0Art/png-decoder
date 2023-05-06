#pragma once

#include <stdexcept>

#define PNG_DECODER_ERROR_MESSAGE(str) (png_decoder::exceptions::getErrorMessage(__FILE__, __LINE__, str))

namespace png_decoder::exceptions {

std::string getErrorMessage(std::string file, int line, std::string message);

class DecodingException : public std::runtime_error {
public:
    DecodingException(const std::string& message);
};


class InvalidSignatureException : public DecodingException {
public:
    InvalidSignatureException();
};


class InvalidIHDRChunkException : public DecodingException {
public:
    InvalidIHDRChunkException();
};

class InvalidPLTEChunkException : public DecodingException {
public:
    InvalidPLTEChunkException();
};

class InvalidIENDChunkException : public DecodingException {
public:
    InvalidIENDChunkException();
};

class InvalidCRCException : public DecodingException {
public:
    InvalidCRCException(const std::string& message);
};

class InvalidColorTypeChunkException : public DecodingException {
public:
    InvalidColorTypeChunkException(const std::string& message);
};

class CriticalChunkTypeChunkException : public DecodingException {
public:
    CriticalChunkTypeChunkException(const std::string& message);
};

class InvalidStreamException : public DecodingException {
public:
    InvalidStreamException(const std::string& message);
};


// zlib wrapping exceptions
class ZlibInvalidCompressionLevelException : public DecodingException {
public:
    ZlibInvalidCompressionLevelException();
};


class ZlibInvalidDeflateDataException : public DecodingException {
public:
    ZlibInvalidDeflateDataException();
};


class ZlibOutOfMemoryException : public DecodingException {
public:
    ZlibOutOfMemoryException();
};


class ZlibVersionMismatchException : public DecodingException {
public:
    ZlibVersionMismatchException();
};

} // namespace png_decoder::exceptions
