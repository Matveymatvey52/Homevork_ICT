// Группа 212, Корепин Матвей
// Реализация классов k-means: поле, индикатор, два способа поиска,
// сам алгоритм, рисунок, проверки и диспетчер Control.

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

int Field::size() const {
    return points_.size();
}

int Field::coneCount() const {
    return cones_.size();
}

const GPoint& Field::point(int i) const {
    return points_[i];
}


// Квадрат сетки gridN x gridN, в который попадает координата v.
int cellOf(float v, int gridN) {
    int s = (int)std::floor((v - FIELD_MIN) / (FIELD_MAX - FIELD_MIN) * gridN);
    if (s < 0) {
        s = 0;
    }
    if (s > gridN - 1) {
        s = gridN - 1;
    }
    return s;
}

// Номер ближайшего центра к точке (x, y).
int nearestCenter(float x, float y, const std::vector<Center>& centers) {
    int best = 0;
    float bestD = -1.0f;
    for (int c = 0; c < (int)centers.size(); ++c) {
        float dx = x - centers[c].x;
        float dy = y - centers[c].y;
        float dist = std::sqrt(dx * dx + dy * dy); // обычное евклидово
        if (bestD < 0.0f || dist < bestD) {
            bestD = dist;
            best = c;
        }
    }
    return best;
}

// --- Indicator ---

void Indicator::reset(int n) {
    label_.assign(n, -1);
}

bool Indicator::set(int i, int c) {
    if (label_[i] == c) {
        return false;
    }
    label_[i] = c;
    return true;
}

int Indicator::get(int i) const {
    return label_[i];
}

int Indicator::size() const {
    return label_.size();
}

// --- PointSearch ---

long long PointSearch::distanceCount() const {
    return distances_;
}

// --- ClassicSearch ---

const char* ClassicSearch::name() const {
    return "классический";
}

void ClassicSearch::prepare(const Field&) {
    distances_ = 0;
}

int ClassicSearch::assign(const Field& field, const std::vector<Center>& centers,
                          Indicator& ind) {
    int changed = 0;
    for (int i = 0; i < field.size(); ++i) {
        int c = nearestCenter(field.point(i).x, field.point(i).y, centers);
        distances_ += centers.size();
        if (ind.set(i, c)) {
            ++changed;
        }
    }
    return changed;
}

// --- GridSearch ---

GridSearch::GridSearch(int gridN) : gridN_(gridN) {
}

const char* GridSearch::name() const {
    return "квадратно-гнездовой";
}

void GridSearch::prepare(const Field& field) {
    distances_ = 0;
    cells_.assign(gridN_ * gridN_, std::vector<int>());
    outside_.clear();
    for (int i = 0; i < field.size(); ++i) {
        float x = field.point(i).x;
        float y = field.point(i).y;
        if (x < FIELD_MIN || x >= FIELD_MAX || y < FIELD_MIN || y >= FIELD_MAX) {
            outside_.push_back(i); // за краем поля - считаем отдельно
            continue;
        }
        cells_[cellOf(y, gridN_) * gridN_ + cellOf(x, gridN_)].push_back(i);
    }
}

int GridSearch::assign(const Field& field, const std::vector<Center>& centers,
                       Indicator& ind) {
    float side = (FIELD_MAX - FIELD_MIN) / gridN_;
    int nodes = gridN_ + 1;

    // Ближайший центр для каждого угла сетки.
    std::vector<int> corner(nodes * nodes);
    for (int r = 0; r < nodes; ++r) {
        for (int c = 0; c < nodes; ++c) {
            corner[r * nodes + c] = nearestCenter(FIELD_MIN + c * side,
                                                  FIELD_MIN + r * side, centers);
            distances_ += centers.size();
        }
    }

    int changed = 0;
    for (int r = 0; r < gridN_; ++r) {
        for (int c = 0; c < gridN_; ++c) {
            const std::vector<int>& cell = cells_[r * gridN_ + c];
            if (cell.empty()) {
                continue;
            }
            int a = corner[r * nodes + c];
            bool same = corner[r * nodes + c + 1] == a &&
                        corner[(r + 1) * nodes + c] == a &&
                        corner[(r + 1) * nodes + c + 1] == a;
            for (int i : cell) {
                int best = a;
                if (!same) { // квадрат на границе кластеров - считаем точку
                    best = nearestCenter(field.point(i).x, field.point(i).y, centers);
                    distances_ += centers.size();
                }
                if (ind.set(i, best)) {
                    ++changed;
                }
            }
        }
    }
    for (int i : outside_) {
        int best = nearestCenter(field.point(i).x, field.point(i).y, centers);
        distances_ += centers.size();
        if (ind.set(i, best)) {
            ++changed;
        }
    }
    return changed;
}

