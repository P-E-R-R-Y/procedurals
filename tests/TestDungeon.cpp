#include <gtest/gtest.h>
#include "TinyKeepDev.hpp"

TEST(GeometryConstraintMethod, A) {
    //std::vector<Rect> rects = generateRectangleInCircle(100, 100, 2, 0.5);
    // Résultat : rectangle aléatoire dans le cercle
    // centre (x, y), largeur w, hauteur h
    auto rects = TinyKeepDev::generateRects<100>(100, 0.5, std::make_pair(0.25f, 0.75f));
    //TinyKeepDev::relaxRectangles(rects, 100);
    for (auto &r: rects) {
        auto [x, y, w, h] = r;
        std::cout << "{" << x << ", " << y << ", " << w << ", " << h << "}," << std::endl;
    }
}
