// Группа 212, Корепин Матвей
// Интерфейс: читает config.txt и конусы, создаёт Control и передаёт ему
// команды пользователя. К остальным классам напрямую не обращается.

#include <iostream>
#include <string>
#include <vector>
#include "functions.h"

int main() {
    Config config;
    if (!config.load("config.txt")) {
        std::cout << "Ошибка: нет файла config.txt рядом с программой." << std::endl;
        return 1;
    }
    std::vector<Cone> cones;
    std::string conesFile = config.getString("cones_file", "cones.txt");
    if (!loadCones(conesFile, cones)) {
        std::cout << "Ошибка: не удалось прочитать " << conesFile << std::endl;
        return 1;
    }

    std::cout << "DBSCAN: фильтр графа и волна." << std::endl;
    std::cout << "Параметры из config.txt:" << std::endl;
    config.print();

    Control control(config, cones);
    control.execute("help");

    std::string line;
    while (true) {
        std::cout << std::endl << "Введите команду и нажмите Enter: ";
        if (!std::getline(std::cin, line)) {
            break;
        }
        if (!control.execute(line)) {
            break;
        }
    }
    std::cout << "Работа завершена." << std::endl;
    return 0;
}
