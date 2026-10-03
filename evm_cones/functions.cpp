// Группа 212, Корепин Матвей
// Реализация всех классов и функций, объявленных в functions.h.

#include "functions.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>

// --- Point ---

Point::Point() : x_(0.0f), y_(0.0f) {
}

Point::Point(float x, float y) : x_(x), y_(y) {
}

float Point::getX() const {
    return x_;
}

float Point::getY() const {
    return y_;
}

void Point::setX(float x) {
    x_ = x;
}

void Point::setY(float y) {
    y_ = y;
}

// --- Circle ---

// ": Point(), radius_(0.0f)" - список инициализации: сначала вызывается
// конструктор базового класса, потом задаётся начальное значение поля.
Circle::Circle() : Point(), radius_(0.0f) {
}

Circle::Circle(float x, float y, float radius) : Point(x, y), radius_(radius) {
}

float Circle::getRadius() const {
    return radius_;
}

void Circle::setRadius(float radius) {
    radius_ = radius;
}

float Circle::area() const {
    // Площадь круга: S = pi * r^2.
    return M_PI * radius_ * radius_;
}

// --- Cone ---

Cone::Cone() : Circle(), height_(0.0f) {
}

Cone::Cone(float x, float y, float radius, float height)
    : Circle(x, y, radius), height_(height) {
}

float Cone::getHeight() const {
    return height_;
}

void Cone::setHeight(float height) {
    height_ = height;
}

float Cone::volume() const {
    // Объём конуса: V = (1/3) * pi * r^2 * h.
    float r = getRadius(); // унаследовано от Circle
    return M_PI * r * r * height_ / 3.0f;
}

// --- Field ---

Field::Field() : minX_(0.0f), maxX_(0.0f), minY_(0.0f), maxY_(0.0f) {
}

Field::Field(float minX, float maxX, float minY, float maxY)
    : minX_(minX), maxX_(maxX), minY_(minY), maxY_(maxY) {
}

float Field::getMinX() const {
    return minX_;
}

float Field::getMaxX() const {
    return maxX_;
}

float Field::getMinY() const {
    return minY_;
}

float Field::getMaxY() const {
    return maxY_;
}

// --- Чтение конусов из файла ---

bool loadConesFromFile(const std::string& filename, std::list<Cone>& coneList) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Не удалось открыть файл: " << filename << std::endl;
        return false;
    }

    int count = 0;
    file >> count; // первая строка файла - количество конусов

    for (int i = 0; i < count; ++i) {
        float x = 0.0f, y = 0.0f, r = 0.0f, h = 0.0f;
        file >> x >> y >> r >> h;

        // Проверка, что числа прочитались (в файле их хватило).
        if (file.fail()) {
            std::cerr << "Ошибка чтения конуса номер " << i << std::endl;
            return false;
        }
        coneList.push_back(Cone(x, y, r, h)); // добавляем в конец списка
    }
    return true;
}

// --- Сетка квадратов ---

int squareX(float x) {
    int s = (int)std::floor((x - FIELD_MIN) / SQUARE);
    // Точки за краем поля относим к крайнему квадрату.
    if (s < 0) {
        s = 0;
    }
    if (s > GRID_N - 1) {
        s = GRID_N - 1;
    }
    return s;
}

int squareY(float y) {
    int s = (int)std::floor((y - FIELD_MIN) / SQUARE);
    if (s < 0) {
        s = 0;
    }
    if (s > GRID_N - 1) {
        s = GRID_N - 1;
    }
    return s;
}

bool coneLess(const Cone& a, const Cone& b) {
    if (squareY(a.getY()) != squareY(b.getY())) {
        return squareY(a.getY()) < squareY(b.getY());
    }
    return squareX(a.getX()) < squareX(b.getX());
}

// --- Вторичные точки ---

float coneSigma(const Cone& cone, float hMin) {
    return cone.getRadius() * hMin / (2.0f * cone.getHeight());
}

std::vector<GPoint> generateGaussPoints(const std::list<Cone>& coneList,
                                        int perCone, std::mt19937& gen) {
    // Ищем высоту самого низкого конуса.
    float hMin = coneList.front().getHeight();
    for (const Cone& cone : coneList) {
        if (cone.getHeight() < hMin) {
            hMin = cone.getHeight();
        }
    }

    std::vector<GPoint> points;
    int coneIndex = 0;
    for (const Cone& cone : coneList) {
        float sigma = coneSigma(cone, hMin);
        // x и y генерируем независимо - облако получается круглым.
        std::normal_distribution<float> distX(cone.getX(), sigma);
        std::normal_distribution<float> distY(cone.getY(), sigma);

        for (int i = 0; i < perCone; ++i) {
            GPoint p;
            p.x = distX(gen);
            p.y = distY(gen);
            p.s1 = squareX(p.x);
            p.s2 = squareY(p.y);
            p.cone = coneIndex;
            points.push_back(p);
        }
        ++coneIndex;
    }
    return points;
}

