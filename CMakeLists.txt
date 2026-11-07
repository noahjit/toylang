cmake_minimum_required(VERSION 3.20)
project(toylang LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(TOYLANG_SANITIZE "Build with ASan + UBSan (GCC/Clang)" OFF)

add_library(toylang_lib
    src/lexer.cpp
)
target_include_directories(toylang_lib PUBLIC src)

if(MSVC)
    target_compile_options(toylang_lib PUBLIC /W4)
else()
    target_compile_options(toylang_lib PUBLIC -Wall -Wextra -Wpedantic)
endif()

if(TOYLANG_SANITIZE AND NOT MSVC)
    target_compile_options(toylang_lib PUBLIC
        -fsanitize=address,undefined -fno-omit-frame-pointer)
    target_link_options(toylang_lib PUBLIC -fsanitize=address,undefined)
endif()

add_executable(toylang src/main.cpp)
target_link_libraries(toylang PRIVATE toylang_lib)

enable_testing()
add_subdirectory(tests)