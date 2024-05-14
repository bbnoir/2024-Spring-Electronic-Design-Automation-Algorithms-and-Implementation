#pragma once
#include "include.hpp"

class Net {
    public:
        Net() {}
        Net(int id, std::string name, int sx, int sy, int tx, int ty) : net_id(id), net_name(name), source_x(sx), source_y(sy), target_x(tx), target_y(ty) {}
        void copy(Net* net) {
            net_id = net->net_id;
            net_name = net->net_name;
            source_x = net->source_x;
            source_y = net->source_y;
            target_x = net->target_x;
            target_y = net->target_y;
            length = net->length;
            path = net->path;
        }

        int net_id;
        std::string net_name;
        int source_x;
        int source_y;
        int target_x;
        int target_y;

        int length;
        std::vector<std::pair<int, int>> path;
        int path_size;
        std::vector<std::vector<int>> cost_map;
};