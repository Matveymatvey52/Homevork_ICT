// Группа 212, Корепин Матвей
// Реализация всех классов и функций, объявленных в functions.h.

#include "functions.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <limits>

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
    return static_cast<float>(M_PI) * radius_ * radius_;
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
    return (1.0f / 3.0f) * static_cast<float>(M_PI) * r * r * height_;
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

        coneList.push_back(Cone(x, y, r, h)); // добавляем в конец списка STL
    }

    return true;
}

// --- Визуализация конусов через gnuplot ---

namespace {
// Число точек, которыми аппроксимируется окружность основания конуса.
const int CIRCLE_POINTS = 40;
}

void writeConeDataFile(const std::list<Cone>& coneList, const std::string& dataFilename) {
    std::ofstream out(dataFilename);

    for (const Cone& cone : coneList) {
        float cx = cone.getX();
        float cy = cone.getY();
        float r = cone.getRadius();
        float h = cone.getHeight();

        // Точки окружности основания (z = 0): угол пробегает полный круг.
        for (int i = 0; i <= CIRCLE_POINTS; ++i) {
            float angle = 2.0f * static_cast<float>(M_PI) * i / CIRCLE_POINTS;
            float px = cx + r * std::cos(angle);
            float py = cy + r * std::sin(angle);
            out << px << " " << py << " " << 0.0f << "\n";
        }

        out << "\n"; // пустая строка - разделитель блоков для gnuplot

        // Боковые линии от основания к вершине, через одну точку из четырёх.
        for (int i = 0; i <= CIRCLE_POINTS; i += 4) {
            float angle = 2.0f * static_cast<float>(M_PI) * i / CIRCLE_POINTS;
            float px = cx + r * std::cos(angle);
            float py = cy + r * std::sin(angle);
            out << px << " " << py << " " << 0.0f << "\n";
            out << cx << " " << cy << " " << h << "\n";
            out << "\n";
        }
    }
}

void writeGnuplotScript(const std::string& scriptFilename,
                         const std::string& dataFilename,
                         const std::string& imageFilename,
                         const Field& field) {
    std::ofstream out(scriptFilename);

    out << "set terminal png size 900,700\n";
    out << "set output '" << imageFilename << "'\n";
    out << "set title 'Конусы на плоскости'\n";
    out << "set xlabel 'x'\n";
    out << "set ylabel 'y'\n";
    out << "set zlabel 'h'\n";
    out << "set xrange [" << field.getMinX() << ":" << field.getMaxX() << "]\n";
    out << "set yrange [" << field.getMinY() << ":" << field.getMaxY() << "]\n";
    out << "set view 60, 30\n";
    out << "splot '" << dataFilename << "' with lines notitle\n";
}

// --- Grid ---

Grid::Grid(const Field& field, int n)
    : minX_(field.getMinX()), minY_(field.getMinY()),
      stepX_((field.getMaxX() - field.getMinX()) / n),
      stepY_((field.getMaxY() - field.getMinY()) / n), n_(n) {
}

int Grid::getN() const {
    return n_;
}

float Grid::getStep() const {
    return stepX_;
}

// Точки за краем поля прижимаем к крайнему квадрату.
int Grid::squareX(float x) const {
    int s = static_cast<int>(std::floor((x - minX_) / stepX_));
    return std::clamp(s, 0, n_ - 1);
}

int Grid::squareY(float y) const {
    int s = static_cast<int>(std::floor((y - minY_) / stepY_));
    return std::clamp(s, 0, n_ - 1);
}

void sortConesBySquares(std::list<Cone>& coneList, const Grid& grid) {
    // У std::list свой метод sort: обычный std::sort для списка не
    // подходит, ему нужен произвольный доступ по индексу.
    coneList.sort([&grid](const Cone& a, const Cone& b) {
        int ay = grid.squareY(a.getY());
        int by = grid.squareY(b.getY());
        if (ay != by) {
            return ay < by;
        }
        return grid.squareX(a.getX()) < grid.squareX(b.getX());
    });
}

// --- Вторичные точки ---

float coneSigma(const Cone& cone, float hMin) {
    return cone.getRadius() * hMin / (2.0f * cone.getHeight());
}

