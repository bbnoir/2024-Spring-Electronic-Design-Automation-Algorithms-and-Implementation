#pragma once
#include "include.hpp"
#include "cell.hpp"

class BucketList {
public:
    BucketList(int maxPin) : maxPin(maxPin), maxGain(-maxPin) {
        for (int i = -maxPin; i <= maxPin; i++)
            bucketMap[i] = new Cell(-1);
    }

    void insert(Cell* cell) {
        int gain = cell->gain;
        if (gain > maxGain)
            maxGain = gain;
        Cell* firstCell = bucketMap[gain]->next;
        cell->prev = bucketMap[gain];
        cell->next = firstCell;
        bucketMap[gain]->next = cell;
        if (firstCell)
            firstCell->prev = cell;
    }

    void erase(Cell* cell) {
        Cell* prevCell = cell->prev;
        Cell* nextCell = cell->next;
        prevCell->next = nextCell;
        if (nextCell)
            nextCell->prev = prevCell;
        cell->prev = nullptr;
        cell->next = nullptr;
        while (maxGain != -maxPin && bucketMap[maxGain]->next == nullptr) maxGain--;
    }

    void reset() {
        for (int i = -maxPin; i <= maxPin; i++)
            bucketMap[i]->next = nullptr;
    }

    Cell* getMaxGainCell() {
        return bucketMap[maxGain]->next;
    }

public:
    int maxPin, maxGain;
    unordered_map<int, Cell*> bucketMap;
};