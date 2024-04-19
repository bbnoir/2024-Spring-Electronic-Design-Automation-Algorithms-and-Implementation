#pragma once
#include "include.hpp"

class Cell {
public:
    Cell() : gain(0), partition(0), locked(false), prev(nullptr), next(nullptr) {}
    ~Cell() {}

    void lock() { locked = true; }
    void unlock() { locked = false; }
    void move() { partition = !partition; }

public:
    int gain;
    bool partition;
    bool locked;
    vector<int> netSet;
    Cell* prev;
    Cell* next;
};