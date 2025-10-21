#include <gtest/gtest.h>
#include <vector>
#include <fstream>
#include <algorithm>
#include <cstdint>
#include <random>
#include <iostream>

// ------------------ Deterministic random2D ------------------
extern float random2D(int x, int y, uint64_t seed);

// ------------------ PPM ------------------
inline void savePPM(const std::string& filename, const std::vector<std::vector<int>>& grid, int maxVal=2) {
    int H = grid.size(), W = grid[0].size();
    std::ofstream out(filename, std::ios::binary);
    out << "P6\n" << W << " " << H << "\n255\n";
    for (int y = 0; y < H; ++y)
        for (int x = 0; x < W; ++x) {
            float norm = float(grid[y][x]) / float(maxVal);
            unsigned char c = static_cast<unsigned char>(norm * 255.0f);
            out.put(c).put(c).put(c);
        }
    out.close();
}

// ------------------ WFC ------------------
struct Cell { 
    std::vector<int> possible; 
    bool collapsed() const { return possible.size()==1; } 
};
using Grid = std::vector<std::vector<Cell>>;

// Définir les voisins autorisés pour chaque tile
const std::vector<std::vector<int>> neighbors = {
//    {0,1},    // tile 0 peut avoir 0 ou 1 à côté
//    {0,1,2},  // tile 1 peut avoir 0,1,2 à côté
//    {1,2},     // tile 2
//    {1,3},     // tile 3
//    {1,4}     // tile 4
    {0,1}, // 0 peut avoir toutes les tiles à côté
    {0,1,2}, // 1 pareil
    {1,2,3}, // 2 pareil
    {2,3}, // 3 pareil
};

// Choix déterministe d’un état
int chooseRandom(const std::vector<int>& options, int x, int y, uint64_t seed) {
    float r = random2D(x, y, seed);
    return options[static_cast<int>(r*options.size()) % options.size()];
}

// ------------------ WFC main ------------------
void waveFunctionCollapse(Grid& grid, uint64_t seed = 0) {
    int H = grid.size(), W = grid[0].size();
    while(true) {
        // Trouver cellule non collapsée avec entropie minimale
        int minOptions = 1000, selX=-1, selY=-1;
        for(int y=0;y<H;y++)
            for(int x=0;x<W;x++)
                if(!grid[y][x].collapsed() && grid[y][x].possible.size()<minOptions){
                    minOptions = grid[y][x].possible.size();
                    selX = x; selY = y;
                }
        if(selX==-1) break; // tout est résolu

        // Collapse
        int choice = chooseRandom(grid[selY][selX].possible, selX, selY, seed);
        grid[selY][selX].possible = {choice};

        // Propagation simple avec règles de voisins
        const int dx[4]={1,-1,0,0}, dy[4]={0,0,1,-1};
        for(int d=0;d<4;d++){
            int nx=selX+dx[d], ny=selY+dy[d];
            if(nx>=0 && nx<W && ny>=0 && ny<H){
                auto& nbr = grid[ny][nx].possible;
                // garder seulement les états autorisés par le choix
                std::vector<int> allowed;
                for(int val : nbr)
                    for(int a : neighbors[choice])
                        if(val == a) allowed.push_back(val);
                if(!allowed.empty())
                    nbr = allowed;
                else
                    nbr = {choice}; // fallback
            }
        }
    }
}

// ------------------ Test ------------------
TEST(WaveFunctionCollapse, GenerateMap) {
    int W=50,H=50;
    Grid grid(H,std::vector<Cell>(W, Cell{{0,1,2,3, 4}}));
    uint64_t seed = 0;

    waveFunctionCollapse(grid, seed);

    // Convert Grid -> int map for PPM
    std::vector<std::vector<int>> map(H,std::vector<int>(W));
    for(int y=0;y<H;y++)
        for(int x=0;x<W;x++)
            map[y][x] = grid[y][x].possible[0];

    savePPM("wfc_test.ppm", map, 4);

    // Vérifier que toutes les cellules sont collapsées
    for(int y=0;y<H;y++)
        for(int x=0;x<W;x++)
            EXPECT_TRUE(grid[y][x].collapsed());
}