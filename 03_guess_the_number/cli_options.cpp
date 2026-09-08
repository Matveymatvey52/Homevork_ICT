// Группа 212, Корепин Матвей
// Домашнее задание 3 (OTUS, C++): "Угадай число" с таблицей рекордов
//
// Реализация разбора аргументов командной строки.
// За основу взята идея из примера argument.cpp (материалы занятия):
// argv[0] — это всегда имя запускаемого файла, поэтому реальные
// аргументы пользователя начинаются с argv[1].

#include "cli_options.h"

#include <iostream> // для std::cout при выводе сообщений об ошибках
#include <string>   // для std::string и std::stoi

CliOptions parse_cli_options(int argc, char** argv) {
    CliOptions options; // создаём структуру со значениями по умолчанию

    bool max_was_set = false;   // был ли явно передан -max
    bool level_was_set = false; // был ли явно передан -level

    // Проходим по всем аргументам, начиная с индекса 1 (argv[0] — имя программы)
    for (int i = 1; i < argc; ++i) {
        std::string current_arg{argv[i]}; // текущий аргумент как std::string

        if (current_arg == "-table") {
            // Флаг без значения — просто отмечаем режим показа таблицы
            options.show_table_only = true;
        }
        else if (current_arg == "-max") {
            // После -max обязательно должно идти число
            if (i + 1 >= argc) {
                std::cout << "Ошибка: после -max должно быть указано число!" << std::endl;
                options.has_error = true;
                return options;
            }
            ++i; // переходим к значению аргумента
            options.max_value = std::stoi(argv[i]); // преобразуем строку в число
            max_was_set = true;
        }
        else if (current_arg == "-level") {
            // После -level обязательно должно идти число уровня (1, 2 или 3)
            if (i + 1 >= argc) {
                std::cout << "Ошибка: после -level должно быть указано число!" << std::endl;
                options.has_error = true;
                return options;
            }
            ++i; // переходим к значению аргумента
            int level = std::stoi(argv[i]); // считываем номер уровня сложности

            // Переводим уровень сложности в максимальное загадываемое значение
            if (level == 1) {
                options.max_value = 10;
            }
            else if (level == 2) {
                options.max_value = 50;
            }
            else if (level == 3) {
                options.max_value = 100;
            }
            else {
                std::cout << "Ошибка: -level должен быть равен 1, 2 или 3!" << std::endl;
                options.has_error = true;
                return options;
            }
            level_was_set = true;
        }
    }

    // По условию дополнительного задания 5: -max и -level одновременно — ошибка
    if (max_was_set && level_was_set) {
        std::cout << "Ошибка: нельзя одновременно указывать -max и -level!" << std::endl;
        options.has_error = true;
    }

    return options;
}
