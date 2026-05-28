#include <gtest/gtest.h>
#include <cmath>
#include "PoissonDiskSampling.hpp"
#include "Visualizer.hpp"

TEST(PoissonDiskSampling, Visualize) {
    float width = 512, height = 512;
    float r = 10.f;

    auto points = PoissonDisk::generate(width, height, r, 10, 123);

    std::cout << "Generated " << points.size() << " points\n";
    std::vector<std::vector<float>> grid(height, std::vector<float>(width, 0.0f));
    for (auto& p : points) {
        int x = int(p.x), y = int(p.y);
        if (x >= 0 && x < width && y >= 0 && y < height) grid[y][x] = 1.0f;
    }
    visualizer2D("poisson_disk.ppm", grid);

    for (size_t i = 0; i < points.size(); ++i)
        for (size_t j = i + 1; j < points.size(); ++j) {
            float d = std::hypot(points[i].x - points[j].x, points[i].y - points[j].y);
            EXPECT_GE(d, r);
        }
}