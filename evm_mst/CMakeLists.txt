# Группа 212, Корепин Матвей
# Файл сборки CMake для проекта: деревья и волна.

cmake_minimum_required(VERSION 3.12)

project(mst_project)

add_executable(mst_project
    main.cpp
    functions.h
    functions.cpp
)

set_target_properties(mst_project PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
