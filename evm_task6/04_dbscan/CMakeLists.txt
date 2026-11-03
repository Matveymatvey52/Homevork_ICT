# Группа 212, Корепин Матвей
# Файл сборки CMake: DBSCAN.

cmake_minimum_required(VERSION 3.12)

project(dbscan)

add_executable(dbscan
    main.cpp
    functions.h
    functions.cpp
)

set_target_properties(dbscan PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)
