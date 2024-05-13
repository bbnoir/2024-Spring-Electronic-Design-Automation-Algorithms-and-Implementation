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
        } else if (token == ".col") {
            ss >> grid_size_col;
            if (grid_size_row > 0 && grid_size_col > 0) {
                grid.resize(grid_size_row, std::vector<t_grid>(grid_size_col, EMPTY));
            }
        } else if (token == ".block") {
            int num_blocks;
            ss >> num_blocks;
            for (int i = 0; i < num_blocks; i++) {
                ss.clear();
                std::getline(fin, line);
                ss.str(line);
                int left_down_x, right_up_x, left_down_y, right_up_y;
                ss >> left_down_y >> right_up_y >> left_down_x >> right_up_x;
                for (int x = left_down_x; x <= right_up_x; x++) {
                    for (int y = left_down_y; y <= right_up_y; y++) {
                        grid[x][y] = BLOCK;
                    }
                }
            }
        } else if (token == ".net") {
            ss >> num_nets;
            for (int i = 0; i < num_nets; i++) {
                std::string net_name;
                int source_x, source_y, target_x, target_y;
                ss.clear();
                std::getline(fin, line);
                ss.str(line);
                ss >> net_name >> source_y >> source_x >> target_y >> target_x;
                Net* net = new Net(net_name, source_x, source_y, target_x, target_y);
                nets.push_back(net);
                grid[source_x][source_y] = WIRE;
                grid[target_x][target_y] = WIRE;
            }
        }
    }
}

void Router::route() {
    int net_order[num_nets];
    for (int i = 0; i < num_nets; i++) {
        net_order[i] = i;
    }

    std::vector<Net*> best_nets;
    for (int i = 0; i < num_nets; i++) {
        best_nets.push_back(new Net("", 0, 0, 0, 0));
    }
    int best_total_length = -1;

    std::vector<std::vector<t_grid>> clean_grid = grid;

    do {
        grid = clean_grid;
        // show permutation
        // std::cout << "Permutation: ";
        // for (int i = 0; i < num_nets; i++) {
            // std::cout << net_order[i] << " ";
        // }
        // std::cout << std::endl;
        bool success = true;
        for (int i = 0; success && i < num_nets; i++) {
            success = routeOneNet(nets[net_order[i]]);
        }
        if (!success) {
            // std::cout << "Failed to route" << std::endl << std::endl;
            continue;
        }
        int total_length = 0;
        for (int i = 0; i < num_nets; i++) {
            total_length += nets[i]->length;
        }
        if (best_total_length == -1 || total_length < best_total_length) {
            best_total_length = total_length;
            for (int i = 0; i < num_nets; i++) {
                best_nets[i]->copy(nets[i]);
            }
        }
        // show total length
        // std::cout << "Total length: " << total_length << std::endl << std::endl;
    } while (std::next_permutation(net_order, net_order + num_nets));

    for (int i = 0; i < num_nets; i++) {
        nets[i]->copy(best_nets[i]);
    }
}

