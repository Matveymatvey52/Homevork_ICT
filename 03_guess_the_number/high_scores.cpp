// Группа 212, Корепин Матвей
// Домашнее задание 3 (OTUS, C++): "Угадай число" с таблицей рекордов
//
// Реализация работы с файлом таблицы рекордов.
//
// Дополнительное задание 3 просит хранить для каждого игрока только его
// минимальное (лучшее) число попыток. Дополнительное задание 4 просит
// перезаписывать предыдущий результат игрока, если новый результат лучше.
// Оба задания на практике дают один и тот же итоговый результат — в файле
// хранится по одной строке на каждого игрока с его лучшим достижением,
// поэтому здесь они реализованы вместе одним и тем же кодом: при каждой
// игре вся таблица перечитывается в память, результат обновляется (если
// игрок новый — добавляется, если старый и сыграл лучше — уменьшается),
// после чего таблица целиком перезаписывается на диск.

#include "high_scores.h"

#include <fstream>
#include <iostream>
#include <map>     // map хранит записи в отсортированном порядке по ключу-имени
#include <string>

// Читает файл таблицы рекордов в память в виде "имя игрока -> лучший результат".
// Если файл ещё не существует (первый запуск программы), возвращает пустую map.
static std::map<std::string, int> read_high_scores_map() {
    std::map<std::string, int> scores; // результат: имя -> минимальное число попыток

    std::ifstream in_file{kHighScoresFilename}; // пробуем открыть файл на чтение
    if (!in_file.is_open()) {
        // Файла ещё нет — это нормально для самого первого запуска игры.
        return scores;
    }

    std::string username;   // имя очередного игрока из файла
    int attempts_count = 0; // его результат (число попыток) из файла

    while (in_file >> username >> attempts_count) {
        // Если имя уже встречалось — оставляем меньшее (лучшее) значение.
        auto existing = scores.find(username);
        if (existing == scores.end() || attempts_count < existing->second) {
            scores[username] = attempts_count;
        }
    }

    return scores;
}

// Полностью перезаписывает файл таблицы рекордов содержимым переданной map.
static void write_high_scores_map(const std::map<std::string, int>& scores) {
    // Открываем файл в обычном режиме (не app!), так как хотим ПЕРЕЗАПИСАТЬ
    // содержимое, а не дописать что-то в конец.
    std::ofstream out_file{kHighScoresFilename};
    if (!out_file.is_open()) {
        std::cout << "Не удалось открыть файл для записи: " << kHighScoresFilename << std::endl;
        return;
    }

    for (const auto& entry : scores) {
        // entry.first — имя игрока, entry.second — его лучший результат
        out_file << entry.first << ' ' << entry.second << std::endl;
    }
}

void append_high_score(const std::string& user_name, int attempts_count) {
    // Загружаем текущую таблицу рекордов в память.
    std::map<std::string, int> scores = read_high_scores_map();

    // Обновляем результат игрока, только если новый результат лучше
    // (либо игрок встречается в первый раз).
    auto existing = scores.find(user_name);
    if (existing == scores.end() || attempts_count < existing->second) {
        scores[user_name] = attempts_count;
    }

    // Сохраняем обновлённую таблицу обратно в файл.
    write_high_scores_map(scores);
}

void print_high_scores() {
    std::map<std::string, int> scores = read_high_scores_map();

    std::cout << "High scores table:" << std::endl;
    for (const auto& entry : scores) {
        std::cout << entry.first << ' ' << entry.second << std::endl;
    }
}
