// Группа 212, Корепин Матвей
// Задание папки 08 OTUS "Инженерный калькулятор".
// Читает одно арифметическое выражение, строит по нему абстрактное
// синтаксическое дерево и печатает его набок.

#include <iostream>
#include "functions.h"

int main() {
    std::cout << "Введите арифметическое выражение и нажмите Enter:" << std::endl;

    Lexer lexer(std::cin);
    Parser parser(lexer);

    ASTNode* ast = parser.parse();
    if (!ast) {
        std::cerr << "Ошибка: " << parser.error() << std::endl;
        return 1;
    }

    ast->print(std::cout);

    // Дерево создавалось через new, удаление корня удаляет его целиком.
    delete ast;
    return 0;
}
