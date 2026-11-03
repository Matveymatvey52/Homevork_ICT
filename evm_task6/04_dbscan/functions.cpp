// Группа 212, Корепин Матвей
// Реализация классов: поле с муравьиными тропами, матрицы D и B,
// фильтр DBSCAN, волна, рисунки, проверки и диспетчер Control.

#include "functions.h"
#include <cmath>
#include <ctime>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>

// --- Вспомогательные функции ---

// Убирает пробелы в начале и в конце строки.
std::string trim(const std::string& s) {
    int begin = 0;
    int end = (int)s.size();
    while (begin < end && (s[begin] == ' ' || s[begin] == '\t' || s[begin] == '\r')) {
        ++begin;
    }
    while (end > begin && (s[end - 1] == ' ' || s[end - 1] == '\t' || s[end - 1] == '\r')) {
        --end;
    }
    return s.substr(begin, end - begin);
}

// Правило сортировки кластеров: больший раньше.
bool biggerCluster(const Cluster& a, const Cluster& b) {
    return a.points.size() > b.points.size();
}

// Корень дерева, в котором лежит точка v (для проверки связности).
int findRoot(std::vector<int>& parent, int v) {
    while (parent[v] != v) {
        parent[v] = parent[parent[v]]; // укорачиваем путь к корню
        v = parent[v];
    }
    return v;
}

// Номер клетки (i, j), i < j, в половине матрицы над диагональю.
long long triIndex(int i, int j, int n) {
    return (long long)i * (2LL * n - i - 1) / 2 + (j - i - 1);
}

// --- Config ---

bool Config::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    std::string line;
    while (std::getline(file, line)) {
        // Всё после # - комментарий.
        std::size_t hash = line.find('#');
        if (hash != std::string::npos) {
            line = line.substr(0, hash);
        }
        std::size_t eq = line.find('=');
        if (eq == std::string::npos) {
            continue; // пустая строка или без "="
        }
        std::string key = trim(line.substr(0, eq));
        std::string value = trim(line.substr(eq + 1));
        if (!key.empty()) {
            values_[key] = value;
        }
    }
    return true;
}

int Config::getInt(const std::string& key, int def) const {
    std::map<std::string, std::string>::const_iterator it = values_.find(key);
    if (it == values_.end()) {
        return def;
    }
    std::istringstream in(it->second);
    int value = def;
    in >> value;
    return value;
}

float Config::getFloat(const std::string& key, float def) const {
    std::map<std::string, std::string>::const_iterator it = values_.find(key);
    if (it == values_.end()) {
        return def;
    }
    std::istringstream in(it->second);
    float value = def;
    in >> value;
    return value;
}

std::string Config::getString(const std::string& key, const std::string& def) const {
    std::map<std::string, std::string>::const_iterator it = values_.find(key);
    if (it == values_.end()) {
        return def;
    }
    return it->second;
}

void Config::print() const {
    std::map<std::string, std::string>::const_iterator it;
    for (it = values_.begin(); it != values_.end(); ++it) {
        std::cout << "  " << it->first << " = " << it->second << std::endl;
    }
}

bool loadCones(const std::string& filename, std::vector<Cone>& cones) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        return false;
    }
    int count = 0;
    file >> count;
    cones.clear();
    for (int i = 0; i < count; ++i) {
        Cone c;
        file >> c.x >> c.y >> c.r >> c.h;
        if (file.fail()) {
            return false;
        }
        cones.push_back(c);
    }
    return count > 0;
}

// --- Field ---

void Field::setCones(const std::vector<Cone>& cones) {
    cones_ = cones;
}

void Field::generate(int perCone, float sigma, int seed) {
    std::mt19937 gen(seed);
    points_.clear();

    float hMin = cones_[0].h;
    for (const Cone& c : cones_) {
        if (c.h < hMin) {
            hMin = c.h;
        }
    }

    for (int k = 0; k < (int)cones_.size(); ++k) {
        // Чем выше конус, тем уже колокол: σ = sigma * hMin / h.
        float s = sigma * hMin / cones_[k].h;
        std::normal_distribution<float> distX(cones_[k].x, s);
        std::normal_distribution<float> distY(cones_[k].y, s);
        for (int i = 0; i < perCone; ++i) {
            GPoint p;
            p.x = distX(gen);
            p.y = distY(gen);
            p.cone = k;
            points_.push_back(p);
        }
    }
}

