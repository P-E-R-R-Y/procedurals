/**
 * @file TestBSP.cpp
 * @author Perry Chouteau (perry.chouteau@outlook.com)
 * @brief 
 * @date 2025-10-13
 */

#include <iostream>
#include <vector>
#include <fstream>
#include <random>
#include <gtest/gtest.h>
#include <algorithm>
#include "Deterministic.hpp"
#include "Visualizer.hpp"

struct Rect {
    int x, y, w, h;
};
struct Point {
    int x, y;
};

// ------------------ BSP SPLIT ------------------
//void splitBSP(Rect r, int depth, std::vector<Rect>& rooms, std::mt19937& rng) {
//    if (depth == 0) { rooms.push_back(r); return; }
//
//    std::uniform_real_distribution<float> chance(0.0f, 1.0f);
//    bool splitH = (chance(rng) > 0.5f); // aléatoire H/V
//
//    if (splitH && r.h > 8) {
//        int split = r.y + r.h / 2 + (rng() % (r.h / 4)) - r.h / 8;
//        splitBSP({r.x, r.y, r.w, split - r.y}, depth - 1, rooms, rng);
//        splitBSP({r.x, split, r.w, r.y + r.h - split}, depth - 1, rooms, rng);
//    } 
//    else if (!splitH && r.w > 8) {
//        int split = r.x + r.w / 2 + (rng() % (r.w / 4)) - r.w / 8;
//        splitBSP({r.x, r.y, split - r.x, r.h}, depth - 1, rooms, rng);
//        splitBSP({split, r.y, r.x + r.w - split, r.h}, depth - 1, rooms, rng);
//    } 
//    else {
//        rooms.push_back(r);
//    }
//}

void splitBSP(Rect r, int depth, std::vector<Rect>& rooms, uint64_t seed = 0) {
    if (depth == 0) { rooms.push_back(r); return; }

    // Génération déterministe
    float chance = random2D(r.x, r.y, seed);
    bool splitH = (chance > 0.5f);

    // Empêcher les rectangles trop fins
    float ratio = static_cast<float>(r.w) / static_cast<float>(r.h);
    if (ratio > 1.5f) splitH = false;     // trop large → couper verticalement
    else if (ratio < 0.66f) splitH = true; // trop haut → couper horizontalement

    // Si le rectangle est trop petit
    if (r.w < 16 || r.h < 16) {
        rooms.push_back(r);
        return;
    }

    if (splitH) {
        // Calcul déterministe du split
        float randVal = random2D(r.x, r.y + 1, seed);
        int offset = static_cast<int>((randVal - 0.5f) * (r.h / 4));
        int split = r.y + (r.h / 2) + offset;

        if (split <= r.y + 4 || split >= r.y + r.h - 4) {
            rooms.push_back(r);
            return;
        }

        splitBSP({r.x, r.y, r.w, split - r.y}, depth - 1, rooms, seed + 1337);
        splitBSP({r.x, split, r.w, r.y + r.h - split}, depth - 1, rooms, seed + 7331);
    } 
    else {
        float randVal = random2D(r.x + 1, r.y, seed);
        int offset = static_cast<int>((randVal - 0.5f) * (r.w / 4));
        int split = r.x + (r.w / 2) + offset;

        if (split <= r.x + 4 || split >= r.x + r.w - 4) {
            rooms.push_back(r);
            return;
        }

        splitBSP({r.x, r.y, split - r.x, r.h}, depth - 1, rooms, seed + 42);
        splitBSP({split, r.y, r.x + r.w - split, r.h}, depth - 1, rooms, seed + 99);
    }
}

// ------------------ OUTILS ------------------
Point center(const Rect& r) { return {r.x + r.w / 2, r.y + r.h / 2}; }

void drawRect(std::vector<std::vector<float>>& map, const Rect& r, float val) {
    for (int y = r.y; y < r.y + r.h; ++y)
        for (int x = r.x; x < r.x + r.w; ++x)
            if (y >= 0 && x >= 0 && y < (int)map.size() && x < (int)map[0].size())
                map[y][x] = val;
}

