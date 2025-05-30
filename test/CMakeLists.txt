enable_testing()

find_package(GTest CONFIG REQUIRED)

add_executable(LexerTest lexer.cpp)
target_link_libraries(LexerTest PRIVATE GTest::gtest GTest::gtest_main GTest::gmock GTest::gmock_main)

add_test(LexerTest LexerTest)