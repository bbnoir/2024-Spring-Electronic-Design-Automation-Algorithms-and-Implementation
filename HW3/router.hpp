#pragma once
#include "include.hpp"
#include "net.hpp"

class Router {
    public:
        Router(std::string filename);
        void route();
        bool routeBruteForce();
        bool routeMinBend();
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
        int dir_order[4] = {0, 1, 2, 3};
        bool min_bend = true;

        int grid_size_row;
        int grid_size_col;
        std::vector<std::vector<t_grid>> grid;
        int num_nets;
        std::vector<Net*> nets;

        int best_total_length;
        std::vector<Net*> init_nets;
        std::vector<Net*> best_nets;
        std::vector<std::vector<t_grid>> clean_grid;

        std::vector<std::vector<int>> shuffled_net_order;
        int num_shuffled_net_order;

        // timer
        const int time_limit = 90; // seconds
        std::chrono::time_point<std::chrono::system_clock> start_time;
        bool checkTimeOut();
        bool checkTimeOut(int local_time_limit);
};