void connectRooms(std::vector<std::vector<float>>& map, const Rect& a, const Rect& b, int corridorWidth = 5) {
    Point pa = center(a);
    Point pb = center(b);

    // 1️⃣ Couloir horizontal
    for (int x = std::min(pa.x, pb.x); x <= std::max(pa.x, pb.x); ++x) {
        for (int w = -corridorWidth; w <= corridorWidth; ++w) {
            int y = pa.y + w;
            if (y >= 0 && y < (int)map.size() && x >= 0 && x < (int)map[0].size())
                map[y][x] = 0;
        }
    }

    // 2️⃣ Couloir vertical
    for (int y = std::min(pa.y, pb.y); y <= std::max(pa.y, pb.y); ++y) {
        for (int w = -corridorWidth; w <= corridorWidth; ++w) {
            int x = pb.x + w;
            if (y >= 0 && y < (int)map.size() && x >= 0 && x < (int)map[0].size())
                map[y][x] = 0;
        }
    }
}
Point wallPoint(const Rect& from, const Rect& to) {
    Point p;
    if (to.x + to.w/2 < from.x + from.w/2) p.x = from.x;       // gauche
    else p.x = from.x + from.w - 1;                             // droite

    if (to.y + to.h/2 < from.y + from.h/2) p.y = from.y;       // haut
    else p.y = from.y + from.h - 1;                             // bas
    return p;
}

Point wallPointSafe(const Rect& from, const Rect& to, int corridorWidth) {
    Point p = center(from);

    // Déterminer le mur le plus proche
    if (to.x + to.w/2 < from.x + from.w/2)      // B est à gauche
        p.x = from.x - corridorWidth/2;         // décale vers l’extérieur
    else if (to.x + to.w/2 > from.x + from.w/2) // B est à droite
        p.x = from.x + from.w - 1 + corridorWidth/2;

    if (to.y + to.h/2 < from.y + from.h/2)      // B est au-dessus
        p.y = from.y - corridorWidth/2;
    else if (to.y + to.h/2 > from.y + from.h/2) // B est en dessous
        p.y = from.y + from.h - 1 + corridorWidth/2;

    return p;
}

void carveCorridor(std::vector<std::vector<float>>& map,
                   const Rect& a, const Rect& b,
                   int corridorWidth = 2)
{
    Point pa = wallPointSafe(a, b, corridorWidth);
    Point pb = wallPointSafe(b, a, corridorWidth);

    // horizontal
    for (int x = std::min(pa.x, pb.x); x <= std::max(pa.x, pb.x); ++x)
        for (int w = -corridorWidth/2; w <= corridorWidth/2; ++w)
            if (pa.y + w >= 0 && pa.y + w < (int)map.size())
                map[pa.y + w][x] = 0;

    // Creuser verticalement
    for (int y = std::min(pa.y, pb.y); y <= std::max(pa.y, pb.y); ++y)
        for (int w = -corridorWidth/2; w <= corridorWidth/2; ++w)
            if (pb.x + w >= 0 && pb.x + w < (int)map[0].size())
                map[y][pb.x + w] = 0;
}

// ------------------ MAIN ------------------
TEST(BSP, test) {
    const int W = 256, H = 256;
    std::vector<std::vector<float>> map(H, std::vector<float>(W, 1.0f)); // 1.0 = mur

    std::vector<Rect> rooms;
    splitBSP({0, 0, W, H}, 4, rooms);

    // Rétrécir les pièces pour éviter le collage
    for (auto& r : rooms) {
        r.x += 3; r.y += 3; r.w -= 6; r.h -= 6;
        drawRect(map, r, 0); // noir = sol
    }

    // Connecter les pièces (ici, simplifié : pièce[i] ↔ pièce[i+1])
    for (size_t i = 0; i + 1 < rooms.size(); ++i)
        connectRooms(map, rooms[i], rooms[i + 1], 5);

    visualizer2D("bsp_dungeon.ppm", map);
    std::cout << "✅ Donjon généré : bsp_dungeon.ppm" << std::endl;
}