#pragma once
#include "include.hpp"

class Net {
    public:
        Net(int id, int s_x, int s_y, int t_x, int t_y) : net_id(id), source_x(s_x), source_y(s_y), target_x(t_x), target_y(t_y) {}

        int net_id;
        std::string net_name;
        int source_x;
        int source_y;
        int target_x;
        int target_y;

        int length;
        std::vector<std::pair<int, int>> path;
};