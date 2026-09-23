# Группа 212, Корепин Матвей
# Файл сборки CMake для приложения расчёта статистики.

cmake_minimum_required(VERSION 3.12)

project(statistics)

add_executable(statistics
    main.cpp
    functions.h
    functions.cpp
)

set_target_properties(statistics PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
