cmake_minimum_required(VERSION 3.17)
project(codesim)

set(CMAKE_CXX_STANDARD 14)

add_executable(codesim AsmSim.cpp KeySim.cpp KeySim.h main.cpp AsmSim.h)