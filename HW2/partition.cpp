#include "partition.hpp"

Partition::Partition(string input_file) {
    ifstream fin(input_file);
    fin >> numNets >> numCells;
    for (int i = 0; i < numNets; i++)
        netArray.emplace_back(new Net());
    for (int i = 0; i <= numCells; i++)
        cellArray.emplace_back(new Cell());
    string tmpString;
    getline(fin, tmpString);
    for (int i = 0; i < numNets; i++) {
        getline(fin, tmpString);
        stringstream ss(tmpString);
        int tmp;
        while (ss >> tmp) {
            netArray[i]->cellSet.insert(tmp);
            cellArray[tmp]->netSet.insert(i);
        }
    }
    minPartitionSize = numCells * 0.45, maxPartitionSize = numCells - minPartitionSize;
    maxPin = 0;
    for (int i = 1; i <= numCells; i++) {
        maxPin = max(maxPin, (int)cellArray[i]->netSet.size());
    }
    bucketList[0] = new BucketList(maxPin);
    bucketList[1] = new BucketList(maxPin);
    fin.close();
}

void Partition::partitioning() {
    start = chrono::system_clock::now();
    int numSeed = 6;
    vector<int> seedList;
    srand(42);
    for (int i = 0; i < numSeed; i++)
        seedList.push_back(rand());
    int bestCutSize = -1;
    bestPartition.resize(numCells);
    for (int seed : seedList) {
        partitioningWithSeed(seed);
        if (!checkLegal()) continue;
        int cutSize = getCutSize();
        cout << "Seed: " << seed << ", Cut Size: " << cutSize << endl;
        if (bestCutSize == -1 || cutSize < bestCutSize) {
            bestCutSize = cutSize;
            for (int i = 0; i < numCells; i++)
                bestPartition[i] = cellArray[i+1]->partition;
        }
        if (chrono::duration_cast<chrono::seconds>(chrono::system_clock::now() - start).count() > timeLimit)
            break;
    }
}

void Partition::partitioningWithSeed(int seed) {
    randomInitPartition(seed);
    initNetDistribution();

    vector<Cell*> moveCellList;
    int maxId = 0, maxCumuGain = 0, cumuGain = 0;
    int iter = 0;
    int stopMove = numCells * 0.95;
    const int maxIter = 30;
    do {
        resetLock();
        initGain();
        moveCellList.clear();

        cumuGain = maxCumuGain = 0;
        maxId = -1;
        int t = 0;
        for (; t < stopMove; t++) {
            Cell* maxGainCell = getMaxGainCell();
            if (maxGainCell == nullptr) break;
            updateGain(maxGainCell);

            cumuGain += maxGainCell->gain;
            if (cumuGain > maxCumuGain) {
                maxCumuGain = cumuGain;
                maxId = t;
            }
            moveCellList.push_back(maxGainCell);
        }
        for (int i = t-1; i > maxId; i--) {
            moveBack(moveCellList[i]);
        }
        iter++;
        if (chrono::duration_cast<chrono::seconds>(chrono::system_clock::now() - start).count() > timeLimit)
            break;
    } while (maxCumuGain > 0 && iter < maxIter);
}

void Partition::randomInitPartition(int seed) {
    srand(seed);
    partitionSize[0] = partitionSize[1] = 0;
    for (int i = 1; i <= numCells; i++) {
        cellArray[i]->partition = rand() % 2;
        partitionSize[cellArray[i]->partition]++;
    }
}

void Partition::initNetDistribution() {
    for (int i = 0; i < numNets; i++) {
        netArray[i]->partitionSize[0] = netArray[i]->partitionSize[1] = 0;
        for (auto j : netArray[i]->cellSet) {
            netArray[i]->partitionSize[cellArray[j]->partition]++;
        }
    }
}

void Partition::resetLock() {
    for (int i = 1; i <= numCells; i++)
        cellArray[i]->unlock();
}

