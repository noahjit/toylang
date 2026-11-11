include(FetchContent)
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.zip
)
# Needed on Windows so gtest uses the same runtime as your project
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(toylang_tests
    lexer_test.cpp
 "ast_test.cpp")
target_link_libraries(toylang_tests PRIVATE toylang_lib GTest::gtest_main)

include(GoogleTest)
gtest_discover_tests(toylang_tests)