/**
 * @file Noise.hpp
 * @author Perry Chouteau (perry.chouteau@outlook.com)
 * @brief 
 * @date 2025-10-12
 */

#pragma once

#include "Smoothing.hpp"
#include <cmath>

/**
 * @brief 2D deterministic value noise in [-1, 1].
 *
 * Computes smooth pseudo-random noise from (x, y) and an optional seed.
 * Values at the four surrounding grid points are hashed and smoothly
 * interpolated using an ease-in-out curve.
 *
 * @param x X coordinate (can be fractional)
 * @param y Y coordinate (can be fractional)
 * @param seed Optional seed for reproducibility
 * @return float Noise value in [-1, 1]
 */
float noise2D(float x, float y, uint64_t seed = 0) noexcept {
    int xi = static_cast<int>(std::floor(x));
    int yi = static_cast<int>(std::floor(y));
    float tx = x - xi;
    float ty = y - yi;

    float v00 = random2D(xi, yi, seed);
    float v10 = random2D(xi + 1, yi, seed);
    float v01 = random2D(xi, yi + 1, seed);
    float v11 = random2D(xi + 1, yi + 1, seed);

    float a = InterpolateEaseInOut(v00, v10, tx);
    float b = InterpolateEaseInOut(v01, v11, tx);
    return InterpolateEaseInOut(a, b, ty);
}

/**
 * @brief Fractal Brownian Motion (fBm)
 * 
 * Adds multiple octaves of noise to create fractal-like complexity.
 * Each octave increases frequency and decreases amplitude.
 * 
 */
inline float fbm2D(float x, float y, int octaves = 5, uint64_t seed = 0, float gain = 0.5f, float lacunarity = 2.0f) noexcept {
    float value = 0.0f;
    float amplitude = 1.0f;
    float frequency = 1.0f;

    for (int i = 0; i < octaves; ++i) {
        value += amplitude * noise2D(x * frequency, y * frequency, seed + i * 1337); //1337 is used to increase the seed diff between each octaves
        frequency *= lacunarity;
        amplitude *= gain;
    }

    return value; // range ~[-1, 1]
}

inline float warp2D(float x, float y, uint64_t seed = 0, float strength = 2.0f) noexcept {
    float warpX = noise2D(x * 0.5f, y * 0.5f, seed + 11);
    float warpY = noise2D(x * 0.5f + 100.0f, y * 0.5f + 100.0f, seed + 23);
    return noise2D(x + warpX * strength, y + warpY * strength, seed + 99);
}

/**
 * @brief Ridged Fractal Noise (inverted fBm)
 * 
 * Turns smooth valleys into sharp ridges and peaks.
 * Useful for mountain heightmaps or cracked stone surfaces.
 */
inline float ridge2D(float x, float y, int octaves = 5, float gain = 0.5f, float lacunarity = 2.0f, uint64_t seed = 0) noexcept {
    auto ridge = [](float n) { return 1.0f - std::fabs(n); };

    float result = 0.0f;
    float amplitude = 0.5f;
    float frequency = 1.0f;

    for (int i = 0; i < octaves; ++i) {
        float n = ridge(noise2D(x * frequency, y * frequency, seed + i * 1000));
        result += n * amplitude;
        frequency *= lacunarity;
        amplitude *= gain;
    }

    return result;
}

inline float rarity2D(float x, float y, int nbVal, float rarity = 0.01f, uint64_t seed = 0) {
    float r = random2D(x, y, seed); // decide si l’objet existe
    if (r > rarity) return 0.0f;    // 0 = rien

    // Choisir une valeur entre 0 et nbVal
    int val = 1 + (int)(random2D(x, y, seed + 999) * nbVal);
    if (val > nbVal) val = nbVal;

    return static_cast<float>(val); // float pour visualiser, peut être casté en int
}

inline float worley2D(float x, float y, uint64_t seed = 0) noexcept {
    int xi = (int)floor(x);
    int yi = (int)floor(y);
    float minDist = 9999.0f;

    // On parcourt les cellules voisines pour trouver le point le plus proche
    for (int j = -1; j <= 1; ++j) {
        for (int i = -1; i <= 1; ++i) {
            int cx = xi + i;
            int cy = yi + j;

            // point aléatoire dans la cellule
            float fx = random2D(cx, cy, seed);
            float fy = random2D(cx, cy, seed + 1337);

            // position absolue du point
            float px = cx + fx;
            float py = cy + fy;

            float dx = px - x;
            float dy = py - y;
            float dist = sqrt(dx*dx + dy*dy);

            if (dist < minDist)
                minDist = dist;
        }
    }

    return minDist; // souvent dans [0,1] après normalisation
}

#include <cmath>
#include <cstdint>
#include <random>

// -------------------- Simplex 2D --------------------
inline float grad(int hash, float x, float y) {
    int h = hash & 7; // 8 directions
    float u = h < 4 ? x : y;
    float v = h < 4 ? y : x;
    return ((h & 1) ? -u : u) + ((h & 2) ? -2.0f*v : 2.0f*v);
}

// initPerm using deterministic RNG
inline void initPerm(uint8_t* perm, uint64_t seed) {
    // Fill 0..255
    for(int i=0;i<256;i++) perm[i]=i;

    // Shuffle using deterministic random2D or mix64
    for(int i=255;i>0;i--){
        // use a procedural hash as deterministic "random" index
        uint64_t h = mix64(seed + i * GOLDEN_64);  // mix64 is your procedural hash RNG
        int j = int(h % (i+1));       // map to 0..i
        std::swap(perm[i], perm[j]);
    }

    // Duplicate for wrap-around
    for(int i=0;i<256;i++) perm[i+256]=perm[i];
}

inline float simplex2D(float xin, float yin, uint64_t seed = 0) {
    static uint8_t perm[512];
    static bool initialized=false;
    if(!initialized){ initPerm(perm, seed); initialized=true; }

    const float F2 = 0.5f*(sqrt(3.0f)-1.0f);
    float s = (xin + yin) * F2;
    int i = floor(xin + s);
    int j = floor(yin + s);
    const float G2 = (3.0f - sqrt(3.0f))/6.0f;
    float t = (i + j) * G2;
    float X0 = i - t, Y0 = j - t;
    float x0 = xin - X0, y0 = yin - Y0;

    int i1 = (x0 > y0) ? 1 : 0;
    int j1 = (x0 > y0) ? 0 : 1;

    float x1 = x0 - i1 + G2;
    float y1 = y0 - j1 + G2;
    float x2 = x0 - 1.0f + 2.0f * G2;
    float y2 = y0 - 1.0f + 2.0f * G2;

    int ii = i & 255;
    int jj = j & 255;

    float n0,n1,n2;

    float t0 = 0.5f - x0*x0 - y0*y0;
    n0 = (t0<0)?0:pow(t0,4) * grad(perm[ii+perm[jj]], x0, y0);

    float t1 = 0.5f - x1*x1 - y1*y1;
    n1 = (t1<0)?0:pow(t1,4) * grad(perm[ii+i1+perm[jj+j1]], x1, y1);

    float t2 = 0.5f - x2*x2 - y2*y2;
    n2 = (t2<0)?0:pow(t2,4) * grad(perm[ii+1+perm[jj+1]], x2, y2);

    return (70.0f * (n0 + n1 + n2)) * 0.5 + 0.5; // valeur brute [0,1] approximative
}

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