void Field::addAntTrails(float step) {
    // Тропа идёт по прямой от центра одного конуса к центру другого.
    const int FROM[2] = {0, 3};
    const int TO[2] = {2, 4};
    for (int t = 0; t < 2; ++t) {
        if (TO[t] >= (int)cones_.size()) {
            continue;
        }
        float x0 = cones_[FROM[t]].x;
        float y0 = cones_[FROM[t]].y;
        float dx = cones_[TO[t]].x - x0;
        float dy = cones_[TO[t]].y - y0;
        float len = std::sqrt(dx * dx + dy * dy);
        int steps = (int)(len / step);
        for (int s = 0; s <= steps; ++s) {
            GPoint p;
            p.x = x0 + dx * s / steps;
            p.y = y0 + dy * s / steps;
            p.cone = -1;
            points_.push_back(p);
        }
    }
}

int Field::size() const {
    return points_.size();
}

int Field::coneCount() const {
    return cones_.size();
}

const GPoint& Field::point(int i) const {
    return points_[i];
}

// --- DistMatrix ---

DistMatrix::DistMatrix() : n_(0) {
}

void DistMatrix::build(const Field& field) {
    n_ = field.size();
    data_.assign((long long)n_ * (n_ - 1) / 2, 0.0f);
    long long k = 0;
    for (int i = 0; i < n_; ++i) {
        const GPoint& p = field.point(i);
        for (int j = i + 1; j < n_; ++j) {
            const GPoint& q = field.point(j);
            float dx = p.x - q.x;
            float dy = p.y - q.y;
            data_[k] = std::sqrt(dx * dx + dy * dy);
            ++k;
        }
    }
}

float DistMatrix::get(int i, int j) const {
    if (i == j) {
        return 0.0f;
    }
    if (i > j) {
        int t = i;
        i = j;
        j = t;
    }
    return data_[triIndex(i, j, n_)];
}

int DistMatrix::size() const {
    return n_;
}

double DistMatrix::megabytes() const {
    return data_.size() * 4.0 / 1024.0 / 1024.0;
}

// --- ByteMatrix ---

ByteMatrix::ByteMatrix() : n_(0) {
}

void ByteMatrix::resize(int n) {
    n_ = n;
    data_.assign((long long)n * n, 0);
}

void ByteMatrix::set(int i, int j) {
    data_[(long long)i * n_ + j] = 1;
    data_[(long long)j * n_ + i] = 1;
}

bool ByteMatrix::get(int i, int j) const {
    return data_[(long long)i * n_ + j] != 0;
}

int ByteMatrix::nextOne(int i, int from) const {
    for (int j = from; j < n_; ++j) {
        if (data_[(long long)i * n_ + j] != 0) {
            return j;
        }
    }
    return -1;
}

int ByteMatrix::size() const {
    return n_;
}

double ByteMatrix::megabytes() const {
    return data_.size() / 1024.0 / 1024.0;
}

// --- BitMatrix ---

BitMatrix::BitMatrix() : n_(0), rowBytes_(0) {
}

void BitMatrix::resize(int n) {
    n_ = n;
    rowBytes_ = (n + 7) / 8;
    data_.assign((long long)n * rowBytes_, 0);
}

void BitMatrix::set(int i, int j) {
    data_[(long long)i * rowBytes_ + j / 8] |= (unsigned char)(1 << (j % 8));
    data_[(long long)j * rowBytes_ + i / 8] |= (unsigned char)(1 << (i % 8));
}

bool BitMatrix::get(int i, int j) const {
    return (data_[(long long)i * rowBytes_ + j / 8] >> (j % 8)) & 1;
}

