/**
 * @file Noise.hpp
 * @author Perry Chouteau (perry.chouteau@outlook.com)
 * @brief 
 * @date 2025-10-12
 */

#pragma once

#include <functional>
#include <fstream>
#include <string>
#include <cmath>
#include <cstdint>
#include <algorithm>

inline void visualizer2D(
    const std::string& filename,
    int width,
    int height,
    float scale,
    const std::function<float(float, float)>& noiseFunc)
{
    std::ofstream out(filename, std::ios::binary);
    if (!out) return;

    out << "P6\n" << width << " " << height << "\n255\n";

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            float nx = (x - width / 2.0f) / scale;
            float ny = (y - height / 2.0f) / scale;
            float n = noiseFunc(nx, ny);

            // Normalize from [-1, 1] → [0, 255]
            unsigned char v = static_cast<unsigned char>(
                std::clamp((n * 0.5f + 0.5f) * 255.0f, 0.0f, 255.0f)
            );

            out.put(v).put(v).put(v); // grayscale RGB
        }
    }

    out.close();
}

inline void visualizer2D(const std::string& filename, const std::vector<std::vector<float>>& map) {
    int H = map.size(), W = map[0].size();
    std::ofstream out(filename, std::ios::binary);
    out << "P6\n" << W << " " << H << "\n255\n";
    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x) {
            unsigned char c = static_cast<unsigned char>(
                std::clamp(map[y][x] * 255.0f, 0.0f, 255.0f)
            );

            out.put(c).put(c).put(c);
        }
    out.close();
}