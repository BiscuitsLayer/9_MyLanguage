cmake_minimum_required(VERSION 3.15)
project(9_MyLanguage)

set(CMAKE_CXX_STANDARD 14)

add_executable(9_MyLanguage main.cpp
        my_stack/StackLib.cpp my_asm/Assembler.cpp my_asm/CPU.cpp my_asm/Disassembler.cpp)