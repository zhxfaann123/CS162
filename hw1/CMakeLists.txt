cmake_minimum_required(VERSION 3.15)
project(hw1 C)

set(CMAKE_C_STANDARD 11)

include_directories(.)

# Settings for lwords


set(CMAKE_C_FLAGS -DPINTOS_LIST)
add_executable(hw1
        debug.c
        debug.h
        list.c
        list.h
        word_count_l.c
        word_helpers.c
        word_helpers.h
        words.c)

# Settings for pwords
#[[
set(CMAKE_C_FLAGS "-DPINTOS_LIST -DPTHREADS")
add_executable(hw1_pthreads
        debug.c
        debug.h
        list.c
        list.h
        word_count_p.c
        word_count.h
        word_helpers.c
        word_helpers.h
        pwords.c)
        ]]


