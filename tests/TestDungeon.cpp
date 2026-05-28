/**
 * @file TestDungeon.cpp
 * @brief Per-step tests for the TinyKeepDev pipeline.
 *        Each step writes a .ppm so you can eyeball the result.
 */

#include <gtest/gtest.h>
#include <cmath>
#include <vector>
#include "TinyKeepDev.hpp"
#include "Visualizer.hpp"

namespace {

constexpr int   W = 512;
constexpr int   H = 512;
constexpr float RADIUS = 200.0f;
constexpr int   NB_RECTS = 10;
constexpr uint64_t SEED = 2;

// world space (origin-centered) → grid pixel
inline std::pair<int,int> toGrid(float wx, float wy) {
    int gx = int((wx + RADIUS) / (2 * RADIUS) * W);
    int gy = int((wy + RADIUS) / (2 * RADIUS) * H);
    return {gx, gy};
}

inline void drawRect(std::vector<std::vector<float>>& map, const Rectf& r, float val) {
    auto [x0, y0] = toGrid(r.x - r.w / 2, r.y - r.h / 2);
    auto [x1, y1] = toGrid(r.x + r.w / 2, r.y + r.h / 2);
    for (int y = std::max(0, y0); y < std::min(H, y1); ++y)
        for (int x = std::max(0, x0); x < std::min(W, x1); ++x)
            map[y][x] = val;
}

// Bresenham
inline void drawLine(std::vector<std::vector<float>>& map, const Linef& l, float val) {
    auto [x0, y0] = toGrid(l.p1.x, l.p1.y);
    auto [x1, y1] = toGrid(l.p2.x, l.p2.y);
    int dx = std::abs(x1 - x0), sx = x0 < x1 ? 1 : -1;
    int dy = -std::abs(y1 - y0), sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;
    while (true) {
        if (x0 >= 0 && x0 < W && y0 >= 0 && y0 < H) map[y0][x0] = val;
        if (x0 == x1 && y0 == y1) break;
        int e2 = 2 * err;
        if (e2 >= dy) { err += dy; x0 += sx; }
        if (e2 <= dx) { err += dx; y0 += sy; }
    }
}

inline int countOverlaps(const std::vector<Rectf>& rects) {
    int n = 0;
    for (size_t i = 0; i < rects.size(); ++i)
        for (size_t j = i + 1; j < rects.size(); ++j) {
            const auto& a = rects[i];
            const auto& b = rects[j];
            bool overlap = !(a.x + a.w / 2 < b.x - b.w / 2 ||
                             a.x - a.w / 2 > b.x + b.w / 2 ||
                             a.y + a.h / 2 < b.y - b.h / 2 ||
                             a.y - a.h / 2 > b.y + b.h / 2);
            if (overlap) ++n;
        }
    return n;
}

} // namespace

// ----------------------------------------------------------------------------
// Step 1: generate rectangles inside a disk
// ----------------------------------------------------------------------------
TEST(TinyKeepDev, Step1_GenerateRects) {
    auto rects = TinyKeepDev::generateRects<NB_RECTS>(RADIUS, 0.5f, {0.25f, 0.75f}, SEED);

    ASSERT_EQ(rects.size(), (size_t)NB_RECTS);

    // every rect corner should be inside the disk
    for (const auto& r : rects) {
        float farthest = std::hypot(r.x, r.y) + std::hypot(r.w, r.h) / 2.0f;
        EXPECT_LE(farthest, RADIUS + 1.0f);
    }

    std::vector<std::vector<float>> map(H, std::vector<float>(W, 0.0f));
    for (const auto& r : rects) drawRect(map, r, 1.0f);
    visualizer2D("tinykeep_1_generate.ppm", map);
}

// ----------------------------------------------------------------------------
// Step 2: relax — overlaps should not increase, ideally decrease
// ----------------------------------------------------------------------------
TEST(TinyKeepDev, Step2_RelaxRects) {
    auto rects = TinyKeepDev::generateRects<NB_RECTS>(RADIUS, 0.5f, {0.25f, 0.75f}, SEED);
    int before = countOverlaps(rects);

    TinyKeepDev::relaxRects<100>(rects);
    int after = countOverlaps(rects);

    EXPECT_LE(after, before) << "relax should not create overlaps (was "
                             << before << ", now " << after << ")";

    std::vector<std::vector<float>> map(H, std::vector<float>(W, 0.0f));
    for (const auto& r : rects) drawRect(map, r, 1.0f);
    visualizer2D("tinykeep_2_relax.ppm", map);
}

// ----------------------------------------------------------------------------
// Step 3: Bowyer-Watson Delaunay triangulation
// ----------------------------------------------------------------------------
TEST(TinyKeepDev, Step3_BowyerWatson) {
    auto rects = TinyKeepDev::generateRects<NB_RECTS>(RADIUS, 0.5f, {0.25f, 0.75f}, SEED);
    TinyKeepDev::relaxRects<100>(rects);
    auto edges = TinyKeepDev::bowyerWatson(rects);

    EXPECT_FALSE(edges.empty()) << "Delaunay produced no edges";
    // a connected planar triangulation of N points has at least N-1 edges
    EXPECT_GE(edges.size(), (size_t)(NB_RECTS - 1));

    std::vector<std::vector<float>> map(H, std::vector<float>(W, 0.0f));
    for (const auto& r : rects) drawRect(map, r, 0.4f);   // rooms darker
    for (const auto& e : edges) drawLine(map, e, 1.0f);    // edges bright
    visualizer2D("tinykeep_3_delaunay.ppm", map);
}

// ----------------------------------------------------------------------------
// Step 4: MST on the Delaunay graph
// ----------------------------------------------------------------------------
TEST(TinyKeepDev, Step4_MST) {
    auto rects = TinyKeepDev::generateRects<NB_RECTS>(RADIUS, 0.5f, {0.25f, 0.75f}, SEED);
    TinyKeepDev::relaxRects<100>(rects);
    auto delaunay  = TinyKeepDev::bowyerWatson(rects);
    auto mst       = TinyKeepDev::mst(delaunay);

    EXPECT_EQ(mst.size(), rects.size() - 1);  // a tree on N nodes has N-1 edges

    std::vector<std::vector<float>> map(H, std::vector<float>(W, 0.0f));
    for (const auto& r : rects) drawRect(map, r, 0.4f);
    for (const auto& e : mst)   drawLine(map, e, 1.0f);
    visualizer2D("tinykeep_4_mst.ppm", map);
}

TEST(TinyKeepDev, Step5_ShortenLine) {
    auto rects = TinyKeepDev::generateRects<NB_RECTS>(RADIUS, 0.5f, {0.25f, 0.75f}, SEED);
    rects.resize(8);  // fewer rooms → more visible shortening effect
    TinyKeepDev::relaxRects<100>(rects);
    auto delaunay  = TinyKeepDev::bowyerWatson(rects);
    auto mst       = TinyKeepDev::mst(delaunay);
    auto corridor  = TinyKeepDev::shortenLine(rects, mst);  // should be the same if MST is deterministic

    EXPECT_EQ(mst.size(), rects.size() - 1);  // a tree on N nodes has N-1 edges

    std::vector<std::vector<float>> map(H, std::vector<float>(W, 0.0f));
    for (const auto& r : rects) drawRect(map, r, 0.1f);
    for (const auto& e : delaunay)   drawLine(map, e, 0.2f);
    for (const auto& e : mst)   drawLine(map, e, 0.6f);
    for (const auto& e : corridor)   drawLine(map, e, 1.0f);
    visualizer2D("tinykeep_5_shorten.ppm", map);
}