int BitMatrix::nextOne(int i, int from) const {
    long long row = (long long)i * rowBytes_;
    int byte = from / 8;
    int bit = from % 8;
    while (byte < rowBytes_) {
        unsigned char v = data_[row + byte];
        if (v != 0) { // пустые байты пропускаем целиком
            for (int k = bit; k < 8; ++k) {
                if ((v >> k) & 1) {
                    int j = byte * 8 + k;
                    return j < n_ ? j : -1;
                }
            }
        }
        ++byte;
        bit = 0;
    }
    return -1;
}

int BitMatrix::size() const {
    return n_;
}

double BitMatrix::megabytes() const {
    return data_.size() / 1024.0 / 1024.0;
}

// --- GraphBuilder ---

long long GraphBuilder::build(const DistMatrix& d, float threshold, BMatrix& b) const {
    int n = d.size();
    b.resize(n);
    long long edges = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (d.get(i, j) < threshold) {
                b.set(i, j);
                ++edges;
            }
        }
    }
    return edges;
}

long long GraphBuilder::build(const DistMatrix& d, float threshold, ByteMatrix& b) const {
    int n = d.size();
    b.resize(n);
    long long edges = 0;
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            if (d.get(i, j) < threshold) {
                b.set(i, j);
                ++edges;
            }
        }
    }
    return edges;
}

// --- Wave ---

std::list<Cluster> Wave::run(const BMatrix& b, const Field& field) {
    int n = b.size();
    a_.assign(n, 0); // a(i) = 0: волна до точки не дошла
    labels_.assign(n, -1);

    std::list<Cluster> clusters;
    std::vector<int> current; // горящие точки текущего шага
    std::vector<int> next;    // точки, которые загорятся на следующем

    for (int k = 0; k < n; ++k) {
        if (a_[k] != 0) {
            continue; // точка уже в каком-то кластере
        }
        // Поджигаем точку k: a(k) = 1, начинается новый кластер.
        Cluster cl;
        a_[k] = 1;
        current.clear();
        current.push_back(k);
        cl.points.push_back(k);
        int step = 1;

        while (!current.empty()) {
            next.clear();
            for (int i : current) {
                for (int j = b.nextOne(i, 0); j != -1; j = b.nextOne(i, j + 1)) {
                    if (a_[j] == 0) {
                        a_[j] = step + 1;
                        next.push_back(j);
                        cl.points.push_back(j);
                    }
                }
            }
            current = next;
            ++step;
        }

        float sumX = 0.0f;
        float sumY = 0.0f;
        for (int idx : cl.points) {
            sumX += field.point(idx).x;
            sumY += field.point(idx).y;
        }
        cl.cx = sumX / cl.points.size();
        cl.cy = sumY / cl.points.size();
        clusters.push_back(cl);
    }

    clusters.sort(biggerCluster);
    int number = 0;
    for (const Cluster& cl : clusters) {
        for (int idx : cl.points) {
            labels_[idx] = number;
        }
        ++number;
    }
    return clusters;
}

const std::vector<int>& Wave::labels() const {
    return labels_;
}

// --- DbscanFilter ---

long long DbscanFilter::apply(const BMatrix& b, int k, BMatrix& bStar) {
    int n = b.size();
    type_.assign(n, 0);

    // Базовые точки: степень вершины не меньше K.
    for (int i = 0; i < n; ++i) {
        int degree = 0;
        for (int j = b.nextOne(i, 0); j != -1 && degree < k; j = b.nextOne(i, j + 1)) {
            ++degree;
        }
        if (degree >= k) {
            type_[i] = 2;
        }
    }
    // Периферийные: сами не базовые, но смежны с базовой.
    for (int i = 0; i < n; ++i) {
        if (type_[i] == 2) {
            continue;
        }
        for (int j = b.nextOne(i, 0); j != -1; j = b.nextOne(i, j + 1)) {
            if (type_[j] == 2) {
                type_[i] = 1;
                break;
            }
        }
    }
    // B*: рёбра, у которых хотя бы один конец базовый.
    bStar.resize(n);
    long long edges = 0;
    for (int i = 0; i < n; ++i) {
        if (type_[i] != 2) {
            continue;
        }
        for (int j = b.nextOne(i, 0); j != -1; j = b.nextOne(i, j + 1)) {
            if (type_[j] == 1 || (type_[j] == 2 && j > i)) {
                bStar.set(i, j);
                ++edges;
            }
        }
    }
    return edges;
}

