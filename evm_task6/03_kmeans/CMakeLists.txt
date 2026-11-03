# Группа 212, Корепин Матвей
# Файл сборки CMake: k-means.

cmake_minimum_required(VERSION 3.12)

project(kmeans)

add_executable(kmeans
    main.cpp
    functions.h
    functions.cpp
)

set_target_properties(kmeans PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)
