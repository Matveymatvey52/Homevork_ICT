// Группа 212, Корепин Матвей
// Реализация классов: поле, формальный элемент, матрицы D и B для волны
// по центрам, рисунки, проверки и диспетчер Control.

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

void Field::setPoints(const std::vector<GPoint>& points) {
    points_ = points;
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

// --- Forel ---

void Forel::run(const Field& field, const std::vector<float>& weights, float r,
                float eps) {
    int n = field.size();
    centers_.clear();
    weights_.clear();
    labels_.assign(n, -1);
    moves_ = 0;
    std::vector<int> group;

    int start = 0;
    while (start < n) {
        if (labels_[start] != -1) {
            ++start; // точка уже вырезана
            continue;
        }
        // Если центр уйдёт и точка start не попадёт в группу, на
        // следующем круге начнём с неё же: точек стало меньше.
        // 1. Первая несобранная точка - начальный центр.
        float cx = field.point(start).x;
        float cy = field.point(start).y;
        for (int iter = 0; iter < 100; ++iter) {
            // 2. Собираем несобранные точки в круге радиуса R.
            group.clear();
            float sumX = 0.0f;
            float sumY = 0.0f;
            float sumW = 0.0f;
            for (int i = 0; i < n; ++i) {
                if (labels_[i] != -1) {
                    continue;
                }
                float dx = field.point(i).x - cx;
                float dy = field.point(i).y - cy;
                if (dx * dx + dy * dy <= r * r) {
                    group.push_back(i);
                    sumX += weights[i] * field.point(i).x;
                    sumY += weights[i] * field.point(i).y;
                    sumW += weights[i];
                }
            }
            if (group.empty()) {
                group.push_back(start); // центр ушёл от всех точек - берём одну
                break;
            }
            // 3. Переносим центр в центр тяжести группы.
            float nx = sumX / sumW;
            float ny = sumY / sumW;
            float shift = std::sqrt((nx - cx) * (nx - cx) + (ny - cy) * (ny - cy));
            cx = nx;
            cy = ny;
            ++moves_;
            if (shift < eps) {
                break; // центр остановился на вершине сгустка
            }
        }
        // 4. Вырезаем группу, центр тяжести запоминаем.
        GPoint center;
        center.x = cx;
        center.y = cy;
        center.cone = -1;
        float w = 0.0f;
        for (int i : group) {
            labels_[i] = centers_.size();
            w += weights[i];
        }
        centers_.push_back(center);
        weights_.push_back(w);
    }
}

const std::vector<GPoint>& Forel::centers() const {
    return centers_;
}

const std::vector<float>& Forel::weights() const {
    return weights_;
}

const std::vector<int>& Forel::labels() const {
    return labels_;
}

int Forel::moves() const {
    return moves_;
}

// --- Plotter ---

const int COLORS[20] = {
    0xe6194b, 0x3cb44b, 0x4363d8, 0xf58231, 0x911eb4,
    0x42d4f4, 0xf032e6, 0x9a6324, 0x469990, 0x800000,
    0xbfef45, 0x000075, 0xffe119, 0xdcbeff, 0x808000,
    0xfabed4, 0xaaffc3, 0xffd8b1, 0x000000, 0x2f4f4f
};

void Plotter::groups(const Field& field, const std::vector<int>& labels,
                     const std::vector<GPoint>& centers, float r, const std::string& title,
                     const std::string& name) const {
    std::ofstream pts(name + "_points.txt");
    for (int i = 0; i < field.size(); ++i) {
        pts << field.point(i).x << " " << field.point(i).y << " "
            << COLORS[labels[i] % 20] << "\n";
    }
    std::ofstream cen(name + "_centers.txt");
    for (const GPoint& c : centers) {
        cen << c.x << " " << c.y << " " << r << "\n";
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
    out << "     '" << name << "_centers.txt' using 1:2:3 with circles lc rgb '#555555' lw 1, \\\n";
    out << "     '" << name << "_centers.txt' using 1:2 with points pt 3 ps 2 lw 2 lc rgb 'black'\n";
}

// --- Tester ---

void Tester::report(const std::string& name, bool ok) const {
    std::cout << "  " << (ok ? "OK     " : "ОШИБКА ") << name << std::endl;
}

void Tester::run(const Field& field, float r, float eps, float level2) const {
    int n = field.size();
    std::cout << "Проверки на " << n << " точках:" << std::endl;
    std::vector<float> ones(n, 1.0f);
    Forel first;
    first.run(field, ones, r, eps);

    // 1. Каждая точка попала ровно в одну группу.
    bool ok = true;
    for (int i = 0; i < n; ++i) {
        if (first.labels()[i] < 0 || first.labels()[i] >= (int)first.centers().size()) {
            ok = false;
        }
    }
    report("каждая точка в одной группе", ok);

    // 2. Точки группы лежат не дальше R (+ε) от её центра.
    ok = true;
    for (int i = 0; i < n; ++i) {
        const GPoint& c = first.centers()[first.labels()[i]];
        float dx = field.point(i).x - c.x;
        float dy = field.point(i).y - c.y;
        if (std::sqrt(dx * dx + dy * dy) > r + eps + 1e-4f) {
            ok = false;
        }
    }
    report("точки группы внутри круга радиуса R", ok);

    // 3. Веса центров в сумме дают число точек.
    float total = 0.0f;
    for (float w : first.weights()) {
        total += w;
    }
    report("сумма весов центров = числу точек", std::fabs(total - n) < 0.5f);

    // 4. Второй уровень: центров меньше, вес сохраняется.
    Field level1;
    level1.setPoints(first.centers());
    Forel second;
    second.run(level1, first.weights(), r * level2, eps);
    total = 0.0f;
    for (float w : second.weights()) {
        total += w;
    }
    report("второй уровень: центров меньше, вес тот же",
           second.centers().size() < first.centers().size() && std::fabs(total - n) < 0.5f);
}

// --- Control ---

Control::Control(const Config& config, const std::vector<Cone>& cones)
    : config_(config), hasData_(false) {
    field_.setCones(cones);
}

bool Control::execute(const std::string& line) {
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
    if (cmd == "forel" || cmd == "test") {
        if (!hasData_) {
            std::cout << "Ошибка: сначала создайте точки командой gen." << std::endl;
            return true;
        }
    }
    if (cmd == "forel") {
        if (args.empty()) {
            args.push_back(config_.getFloat("forel_r", 1.0f));
        }
        for (float r : args) {
            if (r < 0.1f || r > 10.0f) {
                std::cout << "Ошибка: R должен быть от 0.1 до 10." << std::endl;
                return true;
            }
        }
        forel(args);
        return true;
    }
    if (cmd == "test") {
        tester_.run(field_, config_.getFloat("forel_r", 1.0f),
                    config_.getFloat("forel_eps", 0.001f),
                    config_.getFloat("forel_level2", 2.5f));
        return true;
    }
    std::cout << "Неизвестная команда: " << cmd << ". Список команд: help" << std::endl;
    return true;
}

void Control::help() const {
    std::cout << "Команды:" << std::endl;
    std::cout << "  gen N          - создать N точек под каждым конусом (без N - n_gauss)" << std::endl;
    std::cout << "  forel R1 R2 .. - ФОРЭЛ при каждом R: два уровня и волна по центрам" << std::endl;
    std::cout << "  bench          - нагрузочный тест на 1000, 5000, 10000, 20000 точках" << std::endl;
    std::cout << "  test           - проверки классов" << std::endl;
    std::cout << "  config         - показать параметры" << std::endl;
    std::cout << "  help           - список команд" << std::endl;
    std::cout << "  exit           - выход" << std::endl;
}

void Control::generate(int perCone) {
    field_.generate(perCone, config_.getFloat("sigma", 1.0f), config_.getInt("seed", 212));
    hasData_ = true;
    std::cout << "Конусов: " << field_.coneCount() << ", точек: " << field_.size()
              << std::endl;
}

void Control::forel(const std::vector<float>& radii) {
    float eps = config_.getFloat("forel_eps", 0.001f);
    float level2 = config_.getFloat("forel_level2", 2.5f);
    float waveK = config_.getFloat("forel_wave_k", 1.5f);
    int minCluster = config_.getInt("min_cluster", 10);
    int n = field_.size();
    std::vector<float> ones(n, 1.0f);

    std::cout << "R     центров 1-го уровня   центров 2-го уровня   кластеров волной   время, мс"
              << std::endl;
    for (int k = 0; k < (int)radii.size(); ++k) {
        float r = radii[k];
        clock_t start = clock();
        // Первый уровень - по исходным точкам.
        Forel first;
        first.run(field_, ones, r, eps);
        // Второй уровень - по центрам тяжести, радиус больше.
        Field level1;
        level1.setPoints(first.centers());
        Forel second;
        second.run(level1, first.weights(), r * level2, eps);
        double ms = 1000.0 * (clock() - start) / CLOCKS_PER_SEC;

        // Волна по центрам первого уровня: точек в разы меньше.
        DistMatrix d;
        d.build(level1);
        BMatrix b;
        GraphBuilder builder;
        builder.build(d, r * waveK, b);
        Wave wave;
        std::list<Cluster> clusters = wave.run(b, level1);

        // Кластер крупный, если за его центрами не меньше minCluster точек.
        std::vector<int> pointLabel(n);
        int big = 0;
        for (const Cluster& cl : clusters) {
            float w = 0.0f;
            for (int idx : cl.points) {
                w += first.weights()[idx];
            }
            if (w >= minCluster) {
                ++big;
            }
        }
        for (int i = 0; i < n; ++i) {
            pointLabel[i] = wave.labels()[first.labels()[i]];
        }

        std::cout << r << "     " << first.centers().size() << "     "
                  << second.centers().size() << "     " << big << "     " << ms << std::endl;

        std::ostringstream suffix;
        suffix << "_" << k + 1;
        std::ostringstream t1;
        t1 << "ФОРЭЛ, 1-й уровень: R = " << r << ", центров " << first.centers().size();
        plotter_.groups(field_, first.labels(), first.centers(), r, t1.str(),
                        "forel_level1" + suffix.str());
        std::ostringstream t2;
        t2 << "ФОРЭЛ, 2-й уровень: R = " << r * level2 << ", центров "
           << second.centers().size();
        plotter_.groups(level1, second.labels(), second.centers(), r * level2, t2.str(),
                        "forel_level2" + suffix.str());
        std::ostringstream t3;
        t3 << "Волна по центрам 1-го уровня, порог " << r * waveK << ": кластеров " << big;
        plotter_.groups(field_, pointLabel, std::vector<GPoint>(), r, t3.str(),
                        "forel_wave" + suffix.str());
    }
    std::cout << "Рисунки: gnuplot plot_forel_*.gp" << std::endl;
}

void Control::bench() {
    const int TOTALS[4] = {1000, 5000, 10000, 20000};
    float r = config_.getFloat("forel_r", 1.0f);
    float eps = config_.getFloat("forel_eps", 0.001f);
    float level2 = config_.getFloat("forel_level2", 2.5f);
    std::ofstream file("bench_forel.txt");
    file << "# точек R центров1 центров2 время1_мс время2_мс\n";
    std::cout << "Точек   R   центров 1-го ур.   центров 2-го ур.   время 1-го, мс   время 2-го, мс"
              << std::endl;
    for (int t = 0; t < 4; ++t) {
        field_.generate(TOTALS[t] / field_.coneCount(), config_.getFloat("sigma", 1.0f),
                        config_.getInt("seed", 212));
        std::vector<float> ones(field_.size(), 1.0f);
        Forel first;
        clock_t start = clock();
        first.run(field_, ones, r, eps);
        double ms1 = 1000.0 * (clock() - start) / CLOCKS_PER_SEC;
        Field level1;
        level1.setPoints(first.centers());
        Forel second;
        start = clock();
        second.run(level1, first.weights(), r * level2, eps);
        double ms2 = 1000.0 * (clock() - start) / CLOCKS_PER_SEC;
        std::cout << field_.size() << "   " << r << "   " << first.centers().size() << "   "
                  << second.centers().size() << "   " << ms1 << "   " << ms2 << std::endl;
        file << field_.size() << " " << r << " " << first.centers().size() << " "
             << second.centers().size() << " " << ms1 << " " << ms2 << "\n";
    }
    std::cout << "Таблица сохранена в bench_forel.txt" << std::endl;
    generate(config_.getInt("n_gauss", 200));
}