int DbscanFilter::type(int i) const {
    return type_[i];
}

int DbscanFilter::count(int type) const {
    int result = 0;
    for (int t : type_) {
        if (t == type) {
            ++result;
        }
    }
    return result;
}

// --- Plotter ---

// 20 хорошо различимых цветов в виде 0xRRGGBB.
const int COLORS[20] = {
    0xe6194b, 0x3cb44b, 0x4363d8, 0xf58231, 0x911eb4,
    0x42d4f4, 0xf032e6, 0x9a6324, 0x469990, 0x800000,
    0xbfef45, 0x000075, 0xffe119, 0xdcbeff, 0x808000,
    0xfabed4, 0xaaffc3, 0xffd8b1, 0x000000, 0x2f4f4f
};
const int GRAY = 0xcccccc;          // цвет пыли
const long long MAX_PLOT_EDGES = 300000; // больше рёбер gnuplot рисует долго

void Plotter::clusters(const Field& field, const std::list<Cluster>& clusters,
                       const BMatrix& b, int minCluster, const std::string& title,
                       const std::string& name) const {
    std::vector<int> color(field.size(), GRAY);
    int k = 0;
    for (const Cluster& cl : clusters) {
        if ((int)cl.points.size() >= minCluster) {
            for (int idx : cl.points) {
                color[idx] = COLORS[k % 20];
            }
        }
        ++k;
    }

    std::ofstream pts(name + "_points.txt");
    for (int i = 0; i < field.size(); ++i) {
        pts << field.point(i).x << " " << field.point(i).y << " " << color[i] << "\n";
    }

    // Рёбра берём из матрицы B: каждое - две строки и пустая строка.
    long long edges = 0;
    std::ofstream edg(name + "_edges.txt");
    for (int i = 0; i < b.size() && edges < MAX_PLOT_EDGES; ++i) {
        for (int j = b.nextOne(i, i + 1); j != -1; j = b.nextOne(i, j + 1)) {
            edg << field.point(i).x << " " << field.point(i).y << " " << color[i] << "\n";
            edg << field.point(j).x << " " << field.point(j).y << " " << color[i] << "\n\n";
            ++edges;
        }
    }
    edg << "0 0 0\n"; // чтобы файл не был пустым

    std::ofstream out("plot_" + name + ".gp");
    out << "reset\n";
    out << "set terminal png size 1000,1000\n";
    out << "set output '" << name << ".png'\n";
    out << "set title '" << title << "'\n";
    out << "set size square\n";
    out << "set xrange [" << FIELD_MIN << ":" << FIELD_MAX << "]\n";
    out << "set yrange [" << FIELD_MIN << ":" << FIELD_MAX << "]\n";
    out << "unset key\n";
    if (edges < MAX_PLOT_EDGES) {
        out << "plot '" << name << "_edges.txt' using 1:2:3 with lines lc rgb variable lw 0.5, \\\n";
        out << "     '" << name << "_points.txt' using 1:2:3 with points pt 7 ps 0.5 lc rgb variable\n";
    } else {
        out << "plot '" << name << "_points.txt' using 1:2:3 with points pt 7 ps 0.5 lc rgb variable\n";
    }
}

// --- Tester ---

void Tester::report(const std::string& name, bool ok) const {
    std::cout << "  " << (ok ? "OK     " : "ОШИБКА ") << name << std::endl;
}

