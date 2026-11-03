# Группа 212, Корепин Матвей
# Файл сборки CMake: дерево, квадратно-гнездовая реализация.

cmake_minimum_required(VERSION 3.12)

project(mst_grid)

add_executable(mst_grid
    main.cpp
    functions.h
    functions.cpp
)

set_target_properties(mst_grid PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)
