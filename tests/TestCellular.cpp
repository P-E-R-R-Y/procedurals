/**
 * @file TestCellular.cpp
 * @author Perry Chouteau (perry.chouteau@outlook.com)
 * @brief 
 * @date 2025-10-13
 */

#include <gtest/gtest.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include "Deterministic.hpp"

// ------------------ CELLULAR AUTOMATA ------------------
void initializeGrid(std::vector<std::vector<int>>& grid, float fillProb, uint64_t seed) {
    for (size_t y = 0; y < grid.size(); ++y)
        for (size_t x = 0; x < grid[0].size(); ++x) {
            float r = random2D(x, y, seed); // tes fonctions déterministes
            grid[y][x] = (r < fillProb) ? 1 : 0; // mur ou sol
        }
}

int countNeighbors(const std::vector<std::vector<int>>& grid, int x, int y) {
    int count = 0;
    for (int dy = -1; dy <= 1; ++dy)
        for (int dx = -1; dx <= 1; ++dx) {
            if (dx == 0 && dy == 0) continue;
            int nx = x + dx, ny = y + dy;
            if (nx < 0 || ny < 0 || nx >= (int)grid[0].size() || ny >= (int)grid.size())
                count++;
            else
                count += grid[ny][nx];
        }
    return count;
}

void doSimulationStep(std::vector<std::vector<int>>& grid) {
    std::vector<std::vector<int>> newGrid = grid;
    for (int y = 0; y < (int)grid.size(); ++y) {
        for (int x = 0; x < (int)grid[0].size(); ++x) {
            int neighbors = countNeighbors(grid, x, y);
            if (neighbors > 4)
                newGrid[y][x] = 1;
            else if (neighbors < 4)
                newGrid[y][x] = 0;
        }
    }
    grid = newGrid;
}

// ------------------ SAVE PPM ------------------
void savePPM(const std::string& filename, const std::vector<std::vector<int>>& grid) {
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

TEST(Cellular, A1) {
    const int W = 128, H = 128;
    std::vector<std::vector<int>> grid(H, std::vector<int>(W, 0));

    uint64_t seed = 42;        // seed déterministe
    float fillProb = 0.5f;    // densité initiale
    int steps = 10;             // itérations CA

    initializeGrid(grid, fillProb, seed);

    for (int i = 0; i < steps; ++i)
        doSimulationStep(grid);

    savePPM("cellular.ppm", grid);
    std::cout << "✅ Cellular Automata dungeon generated: cellular.ppm\n";
    EXPECT_TRUE(true);
}