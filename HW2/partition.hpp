#pragma once
#include "include.hpp"
#include "cell.hpp"
#include "net.hpp"
#include "bucketlist.hpp"

class Partition {
public:
    Partition(string input_file);
    void partitioning();
    void partitioningWithSeed(int seed);
    void writeResult(string output_file);

    void randomInitPartition(int seed);
    void initNetDistribution();
    void resetLock();
    void initGain();
    Cell* getMaxGainCell();
    void updateGain(Cell* cell);
    void moveBack(Cell* cell);
    int getCutSize();
    bool checkLegal();

public:
    int numNets, numCells;
    int minGroupSize, maxGroupSize;
    int partitionSize[2] = {0, 0};
    int minPartitionSize, maxPartitionSize;
    int maxPin;
    vector<Net*> netArray;
    vector<Cell*> cellArray;
    BucketList* bucketList[2];
    vector<bool> bestPartition;
    // timer
    chrono::time_point<chrono::system_clock> start;
    int timeLimit = 20;
};