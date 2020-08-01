cmake_minimum_required(VERSION 3.15)
project(words)

set(CMAKE_C_STANDARD 99)
set(OBJS
        ${CMAKE_CURRENT_SOURCE_DIR}/wc_sort.o)

include_directories(.)

add_executable(words
    main.c
        ${OBJS}
    word_count.c
    word_count.h)

SET_SOURCE_FILES_PROPERTIES(
        ${OBJS}
        PROPERTIES
        EXTERNAL_OBJECT true
        GENERATED true
)
