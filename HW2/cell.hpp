#pragma once
#include "include.hpp"

class Cell {
public:
    Cell(int cell_id) {
        id = cell_id;
        gain = 0;
        partition = 0;
        locked = false;
        prev = nullptr;
        next = nullptr;
    }
    ~Cell() {}

    void lock() { locked = true; }
    void unlock() { locked = false; }
    void move() { partition = !partition; }

public:
    int id;
    int gain;
    bool partition;
    bool locked;
    unordered_set<int> netSet;
    Cell* prev;
    Cell* next;
};