bool Router::routeOneNet(Net* net) {
    // Lee's algorithm
    std::vector<std::pair<int, int>> queue;
    queue.push_back(std::make_pair(net->source_x, net->source_y));
    std::vector<std::vector<t_grid>> grid_copy = grid;
    grid[net->source_x][net->source_y] = 1;
    grid[net->target_x][net->target_y] = EMPTY;
    int head = 0;
    int tail = 1;
    bool found = false;
    while (head < tail) {
        int x = queue[head].first;
        int y = queue[head].second;
        if (x == net->target_x && y == net->target_y) {
            found = true;
            break;
        }
        if (x > 0 && grid[x - 1][y] == EMPTY) {
            queue.push_back(std::make_pair(x - 1, y));
            grid[x - 1][y] = grid[x][y] + 1;
            tail++;
        }
        if (x < grid_size_row - 1 && grid[x + 1][y] == EMPTY) {
            queue.push_back(std::make_pair(x + 1, y));
            grid[x + 1][y] = grid[x][y] + 1;
            tail++;
        }
        if (y > 0 && grid[x][y - 1] == EMPTY) {
            queue.push_back(std::make_pair(x, y - 1));
            grid[x][y - 1] = grid[x][y] + 1;
            tail++;
        }
        if (y < grid_size_col - 1 && grid[x][y + 1] == EMPTY) {
            queue.push_back(std::make_pair(x, y + 1));
            grid[x][y + 1] = grid[x][y] + 1;
            tail++;
        }
        head++;
    }
    // showGrid();
    if (!found) {
        for (Net* net : nets) {
            net->length = -1;
            net->path.clear();
        }
        return false;
    }
    int x = net->target_x;
    int y = net->target_y;
    net->path.push_back(std::make_pair(x, y));
    int prev_dir = -1, cur_dir = -1;
    // save path if previous direction is not the same as current direction
    for (int i = grid[x][y] - 1; i > 0; i--) {
        if (x > 0 && grid[x - 1][y] == i) {
            cur_dir = 0;
        } else if (x < grid_size_row - 1 && grid[x + 1][y] == i) {
            cur_dir = 1;
        } else if (y > 0 && grid[x][y - 1] == i) {
            cur_dir = 2;
        } else if (y < grid_size_col - 1 && grid[x][y + 1] == i) {
            cur_dir = 3;
        }
        if (prev_dir != -1 && prev_dir != cur_dir) {
            net->path.push_back(std::make_pair(x, y));
        }
        prev_dir = cur_dir;
        if (cur_dir == 0) {
            x--;
        } else if (cur_dir == 1) {
            x++;
        } else if (cur_dir == 2) {
            y--;
        } else if (cur_dir == 3) {
            y++;
        }
    }
    if (net->path.back() != std::make_pair(net->source_x, net->source_y)) {
        net->path.push_back(std::make_pair(net->source_x, net->source_y));
    }
    grid = grid_copy;
    net->length = -1;
    for (int i = 0; i < net->path.size()-1; i++) {
        int sx = net->path[i].first;
        int sy = net->path[i].second;
        int tx = net->path[i + 1].first;
        int ty = net->path[i + 1].second;
        if (sx == tx) {
            for (int y = std::min(sy, ty); y <= std::max(sy, ty); y++) {
                if (grid[sx][y] == BLOCK) {
                    return false;
                }
                grid[sx][y] = WIRE;
            }
            net->length += std::abs(sy - ty);
        } else {
            for (int x = std::min(sx, tx); x <= std::max(sx, tx); x++) {
                if (grid[x][sy] == BLOCK) {
                    return false;
                }
                grid[x][sy] = WIRE;
            }
            net->length += std::abs(sx - tx);
        }
    }
    // showGrid();
    return true;
}

void Router::writeResults(std::string filename) {
    std::ofstream fout(filename);
    for (int i = 0; i < num_nets; i++) {
        Net* net = nets[i];
        fout << net->net_name << " " << net->length << std::endl;
        fout << "begin" << std::endl;
        for (int j = net->path.size() - 1; j > 0; j--) {
            fout << net->path[j].second << " " << net->path[j].first << " " << net->path[j-1].second << " " << net->path[j-1].first << std::endl;
        }
        fout << "end" << std::endl;
    }
}

void Router::showGrid() {
    for (int j = 0; j < grid_size_row; j++) {
        std::cout << "===";
    }
    std::cout << std::endl;
    for (int i = grid_size_col - 1; i >= 0; i--) {
        for (int j = 0; j < grid_size_row; j++) {
            if (grid[j][i] == EMPTY) {
                std::cout << "   ";
            } else if (grid[j][i] == BLOCK) {
                std::cout << "  #";
            } else if (grid[j][i] == WIRE) {
                std::cout << "  *";
            } else {
                std::cout << std::setw(3) << grid[j][i];
            }
        }
        std::cout << std::endl;
    }
    for (int j = 0; j < grid_size_row; j++) {
        std::cout << "===";
    }
    std::cout << std::endl;
}