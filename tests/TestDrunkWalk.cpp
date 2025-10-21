#include <gtest/gtest.h>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cstdint>
#include "Deterministic.hpp"

// Map: 1 = mur, 0 = sol
using Map = std::vector<std::vector<int>>;

// Direction 4-way
struct Dir { int dx, dy; };
const Dir dirs[4] = { {1,0}, {-1,0}, {0,1}, {0,-1} };

// Drunkard Walk
void drunkardWalk(Map& map, int startX, int startY, int steps, uint64_t seed = 0) {
    int x = startX;
    int y = startY;
    map[y][x] = 0; // creuser départ

    for (int i = 0; i < steps; ++i) {
        float r = random2D(x, y, seed + i * GOLDEN_64);
        int dirIndex = static_cast<int>(r * 4.0f) % 4;

        x += dirs[dirIndex].dx;
        y += dirs[dirIndex].dy;

        // Bordures
        x = std::clamp(x, 1, (int)map[0].size() - 2);
        y = std::clamp(y, 1, (int)map.size() - 2);

        map[y][x] = 0; // creuser
    }
}

// Sauvegarde PPM
inline void savePPM(const std::string& filename, const Map& grid) {
    int H = grid.size(), W = grid[0].size();
    std::ofstream out(filename, std::ios::binary);
    out << "P6\n" << W << " " << H << "\n255\n";
    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x) {
            unsigned char c = grid[y][x] ? 255 : 0; // blanc = mur, noir = sol
            out.put(c).put(c).put(c);
        }
    out.close();
}

// ------------------ Test GTest ------------------
TEST(DrunkardWalk, BasicDungeon) {
    const int W = 128, H = 128;
    Map map(H, std::vector<int>(W, 1)); // tout mur

    uint64_t seed = 0;
    drunkardWalk(map, W/2, H/2, 80000, 1);  // 2000 pas à partir du centre

    savePPM("drunkard_test.ppm", map);
    std::cout << "✅ Drunkard Walk dungeon saved: drunkard_test.ppm\n";

    // Test basique pour vérifier que des cases ont été creusées
    int emptyCount = 0;
    for (auto& row : map)
        for (auto cell : row)
            if (cell == 0) emptyCount++;
    EXPECT_GT(emptyCount, 0); // Il doit y avoir au moins un sol creusé
}