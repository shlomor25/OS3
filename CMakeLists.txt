cmake_minimum_required(VERSION 3.12)
project(OS3 C)

set(CMAKE_C_STANDARD 99)

add_executable(OS3 ex31.c ex32.c cmake-build-debug/input.txt cmake-build-debug/configuration.txt cmake-build-debug/correctOutput.txt cmake-build-debug/comp.out)