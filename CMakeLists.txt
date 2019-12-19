cmake_minimum_required(VERSION 3.15)
project(9_MyLanguage)

set(CMAKE_CXX_STANDARD 14)

add_executable(9_MyLanguage main.cpp
        my_stack/Stack.cpp my_asm/Assembler.cpp my_asm/CPU.cpp
        my_lang/General.cpp my_frontend/Frontend.cpp my_frontend_1/Frontend_1.cpp my_middleend/Middleend.cpp my_backend/Backend.cpp)