bool pointLess(const GPoint& a, const GPoint& b) {
    if (a.s2 != b.s2) {
        return a.s2 < b.s2;
    }
    return a.s1 < b.s1;
}

void writePointsFile(const std::vector<GPoint>& points,
                     const std::vector<int>& labels,
                     const std::string& filename) {
    std::ofstream out(filename);
    for (int i = 0; i < (int)points.size(); ++i) {
        out << points[i].x << " " << points[i].y << " " << labels[i] << "\n";
    }
}

void writePointsScript(const std::string& scriptFilename,
                       const std::string& dataFilename,
                       const std::string& imageFilename,
                       const std::string& title,
                       const Field& field) {
    std::ofstream out(scriptFilename);
    out << "reset\n"; // сбросить настройки, оставшиеся от прошлого скрипта
    out << "set terminal png size 900,900\n";
    out << "set output '" << imageFilename << "'\n";
    out << "set title '" << title << "'\n";
    out << "set xlabel 'x'\n";
    out << "set ylabel 'y'\n";
    out << "set size square\n";
    out << "set xrange [" << field.getMinX() << ":" << field.getMaxX() << "]\n";
    out << "set yrange [" << field.getMinY() << ":" << field.getMaxY() << "]\n";
    out << "unset key\n";
    // Третий столбец - метка, lc variable красит точки по номеру метки.
    out << "plot '" << dataFilename
        << "' using 1:2:($3+1) with points pt 7 ps 0.4 lc variable\n";
}

// --- Кластеры ---

void computeClusterStats(Cluster& cl, const std::vector<GPoint>& points) {
    // Начальные значения - координаты первой точки кластера.
    const GPoint& first = points[cl.points[0]];
    cl.minX = cl.maxX = first.x;
    cl.minY = cl.maxY = first.y;

    float sumX = 0.0f;
    float sumY = 0.0f;
    for (int idx : cl.points) {
        const GPoint& p = points[idx];
        if (p.x < cl.minX) cl.minX = p.x;
        if (p.x > cl.maxX) cl.maxX = p.x;
        if (p.y < cl.minY) cl.minY = p.y;
        if (p.y > cl.maxY) cl.maxY = p.y;
        sumX += p.x;
        sumY += p.y;
    }
    cl.cx = sumX / cl.points.size();
    cl.cy = sumY / cl.points.size();
}

bool biggerCluster(const Cluster& a, const Cluster& b) {
    return a.points.size() > b.points.size();
}

std::vector<int> clusterLabels(const std::vector<Cluster>& clusters, int n) {
    std::vector<int> labels(n, 0);
    for (int k = 0; k < (int)clusters.size(); ++k) {
        for (int idx : clusters[k].points) {
            labels[idx] = k;
        }
    }
    return labels;
}

// --- Алгоритм "Волна" ---

std::vector<char> buildIncidenceMatrix(const std::vector<GPoint>& points,
                                       float threshold) {
    int n = points.size();
    std::vector<char> matrix(n * n, 0);

    // Раскладываем номера точек по квадратам сетки.
    std::vector<std::vector<int>> cells(GRID_N * GRID_N);
    for (int i = 0; i < n; ++i) {
        cells[points[i].s2 * GRID_N + points[i].s1].push_back(i);
    }

    // Для каждой точки смотрим только 9 квадратов: свой и 8 соседних.
    for (int i = 0; i < n; ++i) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                int sx = points[i].s1 + dx;
                int sy = points[i].s2 + dy;
                if (sx < 0 || sy < 0 || sx >= GRID_N || sy >= GRID_N) {
                    continue; // такого квадрата нет - вышли за поле
                }
                for (int j : cells[sy * GRID_N + sx]) {
                    float ddx = points[i].x - points[j].x;
                    float ddy = points[i].y - points[j].y;
                    if (j != i && ddx * ddx + ddy * ddy < threshold * threshold) {
                        matrix[i * n + j] = 1;
                    }
                }
            }
        }
    }
    return matrix;
}

std::vector<Cluster> waveClusters(const std::vector<GPoint>& points,
                                  const std::vector<char>& matrix) {
    int n = points.size();
    // a[i] = 0 - волна до точки не дошла; a[i] = k - дошла на шаге k.
    std::vector<int> a(n, 0);
    std::vector<Cluster> clusters;
    std::vector<int> current; // горящие точки текущего шага
    std::vector<int> next;    // точки, которые загорятся на следующем

    for (int k = 0; k < n; ++k) {
        if (a[k] != 0) {
            continue; // точка уже в каком-то кластере
        }
        // Поджигаем точку k - начинается новый кластер.
        Cluster cl;
        a[k] = 1;
        current.clear();
        current.push_back(k);
        cl.points.push_back(k);
        int step = 1;

        while (!current.empty()) {
            next.clear();
            for (int i : current) {
                for (int j = 0; j < n; ++j) {
                    if (matrix[i * n + j] == 1 && a[j] == 0) {
                        a[j] = step + 1;
                        next.push_back(j);
                        cl.points.push_back(j);
                    }
                }
            }
            current = next; // следующий шаг становится текущим
            ++step;
        }

        computeClusterStats(cl, points);
        clusters.push_back(cl);
    }

    std::sort(clusters.begin(), clusters.end(), biggerCluster);
    return clusters;
}

