project($CMAKE_PROJECT_NAME)

add_subdirectory(Channel)

find_package(Curses REQUIRED)

include_directories(${CURSES_INCLUDE_DIRS})

include(FetchContent)

FetchContent_Declare(asio
  # GIT_REPOSITORY https://github.com/chriskohlhoff/asio.git
  GIT_REPOSITORY git@github.com:chriskohlhoff/asio.git
  GIT_TAG master
  CONFIGURE_COMMAND ""
  BUILD_COMMAND ""
)

FetchContent_GetProperties(asio)
if(NOT asio_POPULATED)
  FetchContent_Populate(asio)
endif()

add_library(asio INTERFACE)

target_include_directories(asio INTERFACE ${asio_SOURCE_DIR}/asio/include)

add_executable(server server.cpp)
add_executable(client client.cpp)

target_link_libraries(
    server
    ${CURSES_LIBRARIES}
    asio
)

target_link_libraries(
    client 
    ${CURSES_LIBRARIES}
    asio
)

