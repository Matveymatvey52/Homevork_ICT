# Группа 212, Корепин Матвей
# Сборка калькулятора (задание папки 08 OTUS).

cmake_minimum_required(VERSION 3.12)

project(calc CXX)

add_executable(calc
    main.cpp
    functions.h
    functions.cpp
)

set_target_properties(calc PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)

# По заданию проект должен собираться без предупреждений с этими флагами.
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(calc PRIVATE -Wall -Wextra -Werror -pedantic)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(calc PRIVATE /EHsc /W4 /WX)
endif()
