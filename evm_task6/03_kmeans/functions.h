// Группа 212, Корепин Матвей
// Алгоритм k-means в двух реализациях: классической и квадратно-гнездовой.
// Расстояние в обеих обычное евклидово. Способ поиска ближайшего центра
// спрятан в классе: у обеих реализаций общий интерфейс PointSearch.

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

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
    int cone;
};

// Центр кластера.
struct Center {
    float x;
    float y;
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

// Field: поле с конусами и вторичными точками. Создаёт и хранит точки.
// Пользуется: Cone, GPoint.
class Field {
public:
    void setCones(const std::vector<Cone>& cones);
    void generate(int perCone, float sigma, int seed);
    int size() const;
    int coneCount() const;
    const GPoint& point(int i) const;

private:
    std::vector<Cone> cones_;
    std::vector<GPoint> points_;
};

// Indicator: индикаторная переменная - номер кластера каждой точки.
// Отвечает только за хранение номеров и подсчёт, сколько точек
// сменили кластер. Другими классами не пользуется.
class Indicator {
public:
    void reset(int n);
    // Ставит точке i кластер c. Возвращает true, если номер поменялся.
    bool set(int i, int c);
    int get(int i) const;
    int size() const;

private:
    std::vector<int> label_;
};

// PointSearch: общий интерфейс поиска ближайшего центра.
// Наследники по-разному раздают точкам номера ближайших центров.
class PointSearch {
public:
    virtual ~PointSearch() {}
    virtual const char* name() const = 0;
    // Подготовка к работе с этим набором точек.
    virtual void prepare(const Field& field) = 0;
    // Каждой точке - номер ближайшего центра. Возвращает, сколько
    // точек сменили кластер.
    virtual int assign(const Field& field, const std::vector<Center>& centers,
                       Indicator& ind) = 0;
    // Сколько раз считали расстояние от точки до центра.
    PointSearch() : distances_(0) {}
    long long distanceCount() const;

protected:
    long long distances_;
};

// ClassicSearch: для каждой точки перебирает все центры.
// Пользуется: Field, Center, Indicator.
class ClassicSearch : public PointSearch {
public:
    const char* name() const override;
    void prepare(const Field& field) override;
    int assign(const Field& field, const std::vector<Center>& centers,
               Indicator& ind) override;
};

// GridSearch: квадратно-гнездовой поиск. Поле размечено сеткой.
// Если у всех четырёх углов квадрата ближайший центр один и тот же,
// то и у всех точек внутри квадрата он тот же, и расстояния для них
// не считаем. Точки считаем по одной только в пограничных квадратах.
// Пользуется: Field, Center, Indicator.
class GridSearch : public PointSearch {
public:
    explicit GridSearch(int gridN);
    const char* name() const override;
    void prepare(const Field& field) override;
    int assign(const Field& field, const std::vector<Center>& centers,
               Indicator& ind) override;

private:
    int gridN_;
    std::vector<std::vector<int>> cells_; // номера точек в каждом квадрате
    std::vector<int> outside_;            // точки за краем поля
};

// KMeans: сам алгоритм. Выбирает начальные центры, по очереди
// раздаёт точкам центры и пересчитывает центры как средние.
// Пользуется: Field, PointSearch, Indicator, Center.
class KMeans {
public:
    // Возвращает число итераций.
    int run(const Field& field, int k, int maxIter, PointSearch& search);
    const std::vector<Center>& centers() const;
    const Indicator& indicator() const;

private:
    void initCenters(const Field& field, int k);
    void updateCenters(const Field& field);

    std::vector<Center> centers_;
    Indicator ind_;
};

// Plotter: пишет файлы данных и скрипт gnuplot. Ничего не считает.
// Пользуется: Field, KMeans.
class Plotter {
public:
    void clusters(const Field& field, const KMeans& km, const std::string& title,
                  const std::string& name) const;
};

// Tester: проверки, что классы считают правильно.
// Пользуется: Field, KMeans, ClassicSearch, GridSearch.
class Tester {
public:
    void run(const Field& field, int k, int maxIter, int gridN) const;

private:
    void report(const std::string& name, bool ok) const;
};

// Control: диспетчер. Получает команду от интерфейса, проверяет её и
// раздаёт поручения. Сам ничего не считает.
// Пользуется: Config, Field, KMeans, ClassicSearch, GridSearch, Plotter, Tester.
class Control {
public:
    Control(const Config& config, const std::vector<Cone>& cones);
    bool execute(const std::string& line);

private:
    void help() const;
    void generate(int perCone);
    void kmeans();
    void bench();

    Config config_;
    Field field_;
    Plotter plotter_;
    Tester tester_;
    bool hasData_;
};

#endif // FUNCTIONS_H