void Tester::run(const Field& field, const DistMatrix& d, float threshold, int k) const {
    int n = field.size();
    std::cout << "Проверки на " << n << " точках:" << std::endl;

    // 1. D симметрична, на диагонали нули, значения совпадают с формулой.
    bool ok = true;
    for (int i = 0; i < n; i += 7) {
        int j = (i * 31 + 17) % n;
        float dx = field.point(i).x - field.point(j).x;
        float dy = field.point(i).y - field.point(j).y;
        if (d.get(i, j) != d.get(j, i) || d.get(i, i) != 0.0f ||
            std::fabs(d.get(i, j) - std::sqrt(dx * dx + dy * dy)) > 1e-4f) {
            ok = false;
        }
    }
    report("матрица D симметрична и считает расстояния верно", ok);

    // 2. Матрица в байтах и матрица в битах дают одно и то же.
    GraphBuilder builder;
    ByteMatrix bytes;
    BitMatrix bits;
    builder.build(d, threshold, bytes);
    builder.build(d, threshold, bits);
    ok = true;
    for (int i = 0; i < n && ok; ++i) {
        int a = bytes.nextOne(i, 0);
        int b = bits.nextOne(i, 0);
        while (a != -1 || b != -1) {
            if (a != b) {
                ok = false;
                break;
            }
            a = bytes.nextOne(i, a + 1);
            b = bits.nextOne(i, b + 1);
        }
    }
    report("B в байтах и B в битах совпадают", ok);
    std::cout << "         память B: байты " << bytes.megabytes() << " МБ, биты "
              << bits.megabytes() << " МБ" << std::endl;

    // 3. Волна раскладывает каждую точку ровно в один кластер.
    Wave wave;
    std::list<Cluster> clusters = wave.run(bits, field);
    std::vector<int> seen(n, 0);
    for (const Cluster& cl : clusters) {
        for (int idx : cl.points) {
            seen[idx]++;
        }
    }
    ok = true;
    for (int i = 0; i < n; ++i) {
        if (seen[i] != 1) {
            ok = false;
        }
    }
    report("волна: каждая точка ровно в одном кластере", ok);

    // 4. Точки одного кластера не соединены с точками другого.
    ok = true;
    const std::vector<int>& labels = wave.labels();
    for (int i = 0; i < n; ++i) {
        for (int j = bits.nextOne(i, 0); j != -1; j = bits.nextOne(i, j + 1)) {
            if (labels[i] != labels[j]) {
                ok = false;
            }
        }
    }
    report("волна: рёбра не выходят за кластер", ok);

    // 5. Фильтр DBSCAN раздал типы по определению.
    DbscanFilter filter;
    BitMatrix star;
    filter.apply(bits, k, star);
    ok = true;
    for (int i = 0; i < n; ++i) {
        int degree = 0;
        bool coreNeighbour = false;
        for (int j = bits.nextOne(i, 0); j != -1; j = bits.nextOne(i, j + 1)) {
            ++degree;
            if (filter.type(j) == 2) {
                coreNeighbour = true;
            }
        }
        int expected = degree >= k ? 2 : (coreNeighbour ? 1 : 0);
        if (filter.type(i) != expected) {
            ok = false;
        }
    }
    report("базовые, периферийные и шум - по определению", ok);

    // 6. В B* нет шума и нет рёбер между двумя небазовыми точками.
    ok = true;
    for (int i = 0; i < n; ++i) {
        for (int j = star.nextOne(i, 0); j != -1; j = star.nextOne(i, j + 1)) {
            if (filter.type(i) == 0 || filter.type(j) == 0 ||
                (filter.type(i) != 2 && filter.type(j) != 2)) {
                ok = false;
            }
        }
    }
    report("в B* только рёбра от базовых точек", ok);

    // 7. Каждый кластер после фильтра (кроме одиночного шума) содержит
    //    базовую точку.
    std::list<Cluster> filtered = wave.run(star, field);
    ok = true;
    for (const Cluster& cl : filtered) {
        bool hasCore = false;
        for (int idx : cl.points) {
            if (filter.type(idx) == 2) {
                hasCore = true;
            }
        }
        if (!hasCore && !(cl.points.size() == 1 && filter.type(cl.points[0]) == 0)) {
            ok = false;
        }
    }
    report("в каждом кластере B* есть базовая точка", ok);
}

// --- Control ---

Control::Control(const Config& config, const std::vector<Cone>& cones)
    : config_(config), hasData_(false) {
    field_.setCones(cones);
}

