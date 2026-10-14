// Группа 212, Корепин Матвей
// Серверная часть (класс Server) и класс Control, через который клиент
// из main.cpp передаёт команды серверу.

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <list>
#include <random>
#include <string>
#include <vector>

const float FIELD_MIN = -10.0f; // поле от -10 до 10 по x и по y
const float FIELD_MAX = 10.0f;
const int TREE_GRID_N = 80;     // сетка для дерева: квадрат 0.25 x 0.25
const int MIN_CLUSTER = 10;     // кластеры меньше - это выбросы

// Точка на плоскости. Базовый класс иерархии Point -> Circle -> Cone.
class Point {
public:
    Point();
    Point(float x, float y);

    float getX() const;
    float getY() const;

private:
    float x_;
    float y_;
};

// Круг: наследует Point, добавляет радиус.
class Circle : public Point {
public:
    Circle();
    Circle(float x, float y, float radius);

    float getRadius() const;

private:
    float radius_;
};

// Конус: наследует Circle, добавляет высоту.
class Cone : public Circle {
public:
    Cone();
    Cone(float x, float y, float radius, float height);

    float getHeight() const;

private:
    float height_;
};

// Вторичная точка (из гаусса под конусом).
struct GPoint {
    float x;
    float y;
    int cone; // номер конуса, под которым точка родилась
};

// Ребро графа: номера двух точек и длина.
struct Edge {
    int a;
    int b;
    float len;
};

// Кластер: номера точек и центр масс.
struct Cluster {
    std::vector<int> points;
    float cx;
    float cy;
};

// --- Сервер: хранит данные и выполняет все расчёты ---

class Server {
public:
    Server();

    // Читает конусы из файла: первая строка - количество, дальше "x y R h".
    bool loadCones(const std::string& filename);
    // Генерирует perCone точек 2D-гаусса под каждым конусом.
    void generatePoints(int perCone);

    int coneCount() const;
    int pointCount() const;

    // Дерево алгоритмом Прима: перебираем все точки.
    std::vector<Edge> treeFull() const;
    // Дерево алгоритмом Краскала: рёбра ищем только в своём
    // и 8 соседних квадратах сетки gridN x gridN.
    std::vector<Edge> treeGrid(int gridN) const;

    // Гистограмма длин рёбер: файл данных и скрипт gnuplot.
    void writeHistogram(const std::vector<Edge>& edges, const std::string& title,
                        const std::string& dataFile, const std::string& scriptFile,
                        const std::string& imageFile) const;

    // Граф: соединяем точки, которые ближе threshold.
    std::vector<Edge> buildGraph(float threshold) const;
    // Волна по графу. Кластеры идут от большего к меньшему.
    std::vector<Cluster> wave(const std::vector<Edge>& graph) const;
    // Картинка волны: точки и рёбра, у каждого кластера свой цвет.
    void writeWavePlot(const std::vector<Cluster>& clusters,
                       const std::vector<Edge>& graph, float threshold) const;

private:
    std::list<Cone> cones_;
    std::vector<GPoint> points_;
    std::mt19937 gen_;
};

// Квадрат сетки gridN x gridN, в который попадает координата v.
int cellOf(float v, int gridN);

// Правило сортировки рёбер: короткое раньше.
bool shorterEdge(const Edge& a, const Edge& b);

// Правило сортировки кластеров: больший раньше.
bool biggerCluster(const Cluster& a, const Cluster& b);

// Корень дерева, в котором лежит точка v (для алгоритма Краскала).
int findRoot(std::vector<int>& parent, int v);

// --- Control: связь клиента с сервером ---

class Control {
public:
    Control();

    // Проверяет команду и вызывает нужные методы сервера.
    // Возвращает false, когда пришла команда exit.
    bool execute(const std::string& line);

private:
    void help() const;
    void generate(int perCone);
    void trees();
    void waveCommand(float threshold);

    Server server_;
    bool hasData_; // точки уже сгенерированы
};

#endif // FUNCTIONS_H
