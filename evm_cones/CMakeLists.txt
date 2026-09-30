# Группа 212, Корепин Матвей
# Файл сборки CMake для проекта с конусами.

cmake_minimum_required(VERSION 3.12)

project(cones_project)

add_executable(cones_project
    main.cpp
    functions.h
    functions.cpp
)

set_target_properties(cones_project PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
