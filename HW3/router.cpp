#include "router.hpp"

Router::Router(std::string filename) {
    std::ifstream fin(filename);
    if (!fin) {
        std::cerr << "Error opening file: " << filename << std::endl;
        exit(1);
    }
    grid_size_row = 0;
    grid_size_col = 0;
    while (!fin.eof()) {
        std::string line;
        std::getline(fin, line);
        std::stringstream ss(line);
        std::string token;
        ss >> token;
        if (token == ".row") {
            ss >> grid_size_row;
            if (grid_size_row > 0 && grid_size_col > 0) {
                grid.resize(grid_size_row, std::vector<t_grid>(grid_size_col, EMPTY));
            }
            std::cout << "grid_size_row: " << grid_size_row << std::endl;
        } else if (token == ".col") {
            ss >> grid_size_col;
            if (grid_size_row > 0 && grid_size_col > 0) {
                grid.resize(grid_size_row, std::vector<t_grid>(grid_size_col, EMPTY));
            }
            std::cout << "grid_size_col: " << grid_size_col << std::endl;
        } else if (token == ".block") {
            int num_blocks;
            ss >> num_blocks;
            for (int i = 0; i < num_blocks; i++) {
                ss.clear();
                std::getline(fin, line);
                ss.str(line);
                int left_down_x, left_down_y, right_up_x, right_up_y;
                ss >> left_down_x >> left_down_y >> right_up_x >> right_up_y;
                std::cout << left_down_x << " " << left_down_y << " " << right_up_x << " " << right_up_y << std::endl;
                for (int x = left_down_x; x <= right_up_x; x++) {
                    for (int y = left_down_y; y <= right_up_y; y++) {
                        grid[x][y] = BLOCK;
                    }
                }
            }
            std::cout << "num_blocks: " << num_blocks << std::endl;
        } else if (token == ".net") {
            ss >> num_nets;
            for (int i = 0; i < num_nets; i++) {
                std::string net_name;
                int source_x, source_y, target_x, target_y;
                ss.clear();
                std::getline(fin, line);
                ss.str(line);
                ss >> net_name >> source_x >> source_y >> target_x >> target_y;
                Net* net = new Net(i, source_x, source_y, target_x, target_y);
                nets.push_back(net);
            }
            std::cout << "num_nets: " << num_nets << std::endl;
        }
    }
}

void Router::route() {
    for (int i = 0; i < num_nets; i++) {
        Net* net = nets[i];
        net->path.push_back(std::make_pair(net->source_x, net->source_y));
        net->path.push_back(std::make_pair(net->target_x, net->target_y));
        net->length = 2;
    }
}

void Router::writeResults(std::string filename) {
    std::ofstream fout(filename);
    for (int i = 0; i < num_nets; i++) {
        Net* net = nets[i];
        fout << net->net_name << net->length << std::endl;
        fout << "begin" << std::endl;
        for (int j = 0; j < net->length; j++) {
            fout << net->path[j].first << " " << net->path[j].second << std::endl;
        }
        fout << "end" << std::endl;
    }
}