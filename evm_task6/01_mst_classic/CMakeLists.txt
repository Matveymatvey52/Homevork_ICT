# Группа 212, Корепин Матвей
# Файл сборки CMake: дерево, классическая реализация.

cmake_minimum_required(VERSION 3.12)

project(mst_classic)

add_executable(mst_classic
    main.cpp
    functions.h
    functions.cpp
)

set_target_properties(mst_classic PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)
