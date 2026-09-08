# Группа 212, Корепин Матвей
# Домашнее задание 3 (OTUS, C++): "Угадай число" с таблицей рекордов
#
# Файл сборки CMake. Собирает все .cpp файлы проекта в один
# исполняемый файл guess_the_number.

cmake_minimum_required(VERSION 3.12) # минимальная поддерживаемая версия CMake

project(03_guess_the_number) # имя проекта

# Перечисляем все исходники, из которых собирается исполняемый файл.
add_executable(guess_the_number
    main.cpp
    cli_options.cpp
    cli_options.h
    high_scores.cpp
    high_scores.h
)

# Используем стандарт C++17 и требуем, чтобы он был обязательно доступен.
set_target_properties(guess_the_number PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
)
