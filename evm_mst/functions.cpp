// Группа 212, Корепин Матвей
// Реализация сервера и класса Control.

#include "functions.h"
#include <algorithm>
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>

// --- Point, Circle, Cone ---

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

Circle::Circle() : Point(), radius_(0.0f) {
}

Circle::Circle(float x, float y, float radius) : Point(x, y), radius_(radius) {
}

float Circle::getRadius() const {
    return radius_;
}

Cone::Cone() : Circle(), height_(0.0f) {
}

Cone::Cone(float x, float y, float radius, float height)
    : Circle(x, y, radius), height_(height) {
}

float Cone::getHeight() const {
    return height_;
}

// --- Вспомогательные функции ---

int cellOf(float v, int gridN) {
    int s = (int)std::floor((v - FIELD_MIN) / (FIELD_MAX - FIELD_MIN) * gridN);
    // Точки за краем поля относим к крайнему квадрату.
    if (s < 0) {
        s = 0;
    }
    if (s > gridN - 1) {
        s = gridN - 1;
    }
    return s;
}

bool shorterEdge(const Edge& a, const Edge& b) {
    return a.len < b.len;
}

bool biggerCluster(const Cluster& a, const Cluster& b) {
    return a.points.size() > b.points.size();
}

int findRoot(std::vector<int>& parent, int v) {
    while (parent[v] != v) {
        parent[v] = parent[parent[v]]; // укорачиваем путь к корню
        v = parent[v];
    }
    return v;
}

// Квадрат расстояния между точками.
float dist2(const GPoint& p, const GPoint& q) {
    float dx = p.x - q.x;
    float dy = p.y - q.y;
    return dx * dx + dy * dy;
}

// Раскладывает номера точек по квадратам сетки gridN x gridN.
std::vector<std::vector<int>> makeCells(const std::vector<GPoint>& points,
                                        int gridN) {
    std::vector<std::vector<int>> cells(gridN * gridN);
    for (int i = 0; i < (int)points.size(); ++i) {
        int cx = cellOf(points[i].x, gridN);
        int cy = cellOf(points[i].y, gridN);
        cells[cy * gridN + cx].push_back(i);
    }
    return cells;
}

// Все пары точек i < j из своего и 8 соседних квадратов,
// которые ближе maxLen. maxLen < 0 - брать все такие пары.
std::vector<Edge> neighbourEdges(const std::vector<GPoint>& points,
                                 int gridN, float maxLen) {
    std::vector<std::vector<int>> cells = makeCells(points, gridN);
    std::vector<Edge> edges;
    for (int i = 0; i < (int)points.size(); ++i) {
        int cx = cellOf(points[i].x, gridN);
        int cy = cellOf(points[i].y, gridN);
        for (int dy = -1; dy <= 1; ++dy) {
            for (int dx = -1; dx <= 1; ++dx) {
                int sx = cx + dx;
                int sy = cy + dy;
                if (sx < 0 || sy < 0 || sx >= gridN || sy >= gridN) {
                    continue; // такого квадрата нет
                }
                for (int j : cells[sy * gridN + sx]) {
                    if (j <= i) {
                        continue; // каждую пару берём один раз
                    }
                    float len = std::sqrt(dist2(points[i], points[j]));
                    if (maxLen < 0 || len < maxLen) {
                        Edge e;
                        e.a = i;
                        e.b = j;
                        e.len = len;
                        edges.push_back(e);
                    }
                }
            }
        }
    }
    return edges;
}

// --- Server ---

Server::Server() : gen_(212) {
}

bool Server::loadCones(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    cones_.clear();
    int count = 0;
    file >> count;
    for (int i = 0; i < count; ++i) {
        float x = 0.0f, y = 0.0f, r = 0.0f, h = 0.0f;
        file >> x >> y >> r >> h;
        if (file.fail()) {
            return false;
        }
        cones_.push_back(Cone(x, y, r, h));
    }
    return true;
}

void Server::generatePoints(int perCone) {
    gen_.seed(212); // одно и то же зерно - одни и те же точки
    points_.clear();

    // Высота самого низкого конуса.
    float hMin = cones_.front().getHeight();
    for (const Cone& cone : cones_) {
        if (cone.getHeight() < hMin) {
            hMin = cone.getHeight();
        }
    }

    int coneIndex = 0;
    for (const Cone& cone : cones_) {
        // Чем выше конус, тем меньше сигма: sigma = R * hMin / (2 * h).
        float sigma = cone.getRadius() * hMin / (2.0f * cone.getHeight());
        std::normal_distribution<float> distX(cone.getX(), sigma);
        std::normal_distribution<float> distY(cone.getY(), sigma);
        for (int i = 0; i < perCone; ++i) {
            GPoint p;
            p.x = distX(gen_);
            p.y = distY(gen_);
            p.cone = coneIndex;
            points_.push_back(p);
        }
        ++coneIndex;
    }
}

