#include <gtest/gtest.h>
#include "WaveFunctionCollapse.hpp"
#include "Visualizer.hpp"

TEST(WaveFunctionCollapse, GenerateMap) {
    WaveFunctionCollapse::Rules rules{{
        {0, 1},
        {0, 1, 2},
        {1, 2, 3},
        {2, 3}
    }};

    constexpr int W = 512, H = 512;
    auto grid = WaveFunctionCollapse::collapse(W, H, rules, /*seed=*/0);

    ASSERT_EQ((int)grid.size(), H);
    ASSERT_EQ((int)grid[0].size(), W);

    std::vector<std::vector<float>> map(H, std::vector<float>(W));
    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x)
            map[y][x] = float(grid[y][x]) / float(rules.tileCount() - 1);

    visualizer2D("wave_func_collapse.ppm", map);

    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x) {
            EXPECT_GE(grid[y][x], 0);
            EXPECT_LE(grid[y][x], rules.tileCount() - 1);
        }
}