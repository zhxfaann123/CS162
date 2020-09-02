cmake_minimum_required(VERSION 3.15)
project(hw2)

set(CMAKE_C_STANDARD 11)

include_directories(.)

add_executable(hw2
        shell.c
        tokenizer.c
        tokenizer.h
        Utils.c
        Utils.h
        my_signal.c
        my_signal.h)