int Server::coneCount() const {
    return cones_.size();
}

int Server::pointCount() const {
    return points_.size();
}

std::vector<Edge> Server::treeFull() const {
    int n = points_.size();
    std::vector<Edge> edges;
    std::vector<bool> inTree(n, false);
    std::vector<float> d(n);  // квадрат расстояния от точки до дерева
    std::vector<int> from(n); // ближайшая к точке вершина дерева

    // Начинаем дерево с точки 0.
    inTree[0] = true;
    for (int v = 0; v < n; ++v) {
        d[v] = dist2(points_[v], points_[0]);
        from[v] = 0;
    }

    // Каждый раз перебираем все точки и присоединяем ближайшую к дереву.
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
            if (!inTree[u] && dist2(points_[u], points_[v]) < d[u]) {
                d[u] = dist2(points_[u], points_[v]);
                from[u] = v;
            }
        }
    }
    return edges;
}

std::vector<Edge> Server::treeGrid(int gridN) const {
    int n = points_.size();

    // Кандидаты - только пары из соседних квадратов, по возрастанию длины.
    std::vector<Edge> candidates = neighbourEdges(points_, gridN, -1.0f);
    std::sort(candidates.begin(), candidates.end(), shorterEdge);

    // Краскал: сначала каждая точка - отдельное дерево.
    std::vector<int> parent(n);
    for (int i = 0; i < n; ++i) {
        parent[i] = i;
    }

    // Берём ребро, если оно соединяет два разных дерева.
    std::vector<Edge> edges;
    for (const Edge& e : candidates) {
        int ra = findRoot(parent, e.a);
        int rb = findRoot(parent, e.b);
        if (ra != rb) {
            parent[ra] = rb; // сливаем два дерева в одно
            edges.push_back(e);
        }
    }
    return edges;
}

void Server::writeHistogram(const std::vector<Edge>& edges,
                            const std::string& title,
                            const std::string& dataFile,
                            const std::string& scriptFile,
                            const std::string& imageFile) const {
    const int bins = 50;
    const float maxLen = 2.0f;
    float width = maxLen / bins; // ширина одного столбца

    std::vector<int> hist(bins, 0);
    for (const Edge& e : edges) {
        int k = (int)(e.len / width);
        if (k > bins - 1) {
            k = bins - 1; // очень длинные рёбра - в последний столбец
        }
        hist[k]++;
    }

    std::ofstream data(dataFile);
    for (int k = 0; k < bins; ++k) {
        data << (k + 0.5f) * width << " " << hist[k] << "\n";
    }

    std::ofstream out(scriptFile);
    out << "reset\n";
    out << "set terminal png size 900,600\n";
    out << "set output '" << imageFile << "'\n";
    out << "set title '" << title << "'\n";
    out << "set xlabel 'длина ребра'\n";
    out << "set ylabel 'количество рёбер (лог. шкала)'\n";
    out << "set xrange [0:" << maxLen + width << "]\n";
    out << "set logscale y\n";
    out << "set yrange [0.8:*]\n";
    out << "set style fill solid 0.6\n";
    out << "set boxwidth " << width * 0.9f << "\n";
    out << "unset key\n";
    out << "plot '" << dataFile << "' using 1:2 with boxes\n";
}

std::vector<Edge> Server::buildGraph(float threshold) const {
    // Сторона квадрата не меньше порога, поэтому все соседи точки
    // лежат в её квадрате или в 8 соседних.
    int gridN = (int)((FIELD_MAX - FIELD_MIN) / threshold);
    return neighbourEdges(points_, gridN, threshold);
}

std::vector<Cluster> Server::wave(const std::vector<Edge>& graph) const {
    int n = points_.size();

    // Для каждой точки - список соседей по графу.
    std::vector<std::vector<int>> neighbours(n);
    for (const Edge& e : graph) {
        neighbours[e.a].push_back(e.b);
        neighbours[e.b].push_back(e.a);
    }

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
                for (int j : neighbours[i]) {
                    if (a[j] == 0) {
                        a[j] = step + 1;
                        next.push_back(j);
                        cl.points.push_back(j);
                    }
                }
            }
            current = next;
            ++step;
        }

        // Центр масс кластера.
        float sumX = 0.0f;
        float sumY = 0.0f;
        for (int idx : cl.points) {
            sumX += points_[idx].x;
            sumY += points_[idx].y;
        }
        cl.cx = sumX / cl.points.size();
        cl.cy = sumY / cl.points.size();
        clusters.push_back(cl);
    }

    std::sort(clusters.begin(), clusters.end(), biggerCluster);
    return clusters;
}

