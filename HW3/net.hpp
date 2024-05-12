#pragma once
#include "include.hpp"

class Net {
    public:
        Net(std::string name, int sx, int sy, int tx, int ty) : net_name(name), source_x(sx), source_y(sy), target_x(tx), target_y(ty) {}
        void copy(Net* net) {
            net_name = net->net_name;
            source_x = net->source_x;
            source_y = net->source_y;
            target_x = net->target_x;
            target_y = net->target_y;
            length = net->length;
            path = net->path;
        }

        std::string net_name;
        int source_x;
        int source_y;
        int target_x;
        int target_y;

        int length;
        std::vector<std::pair<int, int>> path;
};