// --- KMeans ---

void KMeans::initCenters(const Field& field, int k) {
    // Первый центр - точка 0, каждый следующий - точка, самая далёкая
    // от уже выбранных центров. Так центры сразу попадают в разные облака.
    centers_.clear();
    Center first;
    first.x = field.point(0).x;
    first.y = field.point(0).y;
    centers_.push_back(first);
    while ((int)centers_.size() < k) {
        int far = 0;
        float farD = -1.0f;
        for (int i = 0; i < field.size(); ++i) {
            int c = nearestCenter(field.point(i).x, field.point(i).y, centers_);
            float dx = field.point(i).x - centers_[c].x;
            float dy = field.point(i).y - centers_[c].y;
            float dist = dx * dx + dy * dy;
            if (dist > farD) {
                farD = dist;
                far = i;
            }
        }
        Center next;
        next.x = field.point(far).x;
        next.y = field.point(far).y;
        centers_.push_back(next);
    }
}

void KMeans::updateCenters(const Field& field) {
    int k = centers_.size();
    std::vector<float> sumX(k, 0.0f);
    std::vector<float> sumY(k, 0.0f);
    std::vector<int> count(k, 0);
    for (int i = 0; i < field.size(); ++i) {
        int c = ind_.get(i);
        sumX[c] += field.point(i).x;
        sumY[c] += field.point(i).y;
        count[c]++;
    }
    for (int c = 0; c < k; ++c) {
        if (count[c] > 0) { // пустой кластер оставляет старый центр
            centers_[c].x = sumX[c] / count[c];
            centers_[c].y = sumY[c] / count[c];
        }
    }
}

int KMeans::run(const Field& field, int k, int maxIter, PointSearch& search) {
    initCenters(field, k);
    ind_.reset(field.size());
    search.prepare(field);
    int iter = 0;
    // Точки -> центры -> точки ..., пока точки меняют кластеры.
    while (iter < maxIter) {
        ++iter;
        int changed = search.assign(field, centers_, ind_);
        if (changed == 0) {
            break;
        }
        updateCenters(field);
    }
    return iter;
}

const std::vector<Center>& KMeans::centers() const {
    return centers_;
}

const Indicator& KMeans::indicator() const {
    return ind_;
}

// --- Plotter ---

const int COLORS[10] = {
    0xe6194b, 0x3cb44b, 0x4363d8, 0xf58231, 0x911eb4,
    0x42d4f4, 0xf032e6, 0x9a6324, 0x469990, 0x800000
};

void Plotter::clusters(const Field& field, const KMeans& km, const std::string& title,
                       const std::string& name) const {
    std::ofstream pts(name + "_points.txt");
    for (int i = 0; i < field.size(); ++i) {
        pts << field.point(i).x << " " << field.point(i).y << " "
            << COLORS[km.indicator().get(i) % 10] << "\n";
    }
    std::ofstream cen(name + "_centers.txt");
    for (const Center& c : km.centers()) {
        cen << c.x << " " << c.y << "\n";
    }

    std::ofstream out("plot_" + name + ".gp");
    out << "reset\n";
    out << "set terminal png size 1000,1000\n";
    out << "set output '" << name << ".png'\n";
    out << "set title '" << title << "'\n";
    out << "set size square\n";
    out << "set xrange [" << FIELD_MIN << ":" << FIELD_MAX << "]\n";
    out << "set yrange [" << FIELD_MIN << ":" << FIELD_MAX << "]\n";
    out << "unset key\n";
    out << "plot '" << name << "_points.txt' using 1:2:3 with points pt 7 ps 0.5 lc rgb variable, \\\n";
    out << "     '" << name << "_centers.txt' using 1:2 with points pt 3 ps 4 lw 3 lc rgb 'black'\n";
}