void Server::writeWavePlot(const std::vector<Cluster>& clusters,
                           const std::vector<Edge>& graph,
                           float threshold) const {
    // 20 хорошо различимых цветов в виде 0xRRGGBB.
    const int COLORS[20] = {
        0xe6194b, 0x3cb44b, 0x4363d8, 0xf58231, 0x911eb4,
        0x42d4f4, 0xf032e6, 0x9a6324, 0x469990, 0x800000,
        0xbfef45, 0x000075, 0xffe119, 0xdcbeff, 0x808000,
        0xfabed4, 0xaaffc3, 0xffd8b1, 0x000000, 0x2f4f4f
    };
    const int GRAY = 0xcccccc; // цвет выбросов

    // Цвет каждой точки по номеру её кластера.
    std::vector<int> color(points_.size(), GRAY);
    for (int k = 0; k < (int)clusters.size(); ++k) {
        if ((int)clusters[k].points.size() < MIN_CLUSTER) {
            continue; // выброс остаётся серым
        }
        for (int idx : clusters[k].points) {
            color[idx] = COLORS[k % 20];
        }
    }

    std::ofstream pts("wave_points.txt");
    for (int i = 0; i < (int)points_.size(); ++i) {
        pts << points_[i].x << " " << points_[i].y << " " << color[i] << "\n";
    }

    // Каждое ребро - две строки и пустая строка, тогда gnuplot
    // рисует его отдельным отрезком. Концы ребра лежат в одном кластере.
    std::ofstream edg("wave_edges.txt");
    for (const Edge& e : graph) {
        edg << points_[e.a].x << " " << points_[e.a].y << " " << color[e.a] << "\n";
        edg << points_[e.b].x << " " << points_[e.b].y << " " << color[e.a] << "\n";
        edg << "\n";
    }

    std::ofstream out("plot_wave.gp");
    out << "reset\n";
    out << "set terminal png size 1000,1000\n";
    out << "set output 'wave.png'\n";
    out << "set title 'Волна, порог " << threshold
        << " (серые - выбросы меньше " << MIN_CLUSTER << " точек)'\n";
    out << "set size square\n";
    out << "set xrange [" << FIELD_MIN << ":" << FIELD_MAX << "]\n";
    out << "set yrange [" << FIELD_MIN << ":" << FIELD_MAX << "]\n";
    out << "unset key\n";
    out << "plot 'wave_edges.txt' using 1:2:3 with lines lc rgb variable lw 0.5, \\\n";
    out << "     'wave_points.txt' using 1:2:3 with points pt 7 ps 0.4 lc rgb variable\n";
}

// --- Control ---

Control::Control() : hasData_(false) {
}

bool Control::execute(const std::string& line) {
    // Разбираем строку: первое слово - команда, второе - число.
    std::istringstream in(line);
    std::string cmd;
    std::string arg;
    in >> cmd >> arg;

    if (cmd.empty()) {
        return true; // пустая строка - ничего не делаем
    }
    if (cmd == "exit") {
        return false;
    }
    if (cmd == "help") {
        help();
        return true;
    }

    if (cmd == "gen") {
        int perCone = 300; // по умолчанию
        if (!arg.empty()) {
            std::istringstream num(arg);
            if (!(num >> perCone)) {
                std::cout << "Ошибка: после gen нужно целое число." << std::endl;
                return true;
            }
        }
        if (perCone < 10 || perCone > 2000) {
            std::cout << "Ошибка: точек на конус должно быть от 10 до 2000." << std::endl;
            return true;
        }
        generate(perCone);
        return true;
    }

    // Остальным командам нужны точки.
    if (cmd == "tree" || cmd == "wave") {
        if (!hasData_) {
            std::cout << "Ошибка: сначала сгенерируйте точки командой gen." << std::endl;
            return true;
        }
    }

    if (cmd == "tree") {
        trees();
        return true;
    }

    if (cmd == "wave") {
        if (arg.empty()) {
            std::cout << "Ошибка: укажите порог, например: wave 0.3" << std::endl;
            return true;
        }
        float threshold = 0.0f;
        std::istringstream num(arg);
        if (!(num >> threshold)) {
            std::cout << "Ошибка: порог должен быть числом." << std::endl;
            return true;
        }
        if (threshold < 0.05f || threshold > 2.0f) {
            std::cout << "Ошибка: порог должен быть от 0.05 до 2." << std::endl;
            return true;
        }
        waveCommand(threshold);
        return true;
    }

    std::cout << "Неизвестная команда: " << cmd << ". Список команд: help" << std::endl;
    return true;
}

