/**
 * @file PoissonDisk.hpp
 * @author Perry Chouteau
 * @brief Poisson disk sampling — random points with guaranteed minimum spacing.
 */

#pragma once

#include "Type.hpp"   // Vector2f
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <random>
#include <vector>

class PoissonDisk {
public:
    /**
     * @brief Generate points evenly spaced by at least `r`, inside a `width` x `height` area.
     * @param width   Area width
     * @param height  Area height
     * @param r       Minimum distance between any two points
     * @param k       Attempts per active point before giving up (default 30)
     * @param seed    Seed for determinism
     */
    static std::vector<Vector2f> generate(float width, float height, float r,
                                          int k = 30, uint64_t seed = 42) {
        const float cellSize = r / std::sqrt(2.0f);
        const int gridW = int(width  / cellSize) + 1;
        const int gridH = int(height / cellSize) + 1;

        std::vector<int>      grid(gridW * gridH, -1);
        std::vector<Vector2f> points;
        std::vector<int>      active;

        std::mt19937 rng(seed);
        std::uniform_real_distribution<float> dx(0, width);
        std::uniform_real_distribution<float> dy(0, height);
        std::uniform_real_distribution<float> angle(0, 2 * M_PI);
        std::uniform_real_distribution<float> radius(r, 2 * r);

        auto insert = [&](const Vector2f& p) {
            points.push_back(p);
            int gx = int(p.x / cellSize), gy = int(p.y / cellSize);
            grid[gy * gridW + gx] = int(points.size()) - 1;
            active.push_back(int(points.size()) - 1);
        };

        insert({dx(rng), dy(rng)});

        while (!active.empty()) {
            int idx = active[rng() % active.size()];
            bool found = false;
            for (int i = 0; i < k; ++i) {
                float a = angle(rng), rad = radius(rng);
                Vector2f cand{ points[idx].x + rad * std::cos(a),
                               points[idx].y + rad * std::sin(a) };
                if (cand.x < 0 || cand.y < 0 || cand.x >= width || cand.y >= height) continue;
                if (isValid(cand, r, grid, cellSize, gridW, gridH, points)) {
                    insert(cand);
                    found = true;
                    break;
                }
            }
            if (!found)
                active.erase(std::remove(active.begin(), active.end(), idx), active.end());
        }
        return points;
    }

private:
    static bool isValid(const Vector2f& pt, float r,
                        const std::vector<int>& grid, float cellSize,
                        int gridW, int gridH,
                        const std::vector<Vector2f>& points) {
        int gx = std::min(int(pt.x / cellSize), gridW - 1);
        int gy = std::min(int(pt.y / cellSize), gridH - 1);

        for (int dy = -2; dy <= 2; ++dy) {
            for (int dx = -2; dx <= 2; ++dx) {
                int nx = gx + dx, ny = gy + dy;
                if (nx < 0 || ny < 0 || nx >= gridW || ny >= gridH) continue;
                int idx = grid[ny * gridW + nx];
                if (idx >= 0) {
                    float d = std::hypot(pt.x - points[idx].x, pt.y - points[idx].y);
                    if (d < r) return false;
                }
            }
        }
        return true;
    }
};