// Группа 212, Корепин Матвей
// Домашнее задание 2 (OTUS, C++): "Пишем свой Hello, World"
//
// Главный файл программы. По условию ДЗ функция main изменяться не
// должна — единственное изменение здесь: тело функции todo_move_me_out
// убрано отсюда (оно теперь в move_me_out.cpp) и добавлен #include
// заголовка move_me_out.h, чтобы main по-прежнему мог её вызывать.

#include "example.h"
#include "move_me_out.h" // объявление todo_move_me_out теперь берём отсюда

#include <iostream> // нужен для std::cout и std::endl

// Точка входа в программу — код этой функции не менялся по условию задания.
int main() {
    std::cout << "Hello, World!" << std::endl; // приветственное сообщение

    example_function();   // вызов функции из example.cpp (не менялась)
    todo_move_me_out();    // вызов функции, вынесенной в move_me_out.cpp

    return 0; // 0 означает успешное завершение программы
}
