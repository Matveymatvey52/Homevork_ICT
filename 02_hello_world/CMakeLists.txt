# Группа 212, Корепин Матвей
# Домашнее задание 2 (OTUS, C++): "Пишем свой Hello, World"
#
# Файл сборки CMake. По сравнению с исходным CMakeLists.txt из
# материалов занятия добавлены новые файлы move_me_out.h и move_me_out.cpp.

cmake_minimum_required(VERSION 3.12) # минимальная поддерживаемая версия CMake

project(02_hello_world) # имя проекта

# Список исходников исполняемого файла: старые файлы + новая пара
# move_me_out.h / move_me_out.cpp, как того требует задание.
add_executable(hello_world
    hello_world.cpp
    example.cpp
    example.h
    move_me_out.cpp
    move_me_out.h
)

# Используем стандарт C++17 и требуем, чтобы он был обязательно доступен.
set_target_properties(hello_world PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
