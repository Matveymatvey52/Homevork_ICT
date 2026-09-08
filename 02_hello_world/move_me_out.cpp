// Группа 212, Корепин Матвей
// Домашнее задание 2 (OTUS, C++): "Пишем свой Hello, World"
//
// Файл-реализация move_me_out.cpp.
// Раньше тело этой функции лежало прямо внутри hello_world.cpp вместе
// с функцией main. По заданию его нужно было "вынести" сюда, в отдельный
// файл, а в hello_world.cpp оставить только вызов через #include.

#include "move_me_out.h"

#include <iostream> // нужен для std::cout и std::endl

// Реализация функции: просто печатает строку в стандартный поток вывода.
void todo_move_me_out() {
    std::cout << "This function should be located in the separated source file" << std::endl;
}
