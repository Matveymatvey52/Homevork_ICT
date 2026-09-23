// Группа 212, Корепин Матвей
// Реализация классов статистик, объявленных в functions.h.

#include "functions.h"
#include <algorithm>
#include <cmath>
#include <limits>

// --- Min ---

// Начинаем с "плюс бесконечности", чтобы первое же число оказалось
// меньше и записалось в m_min.
Min::Min() : m_min{std::numeric_limits<double>::infinity()} {
}

void Min::update(double next) {
    if (next < m_min) {
        m_min = next;
    }
}

double Min::eval() const {
    return m_min;
}

const char* Min::name() const {
    return "min";
}

// --- Max ---

// Здесь наоборот - начинаем с "минус бесконечности".
Max::Max() : m_max{-std::numeric_limits<double>::infinity()} {
}

void Max::update(double next) {
    if (next > m_max) {
        m_max = next;
    }
}

double Max::eval() const {
    return m_max;
}

const char* Max::name() const {
    return "max";
}

// --- Mean ---

Mean::Mean() : m_sum{0.0}, m_count{0} {
}

void Mean::update(double next) {
    m_sum += next;
    ++m_count;
}

double Mean::eval() const {
    if (m_count == 0) {
        return 0.0;
    }
    return m_sum / m_count;
}

const char* Mean::name() const {
    return "mean";
}

// --- Std ---

Std::Std() : m_sum{0.0}, m_sum_sq{0.0}, m_count{0} {
}

// Копим сумму чисел и сумму их квадратов - этого достаточно, чтобы
// потом посчитать отклонение, не храня всю последовательность.
void Std::update(double next) {
    m_sum += next;
    m_sum_sq += next * next;
    ++m_count;
}

double Std::eval() const {
    if (m_count == 0) {
        return 0.0;
    }
    double mean = m_sum / m_count;
    // Дисперсия = среднее квадратов минус квадрат среднего.
    double variance = m_sum_sq / m_count - mean * mean;
    if (variance < 0.0) {
        variance = 0.0; // защита от погрешности вычислений
    }
    return std::sqrt(variance);
}

const char* Std::name() const {
    return "std";
}

// --- Percentile ---

Percentile::Percentile(int percent, const char* title)
    : m_percent{percent}, m_name{title} {
}

// В отличие от остальных классов приходится хранить все числа: для
// процентиля нужна отсортированная последовательность.
void Percentile::update(double next) {
    m_values.push_back(next);
}

double Percentile::eval() const {
    if (m_values.empty()) {
        return 0.0;
    }

    // Сортируем копию, чтобы не менять сам массив.
    std::vector<double> sorted = m_values;
    std::sort(sorted.begin(), sorted.end());

    // Берём элемент, стоящий на нужной доле от начала.
    size_t index = static_cast<size_t>(
        std::ceil(m_percent / 100.0 * sorted.size())) - 1;
    if (index >= sorted.size()) {
        index = sorted.size() - 1;
    }
    return sorted[index];
}

const char* Percentile::name() const {
    return m_name;
}
