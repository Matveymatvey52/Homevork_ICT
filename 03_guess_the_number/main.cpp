// Группа 212, Корепин Матвей
// Домашнее задание 3 (OTUS, C++): "Угадай число" с таблицей рекордов
//
// Главный файл программы. Объединяет заготовки из материалов занятия
// (check_value.cpp — цикл сравнения с подсказками "less than"/"greater
// than", random_value.cpp — генератор случайных чисел, high_scores.cpp —
// таблица рекордов, argument.cpp — разбор аргументов командной строки)
// в одну связную игру, как и требует условие домашнего задания.

#include "cli_options.h"
#include "high_scores.h"

#include <cstdlib>  // для std::rand и std::srand
#include <ctime>    // для std::time (источник seed для генератора)
#include <iostream>
#include <string>

int main(int argc, char** argv) {
    // Разбираем аргументы командной строки (-max, -level, -table).
    CliOptions options = parse_cli_options(argc, argv);

    if (options.has_error) {
        // Сообщение об ошибке уже выведено внутри parse_cli_options.
        return -1;
    }

    if (options.show_table_only) {
        // Дополнительное задание 2: показать таблицу рекордов и сразу выйти.
        print_high_scores();
        return 0;
    }

    // Инициализируем генератор случайных чисел текущим временем,
    // иначе при каждом запуске загадывалось бы одно и то же число.
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    // Загадываем число в диапазоне [0; max_value).
    const int target_value = std::rand() % options.max_value;

    // Спрашиваем имя игрока — оно нужно для таблицы рекордов.
    std::cout << "Hi! Enter your name, please:" << std::endl;
    std::string user_name;
    std::cin >> user_name;

    std::cout << "Enter your guess:" << std::endl;

    int attempts_count = 0; // счётчик попыток игрока
    int current_guess = 0;  // очередное число, введённое игроком

    // Основной игровой цикл: повторяем, пока игрок не угадает число.
    while (true) {
        std::cin >> current_guess;

        if (std::cin.fail()) {
            // Ввод закончился или пользователь ввёл не число — выходим,
            // чтобы не зациклиться, бесконечно повторяя одно и то же сравнение.
            std::cout << "Ввод завершён, игра прервана." << std::endl;
            return -1;
        }

        ++attempts_count; // засчитываем очередную попытку

        if (current_guess < target_value) {
            // Твоё число меньше загаданного — значит загаданное больше.
            std::cout << "загаданное число больше, чем " << current_guess << std::endl;
        }
        else if (current_guess > target_value) {
            // Твоё число больше загаданного — значит загаданное меньше.
            std::cout << "загаданное число меньше, чем " << current_guess << std::endl;
        }
        else {
            // Число угадано — выходим из цикла.
            std::cout << "you win! attempts = " << attempts_count << std::endl;
            break;
        }
    }

    // Сохраняем результат игрока в таблицу рекордов и печатаем её целиком.
    append_high_score(user_name, attempts_count);
    std::cout << std::endl;
    print_high_scores();

    return 0;
}
