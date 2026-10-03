// Группа 212, Корепин Матвей
// Объявления классов и функций. Реализация - в functions.cpp.

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <list>
#include <random>
#include <string>
#include <vector>

// Точка на плоскости. Базовый класс иерархии Point -> Circle -> Cone.
class Point {
public:
    Point();
    Point(float x, float y);

    float getX() const;
    float getY() const;
    void setX(float x);
    void setY(float y);

private:
    float x_;
    float y_;
};

// Круг: наследует Point, добавляет радиус. Площадь - вычисляемый метод.
class Circle : public Point {
public:
    Circle();
    Circle(float x, float y, float radius);

    float getRadius() const;
    void setRadius(float radius);
    float area() const;

private:
    float radius_;
};

// Конус: наследует Circle, добавляет высоту. Объём - вычисляемый метод.
// В задании фигуру называли "пирамидой", но по описанию (круглое
// основание + высота) это конус.
class Cone : public Circle {
public:
    Cone();
    Cone(float x, float y, float radius, float height);

    float getHeight() const;
    void setHeight(float height);
    float volume() const;

private:
    float height_;
};

// Поле - площадка с границами по x и y.
class Field {
public:
    Field();
    Field(float minX, float maxX, float minY, float maxY);

    float getMinX() const;
    float getMaxX() const;
    float getMinY() const;
    float getMaxY() const;

private:
    float minX_;
    float maxX_;
    float minY_;
    float maxY_;
};

// Читает конусы из файла (первая строка - количество N, затем N строк
// "x y R h") в список. false - если файл не прочитался.
bool loadConesFromFile(const std::string& filename, std::list<Cone>& coneList);

// --- Сетка квадратов ---

const float FIELD_MIN = -10.0f;  // поле от -10 до 10 по x и по y
const float FIELD_MAX = 10.0f;
const int GRID_N = 100;          // сетка 100 x 100 квадратов
const float SQUARE = (FIELD_MAX - FIELD_MIN) / GRID_N; // сторона квадрата

// Номер квадрата по x (s1) и по y (s2), от 0 до GRID_N - 1.
int squareX(float x);
int squareY(float y);

// Правило сравнения конусов по квадратам: сначала строка s2, потом столбец s1.
bool coneLess(const Cone& a, const Cone& b);

// --- Вторичные точки (2D-гаусс под конусами) ---

// Вторичная точка: координаты, квадрат сетки и номер конуса.
struct GPoint {
    float x;
    float y;
    int s1;
    int s2;
    int cone;
};

// Сигма гаусса для конуса: sigma = R * hMin / (2 * h).
// Чем выше конус, тем меньше сигма, а круг основания всегда охватывает 2*sigma.
float coneSigma(const Cone& cone, float hMin);

// Генерирует perCone точек 2D-гаусса под каждым конусом.
std::vector<GPoint> generateGaussPoints(const std::list<Cone>& coneList,
                                        int perCone, std::mt19937& gen);

// Правило сравнения точек по квадратам (для std::sort).
bool pointLess(const GPoint& a, const GPoint& b);

// Пишет точки в файл "x y метка" для gnuplot.
void writePointsFile(const std::vector<GPoint>& points,
                     const std::vector<int>& labels,
                     const std::string& filename);

// Скрипт gnuplot: точки на плоскости, цвет по метке.
void writePointsScript(const std::string& scriptFilename,
                       const std::string& dataFilename,
                       const std::string& imageFilename,
                       const std::string& title,
                       const Field& field);

// --- Кластеры ---

// Кластер: номера вторичных точек и их характеристики.
struct Cluster {
    std::vector<int> points;
    float minX, maxX, minY, maxY;
    float cx, cy; // центр масс
};

// Считает min/max и центр масс кластера по его точкам.
void computeClusterStats(Cluster& cl, const std::vector<GPoint>& points);

// Правило сравнения кластеров: больший идёт раньше.
bool biggerCluster(const Cluster& a, const Cluster& b);

// Номер кластера для каждой точки (для раскраски в gnuplot).
std::vector<int> clusterLabels(const std::vector<Cluster>& clusters, int n);

// --- Алгоритм "Волна" ---

// Двоичная матрица n x n одной строкой: b[i*n + j] = 1, если точки i и j
// ближе порога. Соседей ищем только в 9 квадратах вокруг точки.
std::vector<char> buildIncidenceMatrix(const std::vector<GPoint>& points,
                                       float threshold);

// Волна: находит все связные компоненты графа по матрице.
std::vector<Cluster> waveClusters(const std::vector<GPoint>& points,
                                  const std::vector<char>& matrix);

// --- Минимальное покрывающее дерево ---

struct Edge {
    int a;
    int b;
    float len;
};

// Строит дерево (алгоритм Прима): начинаем с точки 0 и каждый раз
// присоединяем ближайшую к дереву точку.
std::vector<Edge> buildSpanningTree(const std::vector<GPoint>& points);

// Гистограмма длин рёбер: bins столбцов от 0 до maxLen.
std::vector<int> edgeHistogram(const std::vector<Edge>& edges,
                               int bins, float maxLen);

// Пишет гистограмму и скрипт gnuplot для неё.
void writeHistogram(const std::vector<int>& hist, float maxLen,
                    float threshold, const std::string& dataFilename,
                    const std::string& scriptFilename,
                    const std::string& imageFilename);

// Удаляет рёбра длиннее порога и собирает оставшиеся куски в кластеры.
std::vector<Cluster> treeClusters(const std::vector<GPoint>& points,
                                  const std::vector<Edge>& edges,
                                  float threshold);

#endif // FUNCTIONS_H
