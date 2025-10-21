#include <gtest/gtest.h>
#include <vector>
#include <fstream>
#include <cmath>
#include <random>
#include <algorithm>
#include <iostream>

struct Point { float x, y; };

// -------------------- PPM --------------------
inline void savePPM(const std::string& filename, const std::vector<Point>& points, int width, int height) {
    std::ofstream out(filename, std::ios::binary);
    out << "P6\n" << width << " " << height << "\n255\n";

    for(int y=0;y<height;y++){
        for(int x=0;x<width;x++){
            unsigned char c = 0; // noir par défaut
            for(auto& p: points){
                if(int(p.x)==x && int(p.y)==y) { c=255; break; } // point blanc
            }
            out.put(c).put(c).put(c);
        }
    }
    out.close();
}

// -------------------- Poisson Disk --------------------
bool isValid(const Point& pt, float r, const std::vector<int>& grid, float cellSize, int gridW, int gridH, const std::vector<Point>& points) {
    int gx = std::min(int(pt.x / cellSize), gridW-1);
    int gy = std::min(int(pt.y / cellSize), gridH-1);

    int range = 2;
    for(int dy=-range; dy<=range; dy++)
        for(int dx=-range; dx<=range; dx++){
            int nx=gx+dx, ny=gy+dy;
            if(nx<0||ny<0||nx>=gridW||ny>=gridH) continue;
            int idx = grid[ny*gridW + nx];
            if(idx>=0){
                Point p = points[idx];
                float dist = std::hypot(pt.x-p.x, pt.y-p.y);
                if(dist < r) return false;
            }
        }
    return true;
}

std::vector<Point> poissonDisk(float width, float height, float r, int k=30, uint64_t seed=42) {
    float cellSize = r / std::sqrt(2.0f);
    int gridW = int(width / cellSize) + 1;
    int gridH = int(height / cellSize) + 1;

    std::vector<int> grid(gridW*gridH, -1);
    std::vector<Point> points;
    std::vector<int> active;

    std::mt19937 rng(seed);
    std::uniform_real_distribution<float> dx(0,width);
    std::uniform_real_distribution<float> dy(0,height);
    std::uniform_real_distribution<float> angle(0, 2*M_PI);
    std::uniform_real_distribution<float> radius(r, 2*r);

    // Premier point
    Point first{dx(rng), dy(rng)};
    points.push_back(first);
    grid[int(first.y/cellSize)*gridW + int(first.x/cellSize)] = 0;
    active.push_back(0);

    while(!active.empty()){
        int idx = active[rng()%active.size()];
        bool found=false;
        for(int i=0;i<k;i++){
            float a = angle(rng);
            float rad = radius(rng);
            Point cand{ points[idx].x + rad*cos(a), points[idx].y + rad*sin(a) };
            if(cand.x<0||cand.y<0||cand.x>=width||cand.y>=height) continue;
            if(isValid(cand, r, grid, cellSize, gridW, gridH, points)){
                points.push_back(cand);
                int cgx=int(cand.x/cellSize), cgy=int(cand.y/cellSize);
                grid[cgy*gridW + cgx] = points.size()-1;
                active.push_back(points.size()-1);
                found=true;
                break;
            }
        }
        if(!found) active.erase(std::remove(active.begin(), active.end(), idx), active.end());
    }
    return points;
}

// -------------------- Test GTest --------------------
TEST(PoissonDiskSampling, Visualize) {
    float width = 200, height=200;
    float r = width / height * 10; // distance min
    uint64_t seed = 123;

    auto points = poissonDisk(width, height, r, 10, seed);

    savePPM("poisson_disk.ppm", points, int(width), int(height));

    std::cout << "Generated " << points.size() << " points\n";

    // Check minimal distance (simple)
    for(size_t i=0;i<points.size();i++)
        for(size_t j=i+1;j<points.size();j++){
            float dist = std::hypot(points[i].x-points[j].x, points[i].y-points[j].y);
            EXPECT_GE(dist, r);
        }
}