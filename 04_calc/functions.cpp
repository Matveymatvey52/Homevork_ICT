// Группа 212, Корепин Матвей
// Реализация лексера, узлов дерева и парсера из functions.h.

#include "functions.h"
#include <cctype>

// --- Lexer ---

Lexer::Lexer(std::istream& in)
    : state_(State::Empty), number_(0), ch_(0), in_(in) {
    next_char();
}

char Lexer::next_char() {
    in_.get(ch_);
    return ch_;
}

// Конец ввода - конец файла или перевод строки.
bool Lexer::end() const {
    return in_.eof() || ch_ == '\n';
}

bool Lexer::isoperator(char ch) const {
    return ch == '+' || ch == '-' || ch == '*' || ch == '/';
}

int Lexer::get_number() const {
    return number_;
}

std::string Lexer::get_operator() const {
    return operator_;
}

std::string Lexer::get_name() const {
    return name_;
}

// Конечный автомат: по очередному символу решаем, остаться в текущем
// состоянии или перейти в другое и вернуть готовый токен.
Lexer::Token Lexer::next_token() {
    for (;;) {
        switch (state_) {
        case State::End:
            return Token::End;

        case State::ReadNumber:
            if (end()) {
                state_ = State::End;
                return Token::Number;
            }
            if (std::isdigit(static_cast<unsigned char>(ch_))) {
                number_ = 10 * number_ + (ch_ - '0'); // дописываем цифру справа
                next_char();
                break;
            }
            state_ = State::Empty;
            return Token::Number;

        case State::ReadName:
            if (end()) {
                state_ = State::End;
                return Token::Name;
            }
            if (std::isalnum(static_cast<unsigned char>(ch_))) {
                name_ += ch_;
                next_char();
                break;
            }
            state_ = State::Empty;
            return Token::Name;

        case State::Empty:
            if (end()) {
                state_ = State::End;
                return Token::End;
            }
            if (std::isspace(static_cast<unsigned char>(ch_))) {
                next_char();
                break;
            }
            if (isoperator(ch_)) {
                operator_ = ch_;
                next_char();
                return Token::Operator;
            }
            if (ch_ == '(') {
                next_char();
                return Token::Lbrace;
            }
            if (ch_ == ')') {
                next_char();
                return Token::Rbrace;
            }
            if (std::isdigit(static_cast<unsigned char>(ch_))) {
                number_ = ch_ - '0';
                state_ = State::ReadNumber;
                next_char();
                break;
            }
            if (std::isalpha(static_cast<unsigned char>(ch_))) {
                name_ = ch_;
                state_ = State::ReadName;
                next_char();
                break;
            }
            // Любой другой символ: сообщаем парсеру, что он незнаком.
            next_char();
            return Token::Unknown;
        }
    }
}

// --- ASTNode ---

ASTNode::ASTNode(const std::string& repr)
    : repr_(repr), lhs_(nullptr), rhs_(nullptr) {
}

ASTNode::ASTNode(const std::string& repr, ASTNode* lhs, ASTNode* rhs)
    : repr_(repr), lhs_(lhs), rhs_(rhs) {
}

// Удаляя узел, удаляем и всё поддерево под ним.
ASTNode::~ASTNode() {
    delete lhs_;
    delete rhs_;
}

std::string ASTNode::repr() const {
    return repr_;
}

void ASTNode::print(std::ostream& out) const {
    inner_print(out, 0);
}

// Дерево печатается "набок": левое поддерево выше, правое ниже,
// глубина узла - это отступ.
void ASTNode::inner_print(std::ostream& out, size_t indent) const {
    if (lhs_) {
        lhs_->inner_print(out, indent + 1);
    }
    for (size_t i = 0; i < indent; ++i) {
        out << "    ";
    }
    out << repr_ << '\n';
    if (rhs_) {
        rhs_->inner_print(out, indent + 1);
    }
}

// --- Наследники ASTNode ---

Number::Number(int val) : ASTNode(std::to_string(val)), val_(val) {
}

int Number::value() const {
    return val_;
}

Variable::Variable(const std::string& name) : ASTNode(name) {
}

Add::Add(ASTNode* lhs, ASTNode* rhs) : ASTNode("+", lhs, rhs) {
}

Sub::Sub(ASTNode* lhs, ASTNode* rhs) : ASTNode("-", lhs, rhs) {
}

Mul::Mul(ASTNode* lhs, ASTNode* rhs) : ASTNode("*", lhs, rhs) {
}

Div::Div(ASTNode* lhs, ASTNode* rhs) : ASTNode("/", lhs, rhs) {
}

// --- Parser ---

using Token = Lexer::Token;

Parser::Parser(Lexer& lexer) : lexer_(lexer), tok_(Token::End) {
}

std::string Parser::error() const {
    return error_;
}

void Parser::next_token() {
    tok_ = lexer_.next_token();
}

ASTNode* Parser::parse() {
    ASTNode* root = expr();
    if (root && tok_ != Token::End) {
        // Выражение разобрано, а в строке что-то осталось: "12 43", "a b".
        error_ = "лишние символы после выражения";
        delete root;
        return nullptr;
    }
    return root;
}

// Сложение и вычитание.
ASTNode* Parser::expr() {
    ASTNode* root = term();
    while (root && tok_ == Token::Operator) {
        char op = lexer_.get_operator().front();
        if (op != '+' && op != '-') {
            break;
        }
        ASTNode* rhs = term();
        if (!rhs) {
            delete root; // не оставляем в памяти уже построенную часть
            return nullptr;
        }
        if (op == '+') {
            root = new Add(root, rhs);
        } else {
            root = new Sub(root, rhs);
        }
    }
    return root;
}

// Умножение и деление.
ASTNode* Parser::term() {
    ASTNode* root = prim();
    while (root && tok_ == Token::Operator) {
        char op = lexer_.get_operator().front();
        if (op != '*' && op != '/') {
            break;
        }
        ASTNode* rhs = prim();
        if (!rhs) {
            delete root;
            return nullptr;
        }
        if (op == '*') {
            root = new Mul(root, rhs);
        } else {
            root = new Div(root, rhs);
        }
    }
    return root;
}

// Число, имя или выражение в скобках.
ASTNode* Parser::prim() {
    next_token();
    ASTNode* node = nullptr;
    switch (tok_) {
    case Token::Number:
        node = new Number(lexer_.get_number());
        break;
    case Token::Name:
        node = new Variable(lexer_.get_name());
        break;
    case Token::Lbrace:
        node = expr();
        if (!node) {
            return nullptr;
        }
        if (tok_ != Token::Rbrace) {
            error_ = "нет закрывающей скобки";
            delete node;
            return nullptr;
        }
        break;
    case Token::Unknown:
        error_ = "недопустимый символ";
        return nullptr;
    default:
        error_ = "ожидалось число, имя переменной или скобка";
        return nullptr;
    }
    next_token();
    return node;
}