// --- Минимальное покрывающее дерево ---

// Квадрат расстояния между точками (корень извлекаем только в конце).
float dist2(const GPoint& p, const GPoint& q) {
    float dx = p.x - q.x;
    float dy = p.y - q.y;
    return dx * dx + dy * dy;
}

std::vector<Edge> buildSpanningTree(const std::vector<GPoint>& points) {
    int n = points.size();
    std::vector<Edge> edges;

    std::vector<bool> inTree(n, false);
    std::vector<float> d(n);  // квадрат расстояния от точки до дерева
    std::vector<int> from(n); // ближайшая к точке вершина дерева

    // Начинаем дерево с точки 0.
    inTree[0] = true;
    for (int v = 0; v < n; ++v) {
        d[v] = dist2(points[v], points[0]);
        from[v] = 0;
    }

    // Присоединяем ближайшую к дереву точку, пока рёбер не станет n-1.
    while ((int)edges.size() < n - 1) {
        int v = -1;
        for (int u = 0; u < n; ++u) {
            if (!inTree[u] && (v == -1 || d[u] < d[v])) {
                v = u;
            }
        }
        inTree[v] = true;

        Edge e;
        e.a = from[v];
        e.b = v;
        e.len = std::sqrt(d[v]);
        edges.push_back(e);

        // Новая вершина могла оказаться ближе к остальным точкам.
        for (int u = 0; u < n; ++u) {
            if (!inTree[u] && dist2(points[u], points[v]) < d[u]) {
                d[u] = dist2(points[u], points[v]);
                from[u] = v;
            }
        }
    }
    return edges;
}

std::vector<int> edgeHistogram(const std::vector<Edge>& edges,
                               int bins, float maxLen) {
    std::vector<int> hist(bins, 0);
    for (const Edge& e : edges) {
        int k = (int)(e.len / maxLen * bins);
        if (k > bins - 1) {
            k = bins - 1; // очень длинные рёбра - в последний столбец
        }
        hist[k]++;
    }
    return hist;
}

void writeHistogram(const std::vector<int>& hist, float maxLen,
                    float threshold, const std::string& dataFilename,
                    const std::string& scriptFilename,
                    const std::string& imageFilename) {
    float width = maxLen / hist.size(); // ширина одного столбца
    std::ofstream data(dataFilename);
    for (int k = 0; k < (int)hist.size(); ++k) {
        data << (k + 0.5f) * width << " " << hist[k] << "\n";
    }

    std::ofstream out(scriptFilename);
    out << "reset\n";
    out << "set terminal png size 900,600\n";
    out << "set output '" << imageFilename << "'\n";
    out << "set title 'Гистограмма длин рёбер покрывающего дерева'\n";
    out << "set xlabel 'длина ребра'\n";
    out << "set ylabel 'количество рёбер (лог. шкала)'\n";
    out << "set xrange [0:" << maxLen + width << "]\n";
    out << "set logscale y\n";
    out << "set yrange [0.8:*]\n";
    out << "set style fill solid 0.6\n";
    out << "set boxwidth " << width * 0.9f << "\n";
    out << "set arrow from " << threshold << ", graph 0 to " << threshold
        << ", graph 1 nohead lw 2 lc rgb 'red'\n";
    out << "set label 'порог' at " << threshold << ", graph 0.95 offset 1,0 tc rgb 'red'\n";
    out << "unset key\n";
    out << "plot '" << dataFilename << "' using 1:2 with boxes\n";
}

std::vector<Cluster> treeClusters(const std::vector<GPoint>& points,
                                  const std::vector<Edge>& edges,
                                  float threshold) {
    int n = points.size();

    // Для каждой точки - список соседей по рёбрам не длиннее порога.
    std::vector<std::vector<int>> neighbours(n);
    for (const Edge& e : edges) {
        if (e.len <= threshold) {
            neighbours[e.a].push_back(e.b);
            neighbours[e.b].push_back(e.a);
        }
    }

    // Обходим оставшиеся куски дерева, каждый кусок - кластер.
    std::vector<bool> seen(n, false);
    std::vector<Cluster> clusters;
    for (int k = 0; k < n; ++k) {
        if (seen[k]) {
            continue;
        }
        Cluster cl;
        std::vector<int> queue;
        queue.push_back(k);
        seen[k] = true;
        for (int q = 0; q < (int)queue.size(); ++q) {
            int v = queue[q];
            cl.points.push_back(v);
            for (int u : neighbours[v]) {
                if (!seen[u]) {
                    seen[u] = true;
                    queue.push_back(u);
                }
            }
        }
        computeClusterStats(cl, points);
        clusters.push_back(cl);
    }

    std::sort(clusters.begin(), clusters.end(), biggerCluster);
    return clusters;
}
