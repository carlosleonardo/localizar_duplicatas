cmake_minimum_required(VERSION 4.1)
project(localizar_duplicatas)

set(CMAKE_CXX_STANDARD 20)

add_executable(localizar_duplicatas main.cpp)

find_package(cryptopp REQUIRED)
target_link_libraries(localizar_duplicatas PRIVATE cryptopp::cryptopp)