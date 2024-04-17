#include "partition.hpp"

Partition::Partition(string input_file) {
    ifstream fin(input_file);
    fin >> numNets >> numCells;
    netArray.resize(numNets);
    cellArray.resize(numCells+1);
    nodeArray.resize(numCells+1);
    for (int i = 0; i < numNets; i++) {
        netArray[i] = new Net();
    }
    for (int i = 1; i <= numCells; i++) {
        cellArray[i] = new Cell(i);
        nodeArray[i] = new Node(i);
    }
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
    randomInitPartition();
    initNetDistribution();

    vector<int> moveCellList;
    int maxId = 0, maxCumuGain = 0, cumuGain = 0;
    int iter = 0;
    const int stopMove = numCells * 0.9;
    do {
        resetLock();
        initGain();
        moveCellList.clear();

        cumuGain = maxCumuGain = maxId = 0;
        for (int t = 0; t < stopMove; t++) {
            // Move nodes and update gain list
            Node* maxGainNode = getMaxGainNode();
            if (maxGainNode == nullptr) break;
            updateGain(maxGainNode);

            Cell* maxGainCell = cellArray[maxGainNode->cell_id];
            cumuGain += maxGainCell->gain;
            if (cumuGain > maxCumuGain) {
                maxCumuGain = cumuGain;
                maxId = t;
            }
            moveCellList.push_back(maxGainCell->id);
        }
        if (maxCumuGain <= 0) break;
        for (int t = stopMove-1; t > maxId; t--) {
            moveBack(moveCellList[t]);
        }
        iter++;
    } while (maxCumuGain > 0);
}

void Partition::randomInitPartition() {
    srand(42);
    for (int i = 1; i <= numCells; i++) {
        cellArray[i]->partition = rand() % 2;
        partitionSize[cellArray[i]->partition]++;
    }
}

void Partition::initNetDistribution() {
    for (int i = 0; i < numNets; i++) {
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
        bucketList[from]->insert(cellArray[i]->gain, nodeArray[i]);
    }
}

Node* Partition::getMaxGainNode() {
    if (partitionSize[0] == minPartitionSize) {
        return bucketList[1]->getMaxGainNode();
    } else if (partitionSize[1] == minPartitionSize) {
        return bucketList[0]->getMaxGainNode();
    } else if (bucketList[0]->maxGain > bucketList[1]->maxGain) {
        return bucketList[0]->getMaxGainNode();
    } else if (bucketList[0]->maxGain < bucketList[1]->maxGain) {
        return bucketList[1]->getMaxGainNode();
    } else if (partitionSize[0] < partitionSize[1]) {
        return bucketList[0]->getMaxGainNode();
    } else if (partitionSize[0] > partitionSize[1]) {
        return bucketList[1]->getMaxGainNode();
    } else {
        return bucketList[0]->getMaxGainNode(); // default
    }
}

void Partition::moveBack(int cell_id) {
    bool from = cellArray[cell_id]->partition;
    bool to = 1 - from;
    cellArray[cell_id]->move();
    partitionSize[from]--;
    partitionSize[to]++;
    for (auto net_id : cellArray[cell_id]->netSet) {
        Net* net = netArray[net_id];
        net->partitionSize[from]--;
        net->partitionSize[to]++;
    }
}

void Partition::updateGain(Node* node) {
    bool from = cellArray[node->cell_id]->partition;
    bool to = 1 - from;
    bucketList[from]->erase(node);
    partitionSize[from]--;
    partitionSize[to]++;
    Cell* updateCell = cellArray[node->cell_id];
    updateCell->lock();
    updateCell->move();
    for (auto net_id : updateCell->netSet) {
        // check critical nets before moving
        Net* net = netArray[net_id];
        if (net->partitionSize[to] == 0) {
            for (int j : net->cellSet) {
                Cell* cell = cellArray[j];
                if (!cell->locked) {
                    bucketList[cell->partition]->erase(nodeArray[j]);
                    cell->gain++;
                    bucketList[cell->partition]->insert(cell->gain, nodeArray[j]);
                }
            }
        } else if (net->partitionSize[to] == 1) {
            for (int j : net->cellSet) {
                Cell* cell = cellArray[j];
                if (cell->partition == to) {
                    if (!cell->locked) {
                        bucketList[to]->erase(nodeArray[j]);
                        cell->gain--;
                        bucketList[to]->insert(cell->gain, nodeArray[j]);
                    }
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
                Cell* cell = cellArray[j];
                if (!cell->locked) {
                    bucketList[cell->partition]->erase(nodeArray[j]);
                    cell->gain--;
                    bucketList[cell->partition]->insert(cell->gain, nodeArray[j]);
                }
            }
        } else if (net->partitionSize[from] == 1) {
            for (int j : net->cellSet) {
                Cell* cell = cellArray[j];
                if (cell->partition == from) {
                    if (!cell->locked) {
                        bucketList[from]->erase(nodeArray[j]);
                        cell->gain++;
                        bucketList[from]->insert(cell->gain, nodeArray[j]);
                    }
                    break;
                }
            }
        }
    }
}

void Partition::writeResult(string output_file) {
    ofstream fout(output_file);
    for (int i = 1; i <= numCells; i++) {
        fout << cellArray[i]->partition << '\n';
    }
    fout.close();
}