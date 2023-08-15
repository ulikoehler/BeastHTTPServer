project(hello-world)
cmake_minimum_required(VERSION 3.2)
include_directories(../../include)
add_executable(hello-world ../../src/BeastHTTPServer.cpp hello-world.cpp)

