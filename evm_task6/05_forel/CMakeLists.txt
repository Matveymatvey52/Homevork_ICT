# Группа 212, Корепин Матвей
# Файл сборки CMake: ФОРЭЛ.

cmake_minimum_required(VERSION 3.12)

project(forel)

add_executable(forel
    main.cpp
    functions.h
    functions.cpp
)

set_target_properties(forel PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)
