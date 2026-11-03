// Группа 212, Корепин Матвей
// ФОРЭЛ (формальный элемент): два уровня и волна по центрам тяжести.
// Классы программы и за что отвечает каждый. Реализация - в functions.cpp.

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include <list>
#include <map>
#include <string>
#include <vector>

const float FIELD_MIN = -10.0f; // поле от -10 до 10 по x и по y
const float FIELD_MAX = 10.0f;

// Конус из файла: центр, радиус основания и высота.
struct Cone {
    float x;
    float y;
    float r;
    float h;
};

// Вторичная точка (из гаусса под конусом).
struct GPoint {
    float x;
    float y;
    int cone; // номер конуса, под которым точка родилась
};

// Ребро: номера двух точек и длина.
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

// Config: читает файл параметров "имя = значение" и отдаёт значения
// по имени. Другими классами не пользуется.
class Config {
public:
    bool load(const std::string& filename);
    int getInt(const std::string& key, int def) const;
    float getFloat(const std::string& key, float def) const;
    std::string getString(const std::string& key, const std::string& def) const;
    void print() const;

private:
    std::map<std::string, std::string> values_;
};

// Читает конусы: первая строка - количество, дальше "x y R h".
bool loadCones(const std::string& filename, std::vector<Cone>& cones);

// Field: поле с конусами и вторичными точками. Отвечает только за то,
// чтобы создать точки и хранить их. Пользуется: Cone, GPoint.
class Field {
public:
    void setCones(const std::vector<Cone>& cones);
    // perCone точек 2D-гаусса под каждым конусом. sigma - для самого
    // низкого конуса, у высоких конусов σ меньше.
    void generate(int perCone, float sigma, int seed);
    // Вместо точек из гаусса - готовый набор (например, центры тяжести).
    void setPoints(const std::vector<GPoint>& points);

    int size() const;
    int coneCount() const;
    const GPoint& point(int i) const;

private:
    std::vector<Cone> cones_;
    std::vector<GPoint> points_;
};

// DistMatrix (матрица D): расстояния между всеми парами точек.
// Считается один раз, дальше её берут и граф B, и дерево.
// Матрица симметричная, поэтому храним только часть над диагональю.
// Пользуется: Field.
class DistMatrix {
public:
    DistMatrix();
    void build(const Field& field);
    float get(int i, int j) const;
    int size() const;
    double megabytes() const;

private:
    int n_;
    std::vector<float> data_;
};

// BitMatrix: двоичная матрица B, один бит на клетку.
// Другими классами не пользуется.
class BitMatrix {
public:
    BitMatrix();
    void resize(int n);
    void set(int i, int j);
    bool get(int i, int j) const;
    int nextOne(int i, int from) const;
    int size() const;
    double megabytes() const;

private:
    int n_;
    int rowBytes_; // байтов в одной строке
    std::vector<unsigned char> data_;
};

// Какая матрица B используется в программе.
typedef BitMatrix BMatrix;

// GraphBuilder: строит двоичную матрицу B по матрице D и порогу:
// единица, если расстояние меньше порога. Пользуется: DistMatrix, BMatrix.
class GraphBuilder {
public:
    // Возвращает число рёбер.
    long long build(const DistMatrix& d, float threshold, BMatrix& b) const;
};

// Wave: волна («степной пожар») находит связные компоненты графа B.
// Свои поля держит внутри: a(i) - шаг, на котором волна дошла до точки,
// и номер кластера каждой точки. Пользуется: BMatrix, Field, Cluster.
class Wave {
public:
    // Кластеры идут от большего к меньшему.
    std::list<Cluster> run(const BMatrix& b, const Field& field);
    const std::vector<int>& labels() const; // номер кластера каждой точки

private:
    std::vector<int> a_;
    std::vector<int> labels_;
};

// Forel: один уровень формального элемента. Берёт первую несобранную
// точку как центр, собирает точки в круге радиуса R, переносит центр
// в их центр тяжести и повторяет, пока центр сдвигается больше чем на ε.
// Группу вырезает и начинает со следующей точки. Хранит центры тяжести,
// их веса (сколько исходных точек за центром) и номер группы каждой точки.
// Пользуется: Field, GPoint.
class Forel {
public:
    // weights - вес каждой точки (на первом уровне все по 1).
    void run(const Field& field, const std::vector<float>& weights, float r, float eps);
    const std::vector<GPoint>& centers() const;
    const std::vector<float>& weights() const;
    const std::vector<int>& labels() const; // номер группы каждой точки
    int moves() const;                       // сколько раз переносили центр

private:
    std::vector<GPoint> centers_;
    std::vector<float> weights_;
    std::vector<int> labels_;
    int moves_;
};

// Plotter: пишет файлы данных и скрипты gnuplot. Ничего не считает.
// Пользуется: Field, Forel.
class Plotter {
public:
    // Точки, раскрашенные по группам, центры и круги радиуса r.
    void groups(const Field& field, const std::vector<int>& labels,
                const std::vector<GPoint>& centers, float r, const std::string& title,
                const std::string& name) const;
};

// Tester: проверки, что классы считают правильно.
// Пользуется: Field, Forel.
class Tester {
public:
    void run(const Field& field, float r, float eps, float level2) const;

private:
    void report(const std::string& name, bool ok) const;
};

// Control: диспетчер. Получает команду от интерфейса, проверяет её и
// раздаёт поручения. Сам ничего не считает.
// Пользуется: Config, Field, Forel, DistMatrix, GraphBuilder, Wave,
// Plotter, Tester.
class Control {
public:
    Control(const Config& config, const std::vector<Cone>& cones);
    bool execute(const std::string& line);

private:
    void help() const;
    void generate(int perCone);
    void forel(const std::vector<float>& radii);
    void bench();

    Config config_;
    Field field_;
    Plotter plotter_;
    Tester tester_;
    bool hasData_;
};

#endif // FUNCTIONS_H
