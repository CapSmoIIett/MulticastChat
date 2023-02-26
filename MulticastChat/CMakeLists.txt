# CMakeList.txt : CMake project for MulticastChat, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

# Add source to this project's executable.
add_executable (MulticastChat "main.cpp" "header.h" "getlocalip.cpp")

find_library(WSOCK32_LIBRARY wsock32)
find_library(WS2_32_LIBRARY ws2_32)

target_link_libraries(MulticastChat wsock32 ws2_32)

# TODO: Add tests and install targets if needed.