std::vector<GPoint> generateGaussPoints(const std::list<Cone>& coneList,
                                        int perCone, std::mt19937& gen,
                                        const Grid& grid) {
    float hMin = std::numeric_limits<float>::max();
    for (const Cone& cone : coneList) {
        hMin = std::min(hMin, cone.getHeight());
    }

    std::vector<GPoint> points;
    points.reserve(coneList.size() * perCone);

    int coneIndex = 0;
    for (const Cone& cone : coneList) {
        float sigma = coneSigma(cone, hMin);
        // x и y независимы: связь между ними нулевая, облако круглое.
        std::normal_distribution<float> distX(cone.getX(), sigma);
        std::normal_distribution<float> distY(cone.getY(), sigma);

        for (int i = 0; i < perCone; ++i) {
            GPoint p;
            p.x = distX(gen);
            p.y = distY(gen);
            p.s1 = grid.squareX(p.x);
            p.s2 = grid.squareY(p.y);
            p.cone = coneIndex;
            points.push_back(p);
        }
        ++coneIndex;
    }
    return points;
}

void sortPointsBySquares(std::vector<GPoint>& points) {
    std::sort(points.begin(), points.end(),
              [](const GPoint& a, const GPoint& b) {
                  if (a.s2 != b.s2) {
                      return a.s2 < b.s2;
                  }
                  return a.s1 < b.s1;
              });
}

void writePointsFile(const std::vector<GPoint>& points,
                     const std::vector<int>& labels,
                     const std::string& filename) {
    std::ofstream out(filename);
    for (size_t i = 0; i < points.size(); ++i) {
        out << points[i].x << " " << points[i].y << " " << labels[i] << "\n";
    }
}

void writePointsScript(const std::string& scriptFilename,
                       const std::string& dataFilename,
                       const std::string& imageFilename,
                       const std::string& title,
                       const Field& field) {
    std::ofstream out(scriptFilename);
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
    cl.minX = cl.minY = std::numeric_limits<float>::max();
    cl.maxX = cl.maxY = -std::numeric_limits<float>::max();
    float sumX = 0.0f;
    float sumY = 0.0f;
    for (int idx : cl.points) {
        const GPoint& p = points[idx];
        cl.minX = std::min(cl.minX, p.x);
        cl.maxX = std::max(cl.maxX, p.x);
        cl.minY = std::min(cl.minY, p.y);
        cl.maxY = std::max(cl.maxY, p.y);
        sumX += p.x;
        sumY += p.y;
    }
    cl.cx = sumX / cl.points.size();
    cl.cy = sumY / cl.points.size();
}

std::vector<int> clusterLabels(const std::vector<Cluster>& clusters, size_t n) {
    std::vector<int> labels(n, 0);
    for (size_t k = 0; k < clusters.size(); ++k) {
        for (int idx : clusters[k].points) {
            labels[idx] = static_cast<int>(k);
        }
    }
    return labels;
}

// Сортируем кластеры по убыванию размера: крупные идут первыми.
static void sortClustersBySize(std::vector<Cluster>& clusters) {
    std::sort(clusters.begin(), clusters.end(),
              [](const Cluster& a, const Cluster& b) {
                  return a.points.size() > b.points.size();
              });
}

// --- Алгоритм "Волна" ---

std::vector<char> buildIncidenceMatrix(const std::vector<GPoint>& points,
                                       const Grid& grid, float threshold) {
    const int n = static_cast<int>(points.size());
    const int g = grid.getN();
    std::vector<char> matrix(static_cast<size_t>(n) * n, 0);

    // Раскладываем номера точек по квадратам сетки.
    std::vector<std::vector<int>> cells(static_cast<size_t>(g) * g);
    for (int i = 0; i < n; ++i) {
        cells[points[i].s2 * g + points[i].s1].push_back(i);
    }

    // Для каждой точки проверяем только 9 квадратов: свой и 8 соседних.
    const float t2 = threshold * threshold;
    for (int i = 0; i < n; ++i) {
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                int sx = points[i].s1 + dx;
                int sy = points[i].s2 + dy;
                if (sx < 0 || sy < 0 || sx >= g || sy >= g) {
                    continue;
                }
                for (int j : cells[sy * g + sx]) {
                    if (j == i) {
                        continue;
                    }
                    float ddx = points[i].x - points[j].x;
                    float ddy = points[i].y - points[j].y;
                    if (ddx * ddx + ddy * ddy < t2) {
                        matrix[static_cast<size_t>(i) * n + j] = 1;
                    }
                }
            }
        }
    }
    return matrix;
}