void Partition::initGain() {
    bucketList[0]->reset();
    bucketList[1]->reset();
    for (int i = 1; i <= numCells; i++) {
        cellArray[i]->gain = 0;
        int from = cellArray[i]->partition, to = 1 - from;
        for (auto j : cellArray[i]->netSet) {
            if (netArray[j]->partitionSize[from] == 1)
                cellArray[i]->gain++;
            if (netArray[j]->partitionSize[to] == 0)
                cellArray[i]->gain--;
        }
        bucketList[from]->insert(cellArray[i]);
    }
}

Cell* Partition::getMaxGainCell() {
    if (partitionSize[0] <= minPartitionSize) {
        return bucketList[1]->getMaxGainCell();
    } else if (partitionSize[1] <= minPartitionSize) {
        return bucketList[0]->getMaxGainCell();
    } else if (bucketList[0]->maxGain >= bucketList[1]->maxGain) {
        return bucketList[0]->getMaxGainCell();
    } else {
        return bucketList[1]->getMaxGainCell();
    }
}

void Partition::moveBack(Cell* cell) {
    bool from = cell->partition;
    bool to = 1 - from;
    cell->move();
    partitionSize[from]--;
    partitionSize[to]++;
    for (auto net_id : cell->netSet) {
        Net* net = netArray[net_id];
        net->partitionSize[from]--;
        net->partitionSize[to]++;
    }
}

void Partition::updateGain(Cell* cell) {
    bool from = cell->partition;
    bool to = 1 - from;
    bucketList[from]->erase(cell);
    partitionSize[from]--;
    partitionSize[to]++;
    cell->lock();
    cell->move();
    for (auto net_id : cell->netSet) {
        // check critical nets before moving
        Net* net = netArray[net_id];
        if (net->partitionSize[to] == 0) {
            for (int j : net->cellSet) {
                Cell* cur_cell = cellArray[j];
                if (!cur_cell->locked) {
                    bucketList[cur_cell->partition]->erase(cur_cell);
                    cur_cell->gain++;
                    bucketList[cur_cell->partition]->insert(cur_cell);
                }
            }
        } else if (net->partitionSize[to] == 1) {
            for (int j : net->cellSet) {
                Cell* cur_cell = cellArray[j];
                if (cur_cell->partition == to && !cur_cell->locked) {
                    bucketList[to]->erase(cur_cell);
                    cur_cell->gain--;
                    bucketList[to]->insert(cur_cell);
                    break;
                }
            }
        }
        // update net distribution
        net->partitionSize[from]--;
        net->partitionSize[to]++;
        // check critical nets after moving
        if (net->partitionSize[from] == 0) {
            for (int j : net->cellSet) {
                Cell* cur_cell = cellArray[j];
                if (!cur_cell->locked) {
                    bucketList[cur_cell->partition]->erase(cur_cell);
                    cur_cell->gain--;
                    bucketList[cur_cell->partition]->insert(cur_cell);
                }
            }
        } else if (net->partitionSize[from] == 1) {
            for (int j : net->cellSet) {
                Cell* cur_cell = cellArray[j];
                if (cur_cell->partition == from && !cur_cell->locked) {
                    bucketList[from]->erase(cur_cell);
                    cur_cell->gain++;
                    bucketList[from]->insert(cur_cell);
                    break;
                }
            }
        }
    }
}

void Partition::writeResult(string output_file) {
    ofstream fout(output_file);
    for (int i = 0; i < numCells; i++) {
        fout << bestPartition[i] << endl;
    }
    fout.close();
}

int Partition::getCutSize() {
    int cutSize = 0;
    for (int i = 0; i < numNets; i++) {
        if (netArray[i]->partitionSize[0] > 0 && netArray[i]->partitionSize[1] > 0)
            cutSize++;
    }
    return cutSize;
}

bool Partition::checkLegal() {
    return partitionSize[0] >= minPartitionSize && partitionSize[0] <= maxPartitionSize;
}