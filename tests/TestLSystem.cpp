/**
 * @file TestLSystem.cpp
 * @author Perry Chouteau (perry.chouteau@outlook.com)
 * @brief 
 * @date 2025-10-13
 */

#include <gtest/gtest.h>
#include <vector>
#include <fstream>
#include <stack>
#include <cmath>
#include <string>
#include <iostream>

// ------------------ PPM helper ------------------
inline void savePPM(const std::string& filename, const std::vector<std::vector<int>>& grid){
    int H = grid.size(), W = grid[0].size();
    std::ofstream out(filename,std::ios::binary);
    out << "P6\n"<< W <<" "<< H << "\n255\n";
    for(int y=0;y<H;y++)
        for(int x=0;x<W;x++){
            unsigned char c = grid[y][x] ? 255 : 0;
            out.put(c).put(c).put(c);
        }
    out.close();
}

// ------------------ L-System ------------------
struct Turtle { float x, y, angle; };

// Apply deterministic rules
inline std::string applyRules(const std::string& input){
    std::string output;
    for(char c : input){
        if(c=='F') output += "F[+F]F[-F]F"; // simple fractal tree
        else output += c;
    }
    return output;
}

// Draw L-System on a 2D grid
inline void drawLSystem(const std::string& instructions, int W, int H,
                        std::vector<std::vector<int>>& grid, float step=1.0f, float angleDelta=25.0f){
    Turtle t{W/2.0f, 0.0f, 90.0f};
    std::stack<Turtle> stack;
    for(char c : instructions){
        if(c=='F'){
            float rad = t.angle * M_PI/180.0f;
            int x1 = int(t.x), y1 = int(t.y);
            t.x += step * cos(rad);
            t.y += step * sin(rad);
            int x2 = int(t.x), y2 = int(t.y);
            if(x2>=0 && x2<W && y2>=0 && y2<H) grid[y2][x2] = 1;
        }
        else if(c=='+') t.angle += angleDelta;
        else if(c=='-') t.angle -= angleDelta;
        else if(c=='[') stack.push(t);
        else if(c==']' && !stack.empty()) { t = stack.top(); stack.pop(); }
    }
}

// ------------------ Test ------------------
TEST(LSystemTest, GenerateFractalTree){
    std::string axiom = "F";
    int iterations = 7;
    for(int i=0;i<iterations;i++) axiom = applyRules(axiom);

    int W=512,H=512;
    std::vector<std::vector<int>> grid(H,std::vector<int>(W,0));

    drawLSystem(axiom,W,H,grid, 8.f, 90.0f);

    savePPM("lsystem_test.ppm", grid);

    // Simple verification : au moins une cellule dessinée
    int count=0;
    for(int y=0;y<H;y++)
        for(int x=0;x<W;x++)
            if(grid[y][x]==1) count++;

}