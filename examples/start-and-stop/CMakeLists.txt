project(start-and-stop)
cmake_minimum_required(VERSION 3.2)
include_directories(../../include)
add_executable(start-and-stop ../../src/BeastHTTPServer.cpp start-and-stop.cpp)

