#pragma once
#include "include.hpp"
#include "cell.hpp"

class Net {
public:
    Net() {
        partitionSize[0] = 0;
        partitionSize[1] = 0;
    }
    ~Net() {}

public:
    int partitionSize[2];
    vector<int> cellSet;
};