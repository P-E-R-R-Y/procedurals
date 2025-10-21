/**
 * @file TestProcedurals.cpp
 * @author Perry Chouteau (perry.chouteau@outlook.com)
 * @brief 
 * @date 2025-10-12
 * 
 * @example TestProcedurals.cpp
 * @{
 */

#include <gtest/gtest.h>
#include <iostream>
#include <fstream>
#include <functional>
#include "Noise.hpp"

TEST(Deterministic, noise2D) {
    std::cout << mix64(0) << std::endl;
    std::cout << random1D(1) << std::endl;
    std::cout << random2D(1, 1) << std::endl;
    std::cout << random3D(1, 1, 1) << std::endl;
    std::cout << random4D(1, 1, 1, 1) << std::endl;

    visualizer2D("noise.ppm", 512, 512, 50.0f, [](float x, float y) {
        return noise2D(x, y);
    });

    visualizer2D("fbm.ppm", 512, 512, 50.0f, [](float x, float y) {
        return fbm2D(x, y);
    });

    visualizer2D("ridge.ppm", 512, 512, 50.0f, [](float x, float y) {
        return ridge2D(x, y, 5);
    });

    visualizer2D("domainWarp.ppm", 512, 512, 50.f, [](float x, float y) {
        float w = warp2D(x, y);

        return noise2D(x + w, y + w);
    });

    visualizer2D("warp.ppm", 512, 512, 50.f, [](float x, float y) {
        return warp2D(x, y);
    });

    visualizer2D("loot.ppm", 512, 512, 50.0f, [](float x, float y) {
        return rarity2D(x, y, 3, 0.001);
    });

    visualizer2D("worley.ppm", 512, 512, 50.0f, [](float x, float y) {
        return worley2D(x, y);
    });

    visualizer2D("inv_worley.ppm", 512, 512, 50.0f, [](float x, float y) {
        return 1.0 - worley2D(x, y);
    });
    
    visualizer2D("simplex.ppm", 512, 512, 50.0f, [](float x, float y) {
        return Quantize<5>(simplex2D(x, y));
    });

    visualizer2D("final.ppm", 512, 512, 50.0f, [](float x, float y) {
        // 1️⃣ Base FBM pour terrain global
        float terrain = fbm2D(x, y, 5);
        // 2️⃣ Ajouter des crêtes/ridges pour montagnes
        float mountains = ridge2D(x * 1.5f, y * 1.5f, 4);
        // 3️⃣ Ajouter distorsion pour organique
        float warped = warp2D(x, y, 1, 2.0f);
        // 4️⃣ Combiner les couches
        float combined = terrain * 0.5 + warped * 0.5;

        // 5️⃣ Optionnel : appliquer seuil pour définir zones sol/eau/mur
        // combined = combined > 0.5f ? 1.0f : 0.0f; // binaire
        combined = Quantize<3>(combined);

        return combined; // valeur finale en [-1,1] ou [0,1] selon normalisation
    });
}