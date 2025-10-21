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

    private:

};