bool Control::execute(const std::string& line) {
    // Первое слово - команда, остальные - числа.
    std::istringstream in(line);
    std::string cmd;
    in >> cmd;
    std::vector<float> args;
    std::string word;
    bool badNumber = false;
    while (in >> word) {
        std::istringstream num(word);
        float value = 0.0f;
        if (num >> value) {
            args.push_back(value);
        } else {
            badNumber = true;
        }
    }

    if (cmd.empty()) {
        return true;
    }
    if (cmd == "exit") {
        return false;
    }
    if (cmd == "help") {
        help();
        return true;
    }
    if (cmd == "config") {
        config_.print();
        return true;
    }
    if (badNumber) {
        std::cout << "Ошибка: после команды должны идти числа." << std::endl;
        return true;
    }
    if (cmd == "gen") {
        int perCone = config_.getInt("n_gauss", 200);
        if (!args.empty()) {
            perCone = (int)args[0];
        }
        if (perCone < 10 || perCone > 10000) {
            std::cout << "Ошибка: точек на конус должно быть от 10 до 10000." << std::endl;
            return true;
        }
        generate(perCone);
        return true;
    }
    if (cmd == "bench") {
        bench();
        return true;
    }
    if (cmd == "wave" || cmd == "dbscan" || cmd == "test") {
        if (!hasData_) {
            std::cout << "Ошибка: сначала создайте точки командой gen." << std::endl;
            return true;
        }
    }
    if (cmd == "wave") {
        float t = config_.getFloat("wave_threshold", 0.4f);
        if (!args.empty()) {
            t = args[0];
        }
        if (t < 0.01f || t > 30.0f) {
            std::cout << "Ошибка: порог должен быть от 0.01 до 30." << std::endl;
            return true;
        }
        waveCommand(t);
        return true;
    }
    if (cmd == "dbscan") {
        std::vector<int> ks;
        for (float a : args) {
            ks.push_back((int)a);
        }
        if (ks.empty()) {
            ks.push_back(config_.getInt("dbscan_k", 4));
        }
        for (int k : ks) {
            if (k < 1 || k > 100) {
                std::cout << "Ошибка: K должно быть от 1 до 100." << std::endl;
                return true;
            }
        }
        dbscan(ks);
        return true;
    }
    if (cmd == "test") {
        tester_.run(field_, d_, config_.getFloat("wave_threshold", 0.4f),
                    config_.getInt("dbscan_k", 4));
        return true;
    }
    std::cout << "Неизвестная команда: " << cmd << ". Список команд: help" << std::endl;
    return true;
}

void Control::help() const {
    std::cout << "Команды:" << std::endl;
    std::cout << "  gen N           - создать N точек под каждым конусом и тропы" << std::endl;
    std::cout << "  wave T          - обычная волна по графу B (без T - wave_threshold)" << std::endl;
    std::cout << "  dbscan K1 K2 .. - фильтр DBSCAN при каждом K, потом волна по B*" << std::endl;
    std::cout << "  bench           - нагрузочный тест на 1000, 5000, 10000, 20000 точках" << std::endl;
    std::cout << "  test            - проверки классов" << std::endl;
    std::cout << "  config          - показать параметры" << std::endl;
    std::cout << "  help            - список команд" << std::endl;
    std::cout << "  exit            - выход" << std::endl;
}

void Control::generate(int perCone) {
    field_.generate(perCone, config_.getFloat("sigma", 1.0f), config_.getInt("seed", 212));
    if (config_.getInt("ant_trail", 1) == 1) {
        field_.addAntTrails(config_.getFloat("trail_step", 0.25f));
    }
    clock_t start = clock();
    d_.build(field_);
    double ms = 1000.0 * (clock() - start) / CLOCKS_PER_SEC;
    hasData_ = true;
    std::cout << "Конусов: " << field_.coneCount() << ", точек вместе с тропами: "
              << field_.size() << std::endl;
    std::cout << "Матрица D: " << d_.megabytes() << " МБ, посчитана за " << ms
              << " мс" << std::endl;
}

// Сколько кластеров не меньше minCluster точек.
int bigClusters(const std::list<Cluster>& clusters, int minCluster) {
    int big = 0;
    for (const Cluster& cl : clusters) {
        if ((int)cl.points.size() >= minCluster) {
            ++big;
        }
    }
    return big;
}

