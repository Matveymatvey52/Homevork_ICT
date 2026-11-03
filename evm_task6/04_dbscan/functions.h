// Группа 212, Корепин Матвей
// DBSCAN как фильтр графа: D -> B -> фильтр по K -> B* -> волна.
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
    // Муравьиные тропы: цепочки точек с шагом step между облаками
    // 1 и 3 и между облаками 4 и 5. Такие точки получают cone = -1.
    void addAntTrails(float step);

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

// ByteMatrix: двоичная матрица B, один байт на клетку.
// С неё начинали. Другими классами не пользуется.
class ByteMatrix {
public:
    ByteMatrix();
    void resize(int n);
    void set(int i, int j);            // ставит 1 в клетки (i,j) и (j,i)
    bool get(int i, int j) const;
    int nextOne(int i, int from) const; // следующая единица в строке i, -1 - нет
    int size() const;
    double megabytes() const;

private:
    int n_;
    std::vector<unsigned char> data_;
};

// BitMatrix: та же матрица B, но один бит на клетку - в 8 раз меньше
// памяти. Методы те же, что у ByteMatrix. Другими классами не пользуется.
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

// Какая матрица B используется в программе. Перешли с байтов на биты,
// поменяв только эту строку: остальной код не изменился.
typedef BitMatrix BMatrix;

// GraphBuilder: строит двоичную матрицу B по матрице D и порогу:
// единица, если расстояние меньше порога. Пользуется: DistMatrix, BMatrix.
class GraphBuilder {
public:
    // Возвращает число рёбер.
    long long build(const DistMatrix& d, float threshold, BMatrix& b) const;
    long long build(const DistMatrix& d, float threshold, ByteMatrix& b) const;
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

// DbscanFilter: фильтр DBSCAN. Степень вершины в B - число соседей.
// Базовая точка - у неё не меньше K соседей. Периферийная - соседей
// меньше K, но среди них есть базовая. Остальные точки - шум, в B* их нет.
// В B* оставляем рёбра, у которых хотя бы один конец базовый: так две
// периферийные точки соседних облаков не склеят облака в одно.
// Пользуется: BMatrix.
class DbscanFilter {
public:
    // Строит bStar по b. Возвращает число рёбер в B*.
    long long apply(const BMatrix& b, int k, BMatrix& bStar);
    int type(int i) const; // 2 - базовая, 1 - периферийная, 0 - шум
    int count(int type) const;

private:
    std::vector<int> type_;
};

// Plotter: пишет файлы данных и скрипты gnuplot. Ничего не считает.
// Пользуется: Field, Cluster, Edge, BMatrix.
class Plotter {
public:
    void clusters(const Field& field, const std::list<Cluster>& clusters,
                  const BMatrix& b, int minCluster, const std::string& title,
                  const std::string& name) const;
};

// Tester: проверки, что классы считают правильно. Печатает OK или ОШИБКА.
// Пользуется: Field, DistMatrix, ByteMatrix, BitMatrix, GraphBuilder,
// Wave, DbscanFilter.
class Tester {
public:
    void run(const Field& field, const DistMatrix& d, float threshold, int k) const;

private:
    void report(const std::string& name, bool ok) const;
};

// Control: диспетчер. Получает команду от интерфейса, проверяет её и
// раздаёт поручения остальным классам. Сам ничего не считает.
// Пользуется: Config, Field, DistMatrix, GraphBuilder, Wave,
// DbscanFilter, Plotter, Tester.
class Control {
public:
    Control(const Config& config, const std::vector<Cone>& cones);
    // Возвращает false, когда пришла команда exit.
    bool execute(const std::string& line);

private:
    void help() const;
    void generate(int perCone);
    void waveCommand(float threshold);
    void dbscan(const std::vector<int>& ks);
    void bench();

    Config config_;
    Field field_;
    DistMatrix d_;
    GraphBuilder builder_;
    Wave wave_;
    DbscanFilter filter_;
    Plotter plotter_;
    Tester tester_;
    bool hasData_;
};

#endif // FUNCTIONS_H
