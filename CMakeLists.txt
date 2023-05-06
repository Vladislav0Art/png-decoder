find_package(PNG)

if (NOT DEFINED PNG_FOUND)
    message(FATAL_ERROR "Can't find libpng")
endif()

include(build.cmake)

add_catch(test_png_decoder test.cpp)
target_compile_definitions(test_png_decoder PUBLIC TASK_DIR="${CMAKE_CURRENT_SOURCE_DIR}/")
target_include_directories(test_png_decoder PRIVATE ${PNG_INCLUDE_DIRS})
target_link_libraries(test_png_decoder ${PNG_STATIC} ${PNG_LIBRARY})
