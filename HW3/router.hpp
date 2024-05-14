#pragma once
#include "include.hpp"
#include "net.hpp"

class Router {
    public:
        Router(std::string filename);
        void route();
        bool routeBruteForce();
        bool routeCostBased();
        bool routeOneNet(Net* net);
        void writeResults(std::string filename);
        bool isOnGrid(int x, int y);

        void showGrid();

        const t_grid EMPTY = 0;
        const t_grid BLOCK = -1;
        const t_grid WIRE = -2;

        const int dx[4] = {1, 0, -1, 0};
        const int dy[4] = {0, 1, 0, -1};

        int grid_size_row;
        int grid_size_col;
        std::vector<std::vector<t_grid>> grid;
        int num_nets;
        std::vector<Net*> nets;
};