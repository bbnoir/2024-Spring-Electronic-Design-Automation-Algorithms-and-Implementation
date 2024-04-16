#pragma once
#include "include.hpp"
#include "cell.hpp"
#include "net.hpp"
#include "bucketlist.hpp"

class Partition {
public:
    Partition(ifstream& fin);
    void partitioning();
    void writeResult(ofstream& fout);

    void randomInitPartition();
    void initNetDistribution();
    void resetLock();
    void initGain();
    Node* getMaxGainNode();
    void updateGain(Node* node);
    void moveBack(int cell_id);

public:
    int numNets, numCells;
    int minGroupSize, maxGroupSize;
    int partitionSize[2] = {0, 0};
    int minPartitionSize, maxPartitionSize;
    int maxPin;
    vector<Net*> netArray;
    vector<Cell*> cellArray;
    vector<Node*> nodeArray;
    BucketList* bucketList[2];

};