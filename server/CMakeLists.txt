cmake_minimum_required(VERSION 3.26)
project(server)

set(CMAKE_CXX_STANDARD 23)
set(HEADERS server.h)
set(SOURCES server.cpp main.cpp)

include(FetchContent)
FetchContent_Declare(
        Poco
        URL https://github.com/pocoproject/poco/archive/refs/tags/poco-1.12.4-release.zip
)
FetchContent_MakeAvailable(Poco)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCES})

target_link_libraries(${PROJECT_NAME} PUBLIC Foundation Net Util)