// Группа 212, Корепин Матвей
// Объявления интерфейса IStatistics и классов статистик.
// Реализация - в functions.cpp.

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <vector>

// Общий интерфейс для всех видов статистики. Чисто виртуальные функции
// (= 0) обязывают каждого наследника реализовать их по-своему - это и
// есть полиморфизм.
class IStatistics {
public:
    virtual ~IStatistics() {}

    virtual void update(double next) = 0;   // обработать очередное число
    virtual double eval() const = 0;        // выдать результат
    virtual const char* name() const = 0;   // название статистики
};

// Минимальное значение.
class Min : public IStatistics {
public:
    Min();

    void update(double next) override;
    double eval() const override;
    const char* name() const override;

private:
    double m_min;
};

// Максимальное значение.
class Max : public IStatistics {
public:
    Max();

    void update(double next) override;
    double eval() const override;
    const char* name() const override;

private:
    double m_max;
};

// Среднее арифметическое.
class Mean : public IStatistics {
public:
    Mean();

    void update(double next) override;
    double eval() const override;
    const char* name() const override;

private:
    double m_sum;
    size_t m_count;
};

// Среднеквадратическое отклонение.
class Std : public IStatistics {
public:
    Std();

    void update(double next) override;
    double eval() const override;
    const char* name() const override;

private:
    double m_sum;
    double m_sum_sq;
    size_t m_count;
};

// Процентиль: значение, ниже которого лежит заданный процент чисел.
class Percentile : public IStatistics {
public:
    Percentile(int percent, const char* title);

    void update(double next) override;
    double eval() const override;
    const char* name() const override;

private:
    std::vector<double> m_values;
    int m_percent;
    const char* m_name;
};

#endif // FUNCTIONS_H
