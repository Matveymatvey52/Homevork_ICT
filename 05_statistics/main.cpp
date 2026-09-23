// Группа 212, Корепин Матвей
// Задание 5 (OTUS, папка 09 "Полиморфизм и все-все-все").
// Расчёт статистических характеристик последовательности чисел.
// Числа вводятся одной строкой через пробел, ввод заканчивается по Enter.

#include <iostream>
#include <sstream>
#include <string>
#include "functions.h"

int main() {
    std::cout << "Здравствуйте! Программа считает статистику по числам." << std::endl;
    std::cout << "Введите числа через пробел и нажмите Enter." << std::endl;

    // Складываем все статистики в один массив указателей на базовый
    // класс. Благодаря полиморфизму работать с ними можно одинаково,
    // хотя внутри каждая считает своё.
    const size_t statistics_count = 6;
    IStatistics* statistics[statistics_count];

    statistics[0] = new Min{};
    statistics[1] = new Max{};
    statistics[2] = new Mean{};
    statistics[3] = new Std{};
    statistics[4] = new Percentile{90, "pct90"};
    statistics[5] = new Percentile{95, "pct95"};

    // Читаем всю строку до Enter, затем разбираем из неё числа.
    std::string line;
    std::getline(std::cin, line);
    std::istringstream input(line);

    double val = 0;
    size_t count = 0;
    while (input >> val) {
        for (size_t i = 0; i < statistics_count; ++i) {
            statistics[i]->update(val);
        }
        ++count;
    }

    // Если разбор остановился не в конце строки, значит в ней был мусор.
    bool bad_input = !input.eof();
    if (bad_input || count == 0) {
        if (bad_input) {
            std::cerr << "Ошибка: во вводе есть не только числа." << std::endl;
        } else {
            std::cerr << "Ошибка: не введено ни одного числа." << std::endl;
        }
        for (size_t i = 0; i < statistics_count; ++i) {
            delete statistics[i];
        }
        return 1;
    }

    // Печатаем результаты.
    for (size_t i = 0; i < statistics_count; ++i) {
        std::cout << statistics[i]->name() << " = "
                  << statistics[i]->eval() << std::endl;
    }

    // Освобождаем память, выделенную через new.
    for (size_t i = 0; i < statistics_count; ++i) {
        delete statistics[i];
    }

    return 0;
}
