/**
 * @file Deterministic.hpp
 * @author Perry Chouteau (perry.chouteau@outlook.com)
 * @brief 
 * @date 2025-10-12
 */

#include <cstdint>

#pragma once

/**
 * @brief phi, inv_phi, golden_ratio
 */
constexpr double PHI = 1.6180339887498949; //golden number (1.0 + 5.0_sqrt) / 2.0;
constexpr double INV_PHI = 1.0 / PHI; //fraction inverse
constexpr uint64_t UINT64_MAX_CONST = ~0ULL; // tous les bits à 1 = 2^64 - 1
constexpr double DOUBLE_MAX = static_cast<double>(UINT64_MAX_CONST); // 18446744073709551615.0 + 1.0
constexpr uint64_t GOLDEN_64 = static_cast<uint64_t>(INV_PHI * DOUBLE_MAX);

/**
 * @brief Magic Multipliers
 */
constexpr uint64_t MAGIC1 = GOLDEN_64 ^ 0xAAAAAAAAAAAAAAAAULL; // 1010…
constexpr uint64_t MAGIC2 = GOLDEN_64 ^ 0x5555555555555555ULL; // 0101…
constexpr uint64_t MAGIC3 = GOLDEN_64 ^ 0x3333333333333333ULL; // 0011…
constexpr uint64_t MAGIC4 = GOLDEN_64 ^ 0xCCCCCCCCCCCCCCCCULL; // 1100…

//inline uint64_t mix64(uint64_t x) noexcept {
//    x += GOLDEN_64; //golden ratio for 64bits
//    x = (x ^ (x >> 30)) * MAGIC1; //scramble small modification -> big change 
//    x = (x ^ (x >> 27)) * MAGIC2; //again
//    x ^= (x >> 31); //
//    return x;
//}

inline uint64_t mix64(uint64_t x) noexcept {
    // Advance with GOLDEN_64 (safe, no FP math)
    x += GOLDEN_64;
    // Scramble safely with XOR and multiply by GOLDEN_64 again
    x ^= (x >> 30);
    x *= GOLDEN_64;   // multiplication is modulo 2^64, safe
    x ^= (x >> 27);
    x *= GOLDEN_64;
    x ^= (x >> 31);
    return x;
}

inline float normaliseHash(uint64_t x) noexcept {
    constexpr double denom = 1.0 / static_cast<double>(UINT64_MAX);
    return static_cast<float>(static_cast<double>(x) * denom);
}

inline float random0D(uint64_t seed = 0) noexcept {
    uint64_t h = seed * GOLDEN_64;
    return normaliseHash(mix64(h));
}

inline float random1D(int x, uint64_t seed = 0) noexcept {
    uint64_t h = static_cast<uint64_t>(x) * MAGIC1 + seed;
    return normaliseHash(mix64(h));
}

inline float random2D(int x, int y, uint64_t seed = 0) noexcept {
    uint64_t h = static_cast<uint64_t>(x) * MAGIC1
               + static_cast<uint64_t>(y) * MAGIC2
               + seed;
    return normaliseHash(mix64(h));
}

inline float random3D(int x, int y, int z, uint64_t seed = 0) noexcept {
    uint64_t h = static_cast<uint64_t>(x) * MAGIC1
               + static_cast<uint64_t>(y) * MAGIC2
               + static_cast<uint64_t>(z) * MAGIC3
               + seed;
    return normaliseHash(mix64(h));
}

inline float random4D(int x, int y, int z, int w, uint64_t seed = 0) noexcept {
    uint64_t h = static_cast<uint64_t>(x) * MAGIC1
               + static_cast<uint64_t>(y) * MAGIC2
               + static_cast<uint64_t>(z) * MAGIC3
               + static_cast<uint64_t>(w) * MAGIC4
               + seed;
    return normaliseHash(mix64(h));
}

inline uint64_t SplitSeed(uint64_t seed, uint64_t channel) noexcept {
    // simple dérivation : ajout du channel * MAGIC1 pour bien mélanger les bits
    return mix64(seed + channel * MAGIC1);
}