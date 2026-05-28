/**
 * @file WaveFunctionCollapse.hpp
 * @author Perry Chouteau
 * @brief Tile-based WFC, best-effort. Always returns a grid.
 *        On contradiction, the offending constraint is skipped and generation continues.
 */

#pragma once

#include <queue>
#include <vector>
#include "Deterministic.hpp"

class WaveFunctionCollapse {
public:
    /**
     * @brief User-defined adjacency rules.
     *        Tile IDs are indices 0..neighbors.size()-1.
     *        neighbors[t] = list of tile IDs allowed next to tile t (any 4-dir).
     */
    struct Rules {
        std::vector<std::vector<int>> neighbors;
        int tileCount() const { return (int)neighbors.size(); }
    };

    /**
     * @brief Generate a width x height tile map.
     *        Best-effort: returns a complete grid even if a few adjacencies
     *        end up violating the rules (contradictions are skipped locally).
     */
    static std::vector<std::vector<int>> collapse(int width, int height,
                                                  const Rules& rules,
                                                  uint64_t seed = 0)
    {
        if (rules.tileCount() <= 0 || width <= 0 || height <= 0)
            return std::vector<std::vector<int>>(std::max(0, height),
                                                 std::vector<int>(std::max(0, width), 0));

        const int W = width, H = height;
        const int tileCount = rules.tileCount();

        std::vector<int> all(tileCount);
        for (int i = 0; i < tileCount; ++i) all[i] = i;
        std::vector<std::vector<std::vector<int>>> cell(
            H, std::vector<std::vector<int>>(W, all));

        using Entry = std::tuple<int, int, int>;
        std::priority_queue<Entry, std::vector<Entry>, std::greater<Entry>> pq;
        for (int y = 0; y < H; ++y)
            for (int x = 0; x < W; ++x)
                pq.emplace(tileCount, x, y);

        const int dxs[4] = {1, -1, 0, 0};
        const int dys[4] = {0, 0, 1, -1};
        uint64_t s = seed ? seed : 1;

        while (!pq.empty()) {
            auto [ent, x, y] = pq.top(); pq.pop();
            if ((int)cell[y][x].size() != ent) continue;  // stale
            if (cell[y][x].size() <= 1)        continue;  // collapsed

            s = mix64(s);
            int choice = cell[y][x][s % cell[y][x].size()];
            cell[y][x] = {choice};

            std::vector<std::pair<int,int>> work{{x, y}};
            while (!work.empty()) {
                auto [cx, cy] = work.back(); work.pop_back();

                std::vector<unsigned char> allow(tileCount, 0);
                for (int t : cell[cy][cx])
                    for (int a : rules.neighbors[t])
                        if (a >= 0 && a < tileCount) allow[a] = 1;

                for (int d = 0; d < 4; ++d) {
                    int nx = cx + dxs[d], ny = cy + dys[d];
                    if (nx < 0 || nx >= W || ny < 0 || ny >= H) continue;

                    std::vector<int> kept;
                    kept.reserve(cell[ny][nx].size());
                    for (int v : cell[ny][nx]) if (allow[v]) kept.push_back(v);

                    if (kept.empty()) continue;   // contradiction → skip, accept local violation

                    if (kept.size() < cell[ny][nx].size()) {
                        cell[ny][nx] = std::move(kept);
                        pq.emplace((int)cell[ny][nx].size(), nx, ny);
                        work.emplace_back(nx, ny);
                    }
                }
            }
        }

        std::vector<std::vector<int>> result(H, std::vector<int>(W));
        for (int y = 0; y < H; ++y)
            for (int x = 0; x < W; ++x)
                result[y][x] = cell[y][x].empty() ? 0 : cell[y][x][0];
        return result;
    }
};