void Control::waveCommand(float threshold) {
    int minCluster = config_.getInt("min_cluster", 10);
    BMatrix b;
    long long edges = builder_.build(d_, threshold, b);
    std::list<Cluster> clusters = wave_.run(b, field_);
    int big = bigClusters(clusters, minCluster);
    std::cout << "Обычная волна по B, порог " << threshold << ": рёбер " << edges
              << ", кластеров " << big << ", пыли " << (int)clusters.size() - big
              << std::endl;
    std::ostringstream title;
    title << "Граф B и волна, порог " << threshold << ": кластеров " << big;
    plotter_.clusters(field_, clusters, b, minCluster, title.str(), "graph_B");
    std::cout << "Рисунок: gnuplot plot_graph_B.gp" << std::endl;
}

void Control::dbscan(const std::vector<int>& ks) {
    float threshold = config_.getFloat("wave_threshold", 0.4f);
    int minCluster = config_.getInt("min_cluster", 10);
    BMatrix b;
    builder_.build(d_, threshold, b);
    int plainBig = bigClusters(wave_.run(b, field_), minCluster);

    std::cout << "Порог " << threshold << ", обычная волна по B: кластеров " << plainBig
              << std::endl;
    std::cout << "K   базовых   периферийных   шум   рёбер в B*   кластеров" << std::endl;
    for (int k : ks) {
        BMatrix star;
        long long edges = filter_.apply(b, k, star);
        std::list<Cluster> clusters = wave_.run(star, field_);
        int big = bigClusters(clusters, minCluster);
        std::cout << k << "   " << filter_.count(2) << "   " << filter_.count(1) << "   "
                  << filter_.count(0) << "   " << edges << "   " << big << std::endl;

        std::ostringstream name;
        name << "graph_Bstar_K" << k;
        std::ostringstream title;
        title << "Граф B* после фильтра DBSCAN, K = " << k << ", порог " << threshold
              << ": кластеров " << big << ", серые - шум";
        plotter_.clusters(field_, clusters, star, minCluster, title.str(), name.str());
    }
    std::cout << "Рисунки: gnuplot plot_graph_Bstar_K*.gp" << std::endl;
}

void Control::bench() {
    const int TOTALS[4] = {1000, 5000, 10000, 20000};
    float threshold = config_.getFloat("wave_threshold", 0.4f);
    int k = config_.getInt("dbscan_k", 4);
    int minCluster = config_.getInt("min_cluster", 10);
    std::ofstream file("bench_dbscan.txt");
    file << "# точек D_мс B_мс фильтр_мс волна_мс кластеров_B кластеров_B*\n";
    std::cout << "Точек   D, мс   B, мс   фильтр, мс   волна, мс   кластеров: B / B*"
              << std::endl;
    for (int t = 0; t < 4; ++t) {
        field_.generate(TOTALS[t] / field_.coneCount(), config_.getFloat("sigma", 1.0f),
                        config_.getInt("seed", 212));
        if (config_.getInt("ant_trail", 1) == 1) {
            field_.addAntTrails(config_.getFloat("trail_step", 0.25f));
        }
        clock_t start = clock();
        d_.build(field_);
        double dMs = 1000.0 * (clock() - start) / CLOCKS_PER_SEC;

        BMatrix b;
        start = clock();
        builder_.build(d_, threshold, b);
        double bMs = 1000.0 * (clock() - start) / CLOCKS_PER_SEC;
        int plainBig = bigClusters(wave_.run(b, field_), minCluster);

        BMatrix star;
        start = clock();
        filter_.apply(b, k, star);
        double fMs = 1000.0 * (clock() - start) / CLOCKS_PER_SEC;

        start = clock();
        int big = bigClusters(wave_.run(star, field_), minCluster);
        double wMs = 1000.0 * (clock() - start) / CLOCKS_PER_SEC;

        std::cout << field_.size() << "   " << dMs << "   " << bMs << "   " << fMs << "   "
                  << wMs << "   " << plainBig << " / " << big << std::endl;
        file << field_.size() << " " << dMs << " " << bMs << " " << fMs << " " << wMs
             << " " << plainBig << " " << big << "\n";
    }
    std::cout << "Таблица сохранена в bench_dbscan.txt" << std::endl;
    generate(config_.getInt("n_gauss", 200));
}
