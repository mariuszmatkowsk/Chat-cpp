project(${CMAKE_PROJECT_NAME})

include(FetchContent)

FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(
    channelTests
    channelTests.cpp
)

target_link_libraries(
    channelTests
    GTest::gtest_main
    channel
)

include(GoogleTest)

gtest_discover_tests(channelTests)

