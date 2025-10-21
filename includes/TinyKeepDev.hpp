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
                next = mix64(next);
                // 2. Taille (diagonale)
                float diagonal = diagMin + random0D(next) * (diagMax - diagMin);
                next = mix64(next);

                // 3. Largeur / Hauteur
                float w = diagonal * ratio / std::sqrt(1.0f + ratio * ratio);
                float h = diagonal / std::sqrt(1.0f + ratio * ratio);

                // 4. Rayon réduit pour que le rectangle reste dans le cercle
                float reducedRadius = radius - diagonal / 2.0f;

                // 5. Position aléatoire uniforme dans le disque
                float theta = random0D(next) * 2.0f * M_PI;
                next = mix64(next);
                float r = reducedRadius * std::sqrt(random0D(next));
                next = mix64(next);

                float x = r * std::cos(theta);
                float y = r * std::sin(theta);

                // Convertir en coin supérieur gauche
                //x -= w * 0.5f;
                //y -= h * 0.5f;

                // 6. Enregistrement du rectangle
                result.push_back({x, y, w, h});

                //next seed
            }
            return result;
        }

        template<int its = 50>
        static std::vector<Vector4f> relaxRects(std::vector<Vector4f> rects) {

            for (int i = 0; i < its; ++i) {
                for (size_t i = 0; i < rects.size(); ++i) {
                    for (size_t j = i + 1; j < rects.size(); ++j) {

                        float dx = rects[j].x - rects[i].x;
                        float dy = rects[j].y - rects[i].y;
                        float distSq = dx*dx + dy*dy;

                        if (isCollidingOrClose(rects[i], rects[j])) {
                            float dx = rects[j].x - rects[i].x;
                            float dy = rects[j].y - rects[i].y;
                            float dist = sqrtf(dx*dx + dy*dy) + 0.001f; // éviter division par 0
                            float nx = dx / dist;
                            float ny = dy / dist;

                            float overlapX = (rects[i].w + rects[j].w)/2 - fabs(dx);
                            float overlapY = (rects[i].h + rects[j].h)/2 - fabs(dy);
                            float overlap = fminf(overlapX, overlapY);

                            float attractionRange = 50.0f; // distance max pour attraction
                            if (overlap > 0) {
                                // Repulsion
                                rects[i].x -= nx * overlap * 0.5f;
                                rects[i].y -= ny * overlap * 0.5f;
                                rects[j].x += nx * overlap * 0.5f;
                                rects[j].y += ny * overlap * 0.5f;
                            } else if (dist < attractionRange) {
                                // Attraction proportionnelle à la distance
                                float attractionStrength = (attractionRange - dist) * 0.05f; // ajustable
                                rects[i].x += nx * attractionStrength;
                                rects[i].y += ny * attractionStrength;
                                rects[j].x -= nx * attractionStrength;
                                rects[j].y -= ny * attractionStrength;
                            }
                        }
                    }
                }
            }
        }

    private:

        static bool isCollidingOrClose(const Vector4f &a, const Vector4f &b, float margin = 0.0f) {
            return !(a.x + a.w/2 + margin < b.x - b.w/2 ||
                    a.x - a.w/2 - margin > b.x + b.w/2 ||
                    a.y + a.h/2 + margin < b.y - b.h/2 ||
                    a.y - a.h/2 - margin > b.y + b.h/2);
        }


};
