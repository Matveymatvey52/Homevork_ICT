// Группа 212, Корепин Матвей
// Задание папки 08 OTUS "Инженерный калькулятор".
// Объявления лексера, узлов дерева (АСД) и парсера.
// Реализация - в functions.cpp.

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <istream>
#include <ostream>
#include <string>

// --- Лексер: разбивает строку на токены ---

class Lexer {
public:
    enum class Token {
        Number,
        Operator,
        End,
        Lbrace,
        Rbrace,
        Name,
        Unknown, // символ, который калькулятор не понимает
    };

    explicit Lexer(std::istream& in);

    // Копировать лексер нельзя: он держит ссылку на поток ввода.
    Lexer(const Lexer& other) = delete;
    Lexer& operator=(const Lexer& other) = delete;

    Token next_token();

    int get_number() const;
    std::string get_operator() const;
    std::string get_name() const;

private:
    enum class State {
        Empty,
        ReadNumber,
        ReadName,
        End,
    };

    char next_char();
    bool end() const;
    bool isoperator(char ch) const;

    State state_;
    std::string name_;
    int number_;
    std::string operator_;
    char ch_;
    std::istream& in_;
};

// --- Узлы абстрактного синтаксического дерева ---

// Базовый узел. Владеет дочерними узлами и удаляет их в деструкторе.
// Правило трёх: раз есть свой деструктор, копирование запрещаем, иначе
// два узла удалили бы одних и тех же детей дважды.
class ASTNode {
public:
    explicit ASTNode(const std::string& repr);
    ASTNode(const std::string& repr, ASTNode* lhs, ASTNode* rhs);

    ASTNode(const ASTNode& other) = delete;
    ASTNode& operator=(const ASTNode& other) = delete;

    virtual ~ASTNode();

    std::string repr() const;
    void print(std::ostream& out) const;

private:
    void inner_print(std::ostream& out, size_t indent) const;

    std::string repr_;
    ASTNode* lhs_;
    ASTNode* rhs_;
};

// Целочисленная константа (лист дерева).
class Number : public ASTNode {
public:
    explicit Number(int val);
    int value() const;

private:
    int val_;
};

// Имя переменной (лист дерева).
class Variable : public ASTNode {
public:
    explicit Variable(const std::string& name);
};

// Арифметические операции: у каждой два дочерних узла.
class Add : public ASTNode {
public:
    Add(ASTNode* lhs, ASTNode* rhs);
};

class Sub : public ASTNode {
public:
    Sub(ASTNode* lhs, ASTNode* rhs);
};

class Mul : public ASTNode {
public:
    Mul(ASTNode* lhs, ASTNode* rhs);
};

class Div : public ASTNode {
public:
    Div(ASTNode* lhs, ASTNode* rhs);
};

// --- Парсер: строит дерево методом рекурсивного спуска ---
//
// Грамматика:
//   E -> T | T + E | T - E
//   T -> P | P * T | P / T
//   P -> Number | Name | ( E )
class Parser {
public:
    explicit Parser(Lexer& lexer);

    Parser(const Parser& other) = delete;
    Parser& operator=(const Parser& other) = delete;

    // Возвращает корень дерева или nullptr при ошибке в выражении.
    ASTNode* parse();

    std::string error() const;

private:
    void next_token();
    ASTNode* expr();
    ASTNode* term();
    ASTNode* prim();

    Lexer& lexer_;
    Lexer::Token tok_;
    std::string error_;
};

#endif // FUNCTIONS_H
