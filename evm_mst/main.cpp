// Группа 212, Корепин Матвей
// Клиентская часть: общается с пользователем и передаёт команды
// в Control. К серверу напрямую не обращается.

#include <iostream>
#include <string>
#include "functions.h"

int main() {
    Control control;

    std::cout << "Кластерный анализ точек под конусами." << std::endl;
    control.execute("help");

    std::string line;
    while (true) {
        std::cout << std::endl << "Введите команду и нажмите Enter: ";
        if (!std::getline(std::cin, line)) {
            break; // ввод закончился
        }
        if (!control.execute(line)) {
            break; // пришла команда exit
        }
    }

    std::cout << "Работа завершена." << std::endl;
    return 0;
}
