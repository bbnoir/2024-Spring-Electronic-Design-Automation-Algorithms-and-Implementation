#include "router.hpp"

Router::Router(std::string filename) {
    start_time = std::chrono::system_clock::now();
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
                Net* net = new Net(i, net_name, source_x, source_y, target_x, target_y);
                nets.push_back(net);
                grid[source_x][source_y] = WIRE;
                grid[target_x][target_y] = WIRE;
            }
        }
    }
    for (int i = 0; i < num_nets; i++) {
        best_nets.push_back(new Net());
        init_nets.push_back(new Net());
        init_nets[i]->copy(nets[i]);
    }
    best_total_length = -1;
    clean_grid = grid;
    int net_order[num_nets];
    for (int i = 0; i < num_nets; i++) {
        net_order[i] = i;
    }
    do {
        shuffled_net_order.push_back(std::vector<int>(net_order, net_order + num_nets));
    } while (std::next_permutation(net_order, net_order + num_nets));
    std::random_shuffle(shuffled_net_order.begin(), shuffled_net_order.end());
    num_shuffled_net_order = shuffled_net_order.size();
}

void Router::route() {
    routeMinBend();
    routeBruteForce();
    if (best_total_length == -1) {
        routeCostBased();
    }
}

bool Router::routeBruteForce() {
    min_bend = false;
    const int local_time_limit = 60;
    for (int i = 0; i < num_nets; i++) {
        nets[i]->copy(init_nets[i]);
    }

    int found_count = 0;
    std::sort(dir_order, dir_order + 4);
    do {
        for (int v = 0; v < num_shuffled_net_order; v++) {
            if (checkTimeOut(local_time_limit)) {
                break;
            }
            grid = clean_grid;
            bool success = true;
            std::vector<int> net_order = shuffled_net_order[v];
            for (int i = 0; success && i < num_nets; i++) {
                success = routeOneNet(nets[net_order[i]]);
                if (!success) {
                    break;
                }
            }
            if (!success) {
                continue;
            }
            found_count++;
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
        }
        if (found_count > 10) {
            break;
        }
    } while (std::next_permutation(dir_order, dir_order + 4));

    return best_total_length != -1;
}

