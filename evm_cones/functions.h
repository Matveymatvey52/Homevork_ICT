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

// Круг: наследует Point, добавляет радиус. Площадь - вычисляемый
// метод, а не хранимое поле.
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

// Поле - площадка на плоскости с ограничениями по x и y, задаёт
// границы для размещения конусов и границы графика в gnuplot.
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
// "x y R h") в список STL. false - если файл не прочитался.
bool loadConesFromFile(const std::string& filename, std::list<Cone>& coneList);

// Записывает точки конусов в файл для gnuplot.
void writeConeDataFile(const std::list<Cone>& coneList, const std::string& dataFilename);

// Создаёт скрипт gnuplot для отрисовки конусов в PNG-картинку.
void writeGnuplotScript(const std::string& scriptFilename,
                         const std::string& dataFilename,
                         const std::string& imageFilename,
                         const Field& field);

// --- Сетка квадратов ---

// Поле делится на n x n одинаковых квадратов. Квадрат задаётся парой
// номеров (s1, s2): s1 - номер столбца по x, s2 - номер строки по y.
class Grid {
public:
    Grid(const Field& field, int n);

    int getN() const;
    float getStep() const;     // сторона квадрата по x
    int squareX(float x) const; // s1 для координаты x
    int squareY(float y) const; // s2 для координаты y

private:
    float minX_;
    float minY_;
    float stepX_;
    float stepY_;
    int n_;
};

// Сортирует конусы по координатам квадрата их центра: сначала по s2,
// при равенстве по s1 (построчно, как читается сетка).
void sortConesBySquares(std::list<Cone>& coneList, const Grid& grid);

// --- Вторичные точки (2D-гаусс под конусами) ---

// Вторичная точка: координаты, квадрат сетки и номер конуса, под
// которым она сгенерирована.
struct GPoint {
    float x;
    float y;
    int s1;
    int s2;
    int cone;
};

// Сигма гаусса для конуса. Чем выше конус, тем меньше сигма:
// sigma = R * hMin / (2 * h). У самого низкого конуса 2*sigma = R,
// у остальных 2*sigma < R, поэтому круг основания всегда охватывает
// область 2*sigma.
float coneSigma(const Cone& cone, float hMin);

// Генерирует perCone точек 2D-гаусса под каждым конусом.
std::vector<GPoint> generateGaussPoints(const std::list<Cone>& coneList,
                                        int perCone, std::mt19937& gen,
                                        const Grid& grid);

// Упорядочивает точки по квадратам (s2, затем s1).
void sortPointsBySquares(std::vector<GPoint>& points);

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

// Кластер: номера вторичных точек и простые характеристики.
struct Cluster {
    std::vector<int> points;
    float minX, maxX, minY, maxY;
    float cx, cy; // центр масс
};

// Считает min/max и центр масс кластера по его точкам.
void computeClusterStats(Cluster& cl, const std::vector<GPoint>& points);

// Метка кластера для каждой точки (для раскраски в gnuplot).
std::vector<int> clusterLabels(const std::vector<Cluster>& clusters, size_t n);

// --- Алгоритм "Волна" ---

// Двоичная матрица инцидентности n x n, хранится одной строкой char:
// b[i*n + j] = 1, если между точками i и j есть ребро.
// Ребро ставится, если расстояние меньше threshold. Соседей ищем
// только в 9 квадратах вокруг точки, поэтому threshold не должен
// превышать сторону квадрата.
std::vector<char> buildIncidenceMatrix(const std::vector<GPoint>& points,
                                       const Grid& grid, float threshold);

// Волновой алгоритм: находит все связные компоненты графа по матрице.
std::vector<Cluster> waveClusters(const std::vector<GPoint>& points,
                                  const std::vector<char>& matrix);

// --- Минимальное покрывающее дерево ---

struct Edge {
    int a;
    int b;
    float len;
};

// Строит минимальное покрывающее дерево (алгоритм Прима): начинаем с
// самого короткого ребра и присоединяем ближайшую к дереву точку.
std::vector<Edge> buildSpanningTree(const std::vector<GPoint>& points);

// Гистограмма длин рёбер: bins столбцов от 0 до maxLen.
std::vector<int> edgeHistogram(const std::vector<Edge>& edges,
                               int bins, float maxLen);

// Пишет гистограмму и скрипт gnuplot для неё.
void writeHistogram(const std::vector<int>& hist, float maxLen,
                    float threshold, const std::string& dataFilename,
                    const std::string& scriptFilename,
                    const std::string& imageFilename);

// Удаляет рёбра длиннее порога и собирает оставшиеся куски дерева
// в кластеры.
std::vector<Cluster> treeClusters(const std::vector<GPoint>& points,
                                  const std::vector<Edge>& edges,
                                  float threshold);

#endif // FUNCTIONS_H