// --- Tester ---

void Tester::report(const std::string& name, bool ok) const {
    std::cout << "  " << (ok ? "OK     " : "ОШИБКА ") << name << std::endl;
}

void Tester::run(const Field& field, int k, int maxIter, int gridN) const {
    int n = field.size();
    std::cout << "Проверки на " << n << " точках:" << std::endl;

    KMeans a;
    KMeans b;
    ClassicSearch classic;
    GridSearch grid(gridN);
    a.run(field, k, maxIter, classic);
    b.run(field, k, maxIter, grid);

    // 1. Обе реализации раздали точкам одни и те же кластеры.
    bool ok = true;
    for (int i = 0; i < n; ++i) {
        if (a.indicator().get(i) != b.indicator().get(i)) {
            ok = false;
        }
    }
    report("классическая и квадратно-гнездовая реализации совпадают", ok);

    // 2. Каждая точка лежит в кластере ближайшего к ней центра.
    ok = true;
    for (int i = 0; i < n; ++i) {
        if (nearestCenter(field.point(i).x, field.point(i).y, b.centers()) !=
            b.indicator().get(i)) {
            ok = false;
        }
    }
    report("каждая точка - у ближайшего центра", ok);

    // 3. Каждый центр - среднее своих точек.
    ok = true;
    for (int c = 0; c < k; ++c) {
        float sx = 0.0f;
        float sy = 0.0f;
        int cnt = 0;
        for (int i = 0; i < n; ++i) {
            if (b.indicator().get(i) == c) {
                sx += field.point(i).x;
                sy += field.point(i).y;
                ++cnt;
            }
        }
        if (cnt > 0 && (std::fabs(sx / cnt - b.centers()[c].x) > 1e-3f ||
                        std::fabs(sy / cnt - b.centers()[c].y) > 1e-3f)) {
            ok = false;
        }
    }
    report("каждый центр - среднее своих точек", ok);

    // 4. Квадратно-гнездовой поиск считает расстояний меньше.
    report("квадратно-гнездовой поиск считает меньше расстояний",
           grid.distanceCount() < classic.distanceCount());
}

// --- Control ---

Control::Control(const Config& config, const std::vector<Cone>& cones)
    : config_(config), hasData_(false) {
    field_.setCones(cones);
}

