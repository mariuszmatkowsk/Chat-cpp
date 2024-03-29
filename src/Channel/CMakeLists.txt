project(${CMAKE_PROJECT_NAME})

add_library(channel INTERFACE channel.hpp)

target_include_directories(
    channel
    INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/..
)