void Control::help() const {
    std::cout << "Команды:" << std::endl;
    std::cout << "  gen N    - прочитать конусы из cones.txt и создать N точек под каждым"
              << " (без N - 300)" << std::endl;
    std::cout << "  tree     - построить дерево двумя способами, замерить время,"
              << " нарисовать гистограммы" << std::endl;
    std::cout << "  wave T   - найти кластеры волной с порогом T" << std::endl;
    std::cout << "  help     - список команд" << std::endl;
    std::cout << "  exit     - выход" << std::endl;
}

void Control::generate(int perCone) {
    if (!server_.loadCones("cones.txt")) {
        std::cout << "Ошибка: не удалось прочитать cones.txt." << std::endl;
        hasData_ = false;
        return;
    }
    server_.generatePoints(perCone);
    hasData_ = true;
    std::cout << "Конусов: " << server_.coneCount()
              << ", точек: " << server_.pointCount() << std::endl;
}

void Control::trees() {
    // Время считаем через clock(): процессорное время программы.
    clock_t start = clock();
    std::vector<Edge> full = server_.treeFull();
    double fullMs = 1000.0 * (clock() - start) / CLOCKS_PER_SEC;

    start = clock();
    std::vector<Edge> grid = server_.treeGrid(TREE_GRID_N);
    double gridMs = 1000.0 * (clock() - start) / CLOCKS_PER_SEC;

    float square = (FIELD_MAX - FIELD_MIN) / TREE_GRID_N;
    int n = server_.pointCount();

    // Рёбра короче стороны квадрата оба способа обязаны найти одинаково.
    int fullShort = 0;
    int gridShort = 0;
    float fullSum = 0.0f;
    float gridSum = 0.0f;
    for (const Edge& e : full) {
        if (e.len < square) {
            ++fullShort;
            fullSum += e.len;
        }
    }
    for (const Edge& e : grid) {
        if (e.len < square) {
            ++gridShort;
            gridSum += e.len;
        }
    }

    std::cout << "Перебор всех точек (Прим):" << std::endl;
    std::cout << "  время " << fullMs << " мс, рёбер " << full.size()
              << ", кусков 1" << std::endl;
    std::cout << "Поиск в 9 квадратах сетки " << TREE_GRID_N << "x" << TREE_GRID_N
              << " (Краскал):" << std::endl;
    std::cout << "  время " << gridMs << " мс, рёбер " << grid.size()
              << ", кусков " << n - (int)grid.size() << std::endl;
    std::cout << "Рёбра короче стороны квадрата " << square << ":" << std::endl;
    std::cout << "  перебор: " << fullShort << " рёбер, сумма длин " << fullSum << std::endl;
    std::cout << "  сетка:   " << gridShort << " рёбер, сумма длин " << gridSum << std::endl;

    server_.writeHistogram(full, "Длины рёбер дерева: перебор всех точек",
                           "hist_full.txt", "plot_hist_full.gp", "hist_full.png");
    server_.writeHistogram(grid, "Длины рёбер дерева: поиск в 9 квадратах",
                           "hist_grid.txt", "plot_hist_grid.gp", "hist_grid.png");
    std::cout << "Гистограммы: gnuplot plot_hist_full.gp plot_hist_grid.gp" << std::endl;
}

void Control::waveCommand(float threshold) {
    std::vector<Edge> graph = server_.buildGraph(threshold);
    std::vector<Cluster> clusters = server_.wave(graph);
    server_.writeWavePlot(clusters, graph, threshold);

    int big = 0;
    int small = 0;
    std::cout << "Рёбер в графе: " << graph.size()
              << ", всего компонент: " << clusters.size() << std::endl;
    for (const Cluster& cl : clusters) {
        if ((int)cl.points.size() < MIN_CLUSTER) {
            ++small;
            continue;
        }
        ++big;
        std::cout << "  Кластер " << big << ": точек " << cl.points.size()
                  << ", центр (" << cl.cx << ", " << cl.cy << ")" << std::endl;
    }
    std::cout << "Крупных кластеров: " << big << ", выбросов: " << small << std::endl;
    std::cout << "Картинка: gnuplot plot_wave.gp" << std::endl;
}
