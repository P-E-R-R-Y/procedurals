/**
 * @file Lissage.hpp
 * @author Perry Chouteau (perry.chouteau@outlook.com)
 * @brief 
 * @date 2025-10-12
 */

#include "Deterministic.hpp"

inline float Lerp(float a, float b, float t) noexcept {
    return a + t * (b - a);
}

inline float Clamp(float x, float minVal, float maxVal) noexcept {
    if (x < minVal) return minVal;
    if (x > maxVal) return maxVal;
    return x;
}

inline float EaseIn(float t) noexcept {
    t = Clamp(t, 0.0f, 1.0f);
    return t * t;
}
inline float EaseOut(float t) noexcept {
    t = Clamp(t, 0.0f, 1.0f);
    return 2.0 * t - t * t;
}

inline float EaseInOut(float t) noexcept {
    return EaseIn(EaseOut(t));
}

inline float InterpolateEaseInOut(float a, float b, float t) noexcept {
    float s = EaseInOut(t);   // easing
    return Lerp(a, b, s);
}

template<int Levels = 10>
inline float Quantize(float v) {
    constexpr float step = 1.0f / (Levels);
    int bucket = (int)(v / step);
    return bucket * step;
}

inline bool RareEvent(float v, int rarityLevel = 100) {
    int bucket = (int)(v * rarityLevel);
    return bucket == 0; // 1% chance
}