bool Control::execute(const std::string& line) {
    std::istringstream in(line);
    std::string cmd;
    std::string arg;
    in >> cmd >> arg;

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
    if (cmd == "gen") {
        int perCone = config_.getInt("n_gauss", 200);
        if (!arg.empty()) {
            std::istringstream num(arg);
            if (!(num >> perCone)) {
                std::cout << "Ошибка: после gen нужно целое число." << std::endl;
                return true;
            }
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
    if (cmd == "kmeans" || cmd == "test") {
        if (!hasData_) {
            std::cout << "Ошибка: сначала создайте точки командой gen." << std::endl;
            return true;
        }
        if (cmd == "kmeans") {
            kmeans();
        } else {
            tester_.run(field_, config_.getInt("kmeans_k", 5),
                        config_.getInt("kmeans_max_iter", 100),
                        config_.getInt("kmeans_grid_n", 20));
        }
        return true;
    }
    std::cout << "Неизвестная команда: " << cmd << ". Список команд: help" << std::endl;
    return true;
}

void Control::help() const {
    std::cout << "Команды:" << std::endl;
    std::cout << "  gen N   - создать N точек под каждым конусом (без N - n_gauss)" << std::endl;
    std::cout << "  kmeans  - k-means двумя способами: время, итерации, рисунок" << std::endl;
    std::cout << "  bench   - нагрузочный тест на 1000, 5000, 10000, 20000 точках" << std::endl;
    std::cout << "  test    - проверки классов" << std::endl;
    std::cout << "  config  - показать параметры" << std::endl;
    std::cout << "  help    - список команд" << std::endl;
    std::cout << "  exit    - выход" << std::endl;
}

void Control::generate(int perCone) {
    field_.generate(perCone, config_.getFloat("sigma", 1.0f), config_.getInt("seed", 212));
    hasData_ = true;
    std::cout << "Конусов: " << field_.coneCount() << ", точек: " << field_.size()
              << std::endl;
}

void Control::kmeans() {
    int k = config_.getInt("kmeans_k", 5);
    int maxIter = config_.getInt("kmeans_max_iter", 100);
    ClassicSearch classic;
    GridSearch grid(config_.getInt("kmeans_grid_n", 20));
    PointSearch* searches[2] = {&classic, &grid};

    for (int s = 0; s < 2; ++s) {
        KMeans km;
        clock_t start = clock();
        int iter = km.run(field_, k, maxIter, *searches[s]);
        double ms = 1000.0 * (clock() - start) / CLOCKS_PER_SEC;
        std::cout << "k-means, поиск " << searches[s]->name() << ": k = " << k
                  << ", итераций " << iter << ", время " << ms << " мс, расстояний "
                  << searches[s]->distanceCount() << std::endl;
        if (s == 1) {
            for (int c = 0; c < k; ++c) {
                std::cout << "  центр " << c + 1 << ": (" << km.centers()[c].x << ", "
                          << km.centers()[c].y << ")" << std::endl;
            }
            std::ostringstream title;
            title << "k-means, k = " << k << ", точек " << field_.size()
                  << ", итераций " << iter;
            plotter_.clusters(field_, km, title.str(), "kmeans");
        }
    }
    std::cout << "Рисунок: gnuplot plot_kmeans.gp" << std::endl;
}

void Control::bench() {
    const int TOTALS[4] = {1000, 5000, 10000, 20000};
    int k = config_.getInt("kmeans_k", 5);
    int maxIter = config_.getInt("kmeans_max_iter", 100);
    std::ofstream file("bench_kmeans.txt");
    file << "# точек итераций классический_мс квадратный_мс расст_классич расст_квадрат\n";
    std::cout << "Точек  итераций  классический, мс  квадратный, мс  расстояний: класс. / квадр."
              << std::endl;
    for (int t = 0; t < 4; ++t) {
        field_.generate(TOTALS[t] / field_.coneCount(), config_.getFloat("sigma", 1.0f),
                        config_.getInt("seed", 212));
        ClassicSearch classic;
        GridSearch grid(config_.getInt("kmeans_grid_n", 20));
        KMeans a;
        KMeans b;
        // Повторяем 20 раз: один прогон идёт быстрее, чем тикают часы.
        const int REPEAT = 20;
        int iter = 0;
        clock_t start = clock();
        for (int r = 0; r < REPEAT; ++r) {
            iter = a.run(field_, k, maxIter, classic);
        }
        double classicMs = 1000.0 * (clock() - start) / CLOCKS_PER_SEC / REPEAT;
        start = clock();
        for (int r = 0; r < REPEAT; ++r) {
            b.run(field_, k, maxIter, grid);
        }
        double gridMs = 1000.0 * (clock() - start) / CLOCKS_PER_SEC / REPEAT;
        std::cout << field_.size() << "  " << iter << "  " << classicMs << "  " << gridMs
                  << "  " << classic.distanceCount() << " / " << grid.distanceCount()
                  << std::endl;
        file << field_.size() << " " << iter << " " << classicMs << " " << gridMs << " "
             << classic.distanceCount() << " " << grid.distanceCount() << "\n";
    }
    std::cout << "Таблица сохранена в bench_kmeans.txt" << std::endl;
    generate(config_.getInt("n_gauss", 200));
}