bool Router::routeMinBend() {
    min_bend = true;
    const int local_time_limit = 30;

    for (int v = 0; v < num_shuffled_net_order; v++) {
        if (checkTimeOut(local_time_limit)) {
            break;
        }
        grid = clean_grid;
        bool success = true;
        std::vector<int> net_order = shuffled_net_order[v];
        for (int i = 0; success && i < num_nets; i++) {
            success = routeOneNet(nets[net_order[i]]);
            if (!success) {
                break;
            }
        }
        if (!success) {
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
    }

    return best_total_length != -1;
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
        for (int i = 0; i < 4; i++) {
            int new_x = x + dx[i];
            int new_y = y + dy[i];
            if (isOnGrid(new_x, new_y) && grid[new_x][new_y] == EMPTY) {
                queue.push_back(std::make_pair(new_x, new_y));
                grid[new_x][new_y] = grid[x][y] + 1;
                tail++;
            }
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
        for (int j = 0; j < 4; j++) {
            int new_x = x + dx[dir_order[j]];
            int new_y = y + dy[dir_order[j]];
            if (isOnGrid(new_x, new_y) && grid[new_x][new_y] == i) {
                cur_dir = dir_order[j];
                if (min_bend && j != 0) {
                    std::swap(dir_order[0], dir_order[j]);
                }
                break;
            }
        }
        if (prev_dir != -1 && prev_dir != cur_dir) {
            net->path.push_back(std::make_pair(x, y));
        }
        prev_dir = cur_dir;
        x += dx[cur_dir];
        y += dy[cur_dir];
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

bool Router::routeCostBased() {
    // sort net based on distance
    std::vector<int> net_order;
    for (int i = 0; i < num_nets; i++) {
        net_order.push_back(i);
    }
    std::vector<int> distance(num_nets);
    for (int i = 0; i < num_nets; i++) {
        distance[i] = std::abs(nets[i]->source_x - nets[i]->target_x) + std::abs(nets[i]->source_y - nets[i]->target_y);
    }
    std::sort(net_order.begin(), net_order.end(), [&distance](int a, int b) {
        return distance[a] < distance[b];
    });

    grid = clean_grid;
    for (Net* net : nets) {
        net->cost_map.resize(grid_size_row, std::vector<int>(grid_size_col, 1));
    }
    bool success = false;
    int route_net_idx = 0;
    while(!checkTimeOut() && !success) {
        Net* net = nets[net_order[route_net_idx]];
        if (routeOneNetCostBased(net)) {
            route_net_idx++;
            if (route_net_idx == num_nets) {
                success = true;
                // std::cout << "Success" << std::endl;
            }
        } else {
            // update cost map on path
            for (int n = 0; n < route_net_idx; n++) {
                Net* update_net = nets[net_order[n]];
                for (int i = 0; i < update_net->path_size-1; i++) {
                    int sx = update_net->path[i].first;
                    int sy = update_net->path[i].second;
                    int tx = update_net->path[i + 1].first;
                    int ty = update_net->path[i + 1].second;
                    if (sx == tx) {
                        for (int y = std::min(sy, ty); y <= std::max(sy, ty); y++) {
                            update_net->cost_map[sx][y]++;
                        }
                    } else {
                        for (int x = std::min(sx, tx); x <= std::max(sx, tx); x++) {
                            update_net->cost_map[x][sy]++;
                        }
                    }
                }
            }
            // ramdomly shuffle net order till route_net_idx
            std::random_shuffle(net_order.begin(), net_order.begin() + route_net_idx);
            route_net_idx = 0;
            grid = clean_grid;
        }
    }
    if (success) {
        for (int i = 0; i < num_nets; i++) {
            best_nets[i]->copy(nets[i]);
        }
    }
}

bool Router::routeOneNetCostBased(Net* net) {
    // std::cout << "Route net " << net->net_id << std::endl;
     // Lee's algorithm
    std::vector<std::pair<int, int>> queue;
    queue.push_back(std::make_pair(net->source_x, net->source_y));
    std::vector<std::vector<t_grid>> grid_init = grid;
    grid[net->source_x][net->source_y] = 1;
    grid[net->target_x][net->target_y] = EMPTY;
    std::vector<std::vector<t_grid>> grid_note = grid;
    int head = 0;
    int tail = 1;
    bool found = false;
    while (head < tail) {
        // showGrid();
        int x = queue[head].first;
        int y = queue[head].second;
        if (x == net->target_x && y == net->target_y) {
            found = true;
            head++;
            continue;
            // break;
        }
        grid_note[x][y] = WIRE;
        for (int i = 0; i < 4; i++) {
            int new_x = x + dx[i];
            int new_y = y + dy[i];
            if (isOnGrid(new_x, new_y) && grid_note[new_x][new_y] == EMPTY) {
                queue.push_back(std::make_pair(new_x, new_y));
                if (grid[new_x][new_y] == EMPTY) {
                    grid[new_x][new_y] = grid[x][y] + net->cost_map[new_x][new_y];
                } else {
                    grid[new_x][new_y] = std::min(grid[new_x][new_y], grid[x][y] + net->cost_map[new_x][new_y]);
                }
                tail++;
            }
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
    for (int i = grid[x][y] - net->cost_map[x][y]; i > 0; i -= net->cost_map[x][y]) {
        for (int j = 0; j < 4; j++) {
            int new_x = x + dx[dir_order[j]];
            int new_y = y + dy[dir_order[j]];
            if (isOnGrid(new_x, new_y) && grid[new_x][new_y] == i) {
                cur_dir = dir_order[j];
                if (min_bend && j != 0) {
                    std::swap(dir_order[0], dir_order[j]);
                }
                break;
            }
        }
        if (prev_dir != -1 && prev_dir != cur_dir) {
            net->path.push_back(std::make_pair(x, y));
        }
        prev_dir = cur_dir;
        x += dx[cur_dir];
        y += dy[cur_dir];
    }
    if (net->path.back() != std::make_pair(net->source_x, net->source_y)) {
        net->path.push_back(std::make_pair(net->source_x, net->source_y));
    }
    net->path_size = net->path.size();
    grid = grid_init;
    net->length = -1;
    // std::cout << "Path size: " << int(net->path.size()) << std::endl;
    for (int i = 0; i < net->path.size()-1; i++) {
        int sx = net->path[i].first;
        int sy = net->path[i].second;
        int tx = net->path[i + 1].first;
        int ty = net->path[i + 1].second;
        if (sx == tx) {
            for (int y = std::min(sy, ty); y <= std::max(sy, ty); y++) {
                grid[sx][y] = WIRE;
            }
            net->length += std::abs(sy - ty);
        } else {
            for (int x = std::min(sx, tx); x <= std::max(sx, tx); x++) {
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
        Net* net = best_nets[i];
        fout << net->net_name << " " << net->length << std::endl;
        fout << "begin" << std::endl;
        for (int j = net->path.size() - 1; j > 0; j--) {
            fout << net->path[j].second << " " << net->path[j].first << " " << net->path[j-1].second << " " << net->path[j-1].first << std::endl;
        }
        fout << "end" << std::endl;
    }
}

void Router::showGrid() {
    for (int j = 0; j < grid_size_col; j++) {
        std::cout << "===";
    }
    std::cout << std::endl;
    for (int i = grid_size_row - 1; i >= 0; i--) {
        for (int j = 0; j < grid_size_col; j++) {
            if (grid[i][j] == EMPTY) {
                std::cout << "   ";
            } else if (grid[i][j] == BLOCK) {
                std::cout << "  #";
            } else if (grid[i][j] == WIRE) {
                std::cout << "  *";
            } else {
                std::cout << std::setw(3) << grid[i][j];
            }
        }
        std::cout << std::endl;
    }
    for (int j = 0; j < grid_size_col; j++) {
        std::cout << "===";
    }
    std::cout << std::endl;
}

bool Router::isOnGrid(int x, int y) {
    return x >= 0 && x < grid_size_row && y >= 0 && y < grid_size_col;
}

bool Router::checkTimeOut() {
    return checkTimeOut(time_limit);
}

bool Router::checkTimeOut(int local_time_limit) {
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - start_time).count() > local_time_limit;
}