std::vector<Cluster> waveClusters(const std::vector<GPoint>& points,
                                  const std::vector<char>& matrix) {
    const int n = static_cast<int>(points.size());
    // a[i] = 0 - волна до точки не дошла; a[i] = k - дошла на шаге k.
    std::vector<int> a(n, 0);
    std::vector<Cluster> clusters;
    std::vector<int> current; // горящие точки текущего шага
    std::vector<int> next;    // точки, которые загорятся на следующем

    for (int k = 0; k < n; ++k) {
        if (a[k] != 0) {
            continue;
        }
        // Поджигаем точку k - начинается новый кластер.
        Cluster cl;
        a[k] = 1;
        current.assign(1, k);
        cl.points.push_back(k);
        int step = 1;

        while (!current.empty()) {
            next.clear();
            for (int i : current) {
                const char* row = &matrix[static_cast<size_t>(i) * n];
                for (int j = 0; j < n; ++j) {
                    if (row[j] && a[j] == 0) {
                        a[j] = step + 1;
                        next.push_back(j);
                        cl.points.push_back(j);
                    }
                }
            }
            current.swap(next); // следующий шаг становится текущим
            ++step;
        }

        computeClusterStats(cl, points);
        clusters.push_back(cl);
    }

    sortClustersBySize(clusters);
    return clusters;
}

// --- Минимальное покрывающее дерево ---

static float dist2(const GPoint& p, const GPoint& q) {
    float dx = p.x - q.x;
    float dy = p.y - q.y;
    return dx * dx + dy * dy;
}

std::vector<Edge> buildSpanningTree(const std::vector<GPoint>& points) {
    const int n = static_cast<int>(points.size());
    std::vector<Edge> edges;
    if (n < 2) {
        return edges;
    }

    // База: самое короткое ребро из всех пар точек.
    int a0 = 0;
    int b0 = 1;
    float best = dist2(points[0], points[1]);
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            float d = dist2(points[i], points[j]);
            if (d < best) {
                best = d;
                a0 = i;
                b0 = j;
            }
        }
    }

    std::vector<char> inTree(n, 0);
    std::vector<float> d(n);    // квадрат расстояния от точки до дерева
    std::vector<int> from(n);   // ближайшая к точке вершина дерева
    inTree[a0] = inTree[b0] = 1;
    edges.push_back({a0, b0, std::sqrt(best)});

    for (int v = 0; v < n; ++v) {
        float da = dist2(points[v], points[a0]);
        float db = dist2(points[v], points[b0]);
        d[v] = std::min(da, db);
        from[v] = (da < db) ? a0 : b0;
    }

    // Шаг: присоединяем ближайшую к дереву точку, пока рёбер не n-1.
    while (static_cast<int>(edges.size()) < n - 1) {
        int v = -1;
        for (int u = 0; u < n; ++u) {
            if (!inTree[u] && (v < 0 || d[u] < d[v])) {
                v = u;
            }
        }
        inTree[v] = 1;
        edges.push_back({from[v], v, std::sqrt(d[v])});

        // Расстояние до дерева могло уменьшиться за счёт новой вершины.
        for (int u = 0; u < n; ++u) {
            if (!inTree[u]) {
                float du = dist2(points[u], points[v]);
                if (du < d[u]) {
                    d[u] = du;
                    from[u] = v;
                }
            }
        }
    }
    return edges;
}

std::vector<int> edgeHistogram(const std::vector<Edge>& edges,
                               int bins, float maxLen) {
    std::vector<int> hist(bins, 0);
    for (const Edge& e : edges) {
        int k = static_cast<int>(e.len / maxLen * bins);
        hist[std::min(k, bins - 1)]++;
    }
    return hist;
}

void writeHistogram(const std::vector<int>& hist, float maxLen,
                    float threshold, const std::string& dataFilename,
                    const std::string& scriptFilename,
                    const std::string& imageFilename) {
    const float width = maxLen / hist.size();
    std::ofstream data(dataFilename);
    for (size_t k = 0; k < hist.size(); ++k) {
        data << (k + 0.5f) * width << " " << hist[k] << "\n";
    }

    std::ofstream out(scriptFilename);
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
    const int n = static_cast<int>(points.size());

    // Оставляем только рёбра не длиннее порога.
    std::vector<std::vector<int>> adj(n);
    for (const Edge& e : edges) {
        if (e.len <= threshold) {
            adj[e.a].push_back(e.b);
            adj[e.b].push_back(e.a);
        }
    }

    // Обходим оставшиеся куски дерева, каждый кусок - кластер.
    std::vector<char> seen(n, 0);
    std::vector<Cluster> clusters;
    for (int k = 0; k < n; ++k) {
        if (seen[k]) {
            continue;
        }
        Cluster cl;
        std::vector<int> queue(1, k);
        seen[k] = 1;
        for (size_t q = 0; q < queue.size(); ++q) {
            int v = queue[q];
            cl.points.push_back(v);
            for (int u : adj[v]) {
                if (!seen[u]) {
                    seen[u] = 1;
                    queue.push_back(u);
                }
            }
        }
        computeClusterStats(cl, points);
        clusters.push_back(cl);
    }

    sortClustersBySize(clusters);
    return clusters;
}
