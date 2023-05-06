set(PNG_DECODER_SOURCES
    png_decoder.h
    png_decoder.cpp
    exceptions/exceptions.h
    exceptions/exceptions.cpp
    utils/utils.h
    utils/utils.cpp
    inflate/inflate.h
    inflate/inflate.cpp
    misc/crc.h
    misc/structs.h
    scanline-reader/scanline_reader.h
    scanline-reader/scanline_reader.cpp
    scanline-reader/strategy/strategy.h
    scanline-reader/strategy/strategy.cpp
    defilter/defilter.h
    defilter/defilter.cpp
    )

add_library(png_decoder_lib STATIC ${PNG_DECODER_SOURCES})

find_package(ZLIB)
target_link_libraries(png_decoder_lib ZLIB::ZLIB)

find_package(Boost COMPONENTS system REQUIRED)
target_link_libraries(png_decoder_lib Boost::system)

# it will allow you to automatically add the correct include directories with "target_link_libraries"
target_include_directories(png_decoder_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(png_decoder_lib PUBLIC ../)