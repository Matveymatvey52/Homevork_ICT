// Группа 212, Корепин Матвей
// Конусы на плоскости: сетка квадратов, 2D-гаусс под конусами,
// кластеризация волной и минимальным покрывающим деревом.

#include <algorithm>
#include <iostream>
#include <list>
#include <random>
#include <vector>
#include "functions.h"

const int POINTS_PER_CONE = 1000; // точек гаусса под каждым конусом
// Порог для рёбер дерева, подобран по гистограмме. При 0.3 и выше
// сливаются облака конусов (3, 3) и (5, 5): они касаются хвостами.
const float TREE_THRESHOLD = 0.25f;
const int MIN_CLUSTER = 10;       // кластеры меньше - это отдельные выбросы

// Печатает крупные кластеры и считает мелкие.
void printClusters(const std::vector<Cluster>& clusters) {
    int big = 0;
    int small = 0;
    for (const Cluster& cl : clusters) {
        if ((int)cl.points.size() < MIN_CLUSTER) {
            ++small;
            continue;
        }
        ++big;
        std::cout << "  Кластер " << big << ": точек " << cl.points.size()
                  << ", центр масс (" << cl.cx << ", " << cl.cy << ")"
                  << ", x от " << cl.minX << " до " << cl.maxX
                  << ", y от " << cl.minY << " до " << cl.maxY << std::endl;
        std::cout << "    номера точек: ";
        for (int i = 0; i < (int)cl.points.size() && i < 8; ++i) {
            std::cout << cl.points[i] << " ";
        }
        std::cout << "..." << std::endl;
    }
    std::cout << "  Крупных кластеров: " << big
              << ", выбросов (меньше " << MIN_CLUSTER << " точек): " << small
              << std::endl;
}

int main() {
    std::list<Cone> cones; // двунаправленный список из STL
    if (!loadConesFromFile("cones.txt", cones)) {
        std::cerr << "Программа завершена из-за ошибки чтения файла." << std::endl;
        return -1;
    }

    Field field(-10.0f, 10.0f, -10.0f, 10.0f);

    // 1. Координаты квадратов конусов и сортировка по ним.
    cones.sort(coneLess); // сортировка списка по нашему правилу
    std::cout << "Конусы, упорядоченные по квадратам сетки "
              << GRID_N << "x" << GRID_N << " (сторона квадрата "
              << SQUARE << "):" << std::endl;
    int index = 1;
    for (const Cone& cone : cones) {
        std::cout << "  Конус " << index << ": (" << cone.getX() << ", "
                  << cone.getY() << "), R=" << cone.getRadius()
                  << ", h=" << cone.getHeight()
                  << ", квадрат (" << squareX(cone.getX()) << ", "
                  << squareY(cone.getY()) << ")" << std::endl;
        ++index;
    }

    // 2. 2D-гаусс под каждым конусом.
    std::mt19937 gen(212); // фиксированное зерно - результат повторяется
    std::vector<GPoint> points =
        generateGaussPoints(cones, POINTS_PER_CONE, gen);
    std::sort(points.begin(), points.end(), pointLess);
    std::cout << std::endl << "Сгенерировано вторичных точек: "
              << points.size() << std::endl;
    std::cout << "Первые точки после сортировки по квадратам:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "  (" << points[i].x << ", " << points[i].y
                  << ") квадрат (" << points[i].s1 << ", " << points[i].s2
                  << ")" << std::endl;
    }

    std::vector<int> coneLabels(points.size());
    for (int i = 0; i < (int)points.size(); ++i) {
        coneLabels[i] = points[i].cone;
    }
    writePointsFile(points, coneLabels, "gauss_points.txt");
    writePointsScript("plot_gauss.gp", "gauss_points.txt", "gauss.png",
                      "2D-гаусс под конусами", field);

    // 3. Волна по двоичной матрице.
    float waveThreshold = SQUARE;
    std::vector<char> matrix = buildIncidenceMatrix(points, waveThreshold);
    std::vector<Cluster> wave = waveClusters(points, matrix);
    std::cout << std::endl << "Алгоритм \"Волна\" (порог " << waveThreshold
              << "), всего компонент: " << wave.size() << std::endl;
    printClusters(wave);
    writePointsFile(points, clusterLabels(wave, points.size()), "wave_points.txt");
    writePointsScript("plot_wave.gp", "wave_points.txt", "wave.png",
                      "Кластеры, найденные волной", field);

    // 4. Минимальное покрывающее дерево.
    std::vector<Edge> tree = buildSpanningTree(points);
    std::vector<int> hist = edgeHistogram(tree, 50, 2.0f);
    writeHistogram(hist, 2.0f, TREE_THRESHOLD, "tree_hist.txt",
                   "plot_hist.gp", "tree_hist.png");
    std::vector<Cluster> treeCl = treeClusters(points, tree, TREE_THRESHOLD);
    std::cout << std::endl << "Минимальное покрывающее дерево: рёбер "
              << tree.size() << ", порог " << TREE_THRESHOLD
              << ", всего кусков: " << treeCl.size() << std::endl;
    printClusters(treeCl);
    writePointsFile(points, clusterLabels(treeCl, points.size()), "tree_points.txt");
    writePointsScript("plot_tree.gp", "tree_points.txt", "tree.png",
                      "Кластеры, найденные деревом", field);

    std::cout << std::endl << "Картинки: gnuplot plot_gauss.gp plot_wave.gp "
              << "plot_hist.gp plot_tree.gp" << std::endl;
    return 0;
}
