/**
 * @file TeenyKeepDev.hpp
 * @author Perry Chouteau (perry.chouteau@outlook.com)
 * @brief 
 * @date 2025-10-15
 */

#include "Deterministic.hpp"
#include <cstdint>
#include <vector>
#include <stdio.h>
#include "type.hpp"

class TinyKeepDev {

    public:

        /**
         * @brief 
         * 
         * @tparam nbRects 
         * @param radius 
         * @param maxRectRatio \in ]0,1]
         * @param radiusRatios \in ]0,2]
         * @param seed 
         * @return Rects
         */
        template<int nbRects>
        static std::vector<Vector4f> generateRects(float radius, float maxRectRatio = 0.5, std::pair<float, float> radiusRatios = {0.1f, 1.f} , uint64_t seed = 0) {
            std::vector<Vector4f> result;

            // 1. Ratio largeur/hauteur
            float denominator = 1 / maxRectRatio; //maxRectRatio = 0.5, inv.. = 2; 

            // 2. Taille (diagonale)
            float diagMin = radius * radiusRatios.first;
            float diagMax = radius * radiusRatios.second;

            uint64_t next = seed;

            for (int i = 0; i < nbRects; ++i) {

                // 1. Ratio largeur/hauteur
                float ratio = maxRectRatio + random0D(next) * (denominator - maxRectRatio);
                // 2. Taille (diagonale)
                float diagonal = diagMin + random0D(next) * (diagMax - diagMin);

                // 3. Largeur / Hauteur
                float w = diagonal * ratio / std::sqrt(1.0f + ratio * ratio);
                float h = diagonal / std::sqrt(1.0f + ratio * ratio);

                // 4. Rayon réduit pour que le rectangle reste dans le cercle
                float reducedRadius = radius - diagonal / 2.0f;

                // 5. Position aléatoire uniforme dans le disque
                float theta = random0D(next) * 2.0f * M_PI;
                float r = reducedRadius * std::sqrt(random0D(mix64(next)));

                float x = r * std::cos(theta);
                float y = r * std::sin(theta);

                // 6. Enregistrement du rectangle
                result.push_back({x, y, w, h});

                //next seed
                next = mix64(next);
            }
            return result;
        }

        
        // Helper for sign
        static inline float sgn(float x) { return (x > 0) - (x < 0); }

        static bool stepSeparation(std::vector<Vector4f>& rects, std::vector<Vector2f>& vel, float dt = 0.1f, float force = 0.05f, float damping = 0.9f) {
            bool moving = false;

            for (size_t i = 0; i < rects.size(); ++i) {
                float fx = 0, fy = 0;

                for (size_t j = 0; j < rects.size(); ++j) {
                    if (i == j) continue;

                    auto& a = rects[i];
                    auto& b = rects[j];

                    float dx = a.x - b.x;
                    float dy = a.y - b.y;
                    float overlapX = (a.w + b.w) * 0.5f - std::fabs(dx);
                    float overlapY = (a.h + b.h) * 0.5f - std::fabs(dy);

                    if (overlapX > 0 && overlapY > 0) {
                        // push proportionally to overlap (repulsion)
                        fx += sgn(dx) * overlapX * force;
                        fy += sgn(dy) * overlapY * force;
                    }
                }

                vel[i].x = (vel[i].x + fx) * damping;
                vel[i].y = (vel[i].y + fy) * damping;

                rects[i].x += vel[i].x * dt;
                rects[i].y += vel[i].y * dt;

                if (std::fabs(vel[i].x) > 0.001f || std::fabs(vel[i].y) > 0.001f)
                    moving = true;
            }

            return moving; // true = still moving
        }

    private:

};
