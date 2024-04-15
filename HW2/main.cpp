#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_set>

using namespace std;

int numNets, numNodes;
int minGroupSize, maxGroupSize;
vector<unordered_set<int>> netArray, nodeArray;
int maxPin = 0;

class BucketList {
public:
    const int maxPin;
    vector<unordered_set<int>> bucketList;

    BucketList(int maxPin) : maxPin(maxPin) {
        bucketList.resize(2*maxPin+1);
    }

    void insert(int gain, int node) {
        bucketList[gain+maxPin].insert(node);
    }

    void erase(int gain, int node) {
        bucketList[gain+maxPin].erase(node);
    }

    int getMaxGain() {
        for (int i = 2*maxPin; i >= 0; i--) {
            if (!bucketList[i].empty()) {
                return i-maxPin;
            }
        }
        return 0;
    }

    void print() {
        for (int i = 0; i < 2*maxPin+1; i++) {
            cout << i-maxPin << ": ";
            for (auto j : bucketList[i]) {
                cout << j << ' ';
            }
            cout << endl;
        }
    }
};

int getMaxGainNode(BucketList& bucketListA, BucketList& bucketListB, const int groupCountA, const int groupCountB, int& fromGroup) {
    int maxGainA = bucketListA.getMaxGain(), maxGainB = bucketListB.getMaxGain();
    if (groupCountA <= minGroupSize || groupCountB >= maxGroupSize) {
        if (bucketListB.bucketList[maxGainB+maxPin].empty()) 
            return -1;
        fromGroup = 1;
        return *(bucketListB.bucketList[maxGainB+maxPin].begin());
    } else if (groupCountB <= minGroupSize || groupCountA >= maxGroupSize) {
        if (bucketListA.bucketList[maxGainA+maxPin].empty()) 
            return -1;
        fromGroup = 0;
        return *(bucketListA.bucketList[maxGainA+maxPin].begin());
    } else {
        if (maxGainA > maxGainB) {
            if (bucketListA.bucketList[maxGainA+maxPin].empty()) 
                return -1;
            fromGroup = 0;
            return *(bucketListA.bucketList[maxGainA+maxPin].begin());
        } else {
            if (bucketListB.bucketList[maxGainB+maxPin].empty()) 
                return -1;
            fromGroup = 1;
            return *(bucketListB.bucketList[maxGainB+maxPin].begin());
        }
    }
    return -1;
}

int main(int argv, char* argc[]) {

    // Parse input file
    ifstream fin(argc[1]);
    fin >> numNets >> numNodes;
    netArray.resize(numNets);
    nodeArray.resize(numNodes+1);
    for (int i = 0; i < numNets; i++) {
        string tmpString;
        getline(fin, tmpString);
        stringstream ss(tmpString);
        int tmp;
        while (ss >> tmp) {
            netArray[i].insert(tmp);
            nodeArray[tmp].insert(i);
        }
    }
    fin.close();

    minGroupSize = numNodes * 0.45, maxGroupSize = numNodes * 0.55;

    // Random initialization
    vector<int> group(numNodes, 0);
    int groupCount[2] = {0, 0};
    srand(42);
    for (int i = 1; i <= numNodes; i++) {
        group[i] = rand() % 2;
        groupCount[group[i]]++;
        maxPin = max(maxPin, (int)nodeArray[i].size());
    }

    // FM loop
    vector<int> bestGroup = group;
    int bestGroupCount[2] = {groupCount[0], groupCount[1]};
    while (1) {
        // Construct gain list
        vector<int> gainList(numNodes+1, 0);
        vector<vector<int>> netDistribution(numNets, vector<int>(2, 0));
        for (int i = 0; i < numNets; i++)
            for (auto j : netArray[i])
                netDistribution[i][group[j]]++;
        for (int i = 1; i <= numNodes; i++) {
            for (auto j : nodeArray[i]) {
                if (netDistribution[j][group[i]] == 1)
                    gainList[i]++;
                if (netDistribution[j][1-group[i]] == 0)
                    gainList[i]--;
            }
        }
        BucketList bucketList[2] = {BucketList(maxPin), BucketList(maxPin)};
        for (int i = 1; i <= numNodes; i++) {
            bucketList[group[i]].insert(gainList[i], i);
        }

        int cumuGain = 0;
        int cumuGainFlag = 0;
        vector<int> lockList(numNodes+1, 0);

        for (int t = 0; t < numNodes; t++) {
            // Move nodes and update gain list
            int fromGroup, toGroup;
            int maxGainNode = getMaxGainNode(bucketList[0], bucketList[1], groupCount[0], groupCount[1], fromGroup);
            if (maxGainNode == -1)
                break;
            toGroup = 1 - fromGroup;
            bucketList[fromGroup].erase(gainList[maxGainNode], maxGainNode);
            lockList[maxGainNode] = 1;
            group[maxGainNode] = 1 - group[maxGainNode];
            groupCount[fromGroup]--;
            groupCount[toGroup]++;
            for (auto i : nodeArray[maxGainNode]) {
                // check critical nets before moving
                if (netDistribution[i][toGroup] == 0) {
                    for (auto j : netArray[i]) {
                        if (lockList[j] == 0) {
                            bucketList[group[j]].erase(gainList[j], j);
                            gainList[j]++;
                            bucketList[group[j]].insert(gainList[j], j);
                        }
                    }
                } else if (netDistribution[i][toGroup] == 1) {
                    for (auto j : netArray[i]) {
                        if (group[j] == toGroup && lockList[j] == 0) {
                            bucketList[group[j]].erase(gainList[j], j);
                            gainList[j]--;
                            bucketList[group[j]].insert(gainList[j], j);
                        }
                    }
                }
                // update net distribution
                netDistribution[i][fromGroup]--;
                netDistribution[i][toGroup]++;
                // check critical nets after moving
                if (netDistribution[i][fromGroup] == 0) {
                    for (auto j : netArray[i]) {
                        if (lockList[j] == 0) {
                            bucketList[group[j]].erase(gainList[j], j);
                            gainList[j]--;
                            bucketList[group[j]].insert(gainList[j], j);
                        }
                    }
                } else if (netDistribution[i][fromGroup] == 1) {
                    for (auto j : netArray[i]) {
                        if (group[j] == fromGroup && lockList[j] == 0) {
                            bucketList[group[j]].erase(gainList[j], j);
                            gainList[j]++;
                            bucketList[group[j]].insert(gainList[j], j);
                        }
                    }
                }
            }

            cumuGain += gainList[maxGainNode];
            if (cumuGain > 0) {
                cumuGainFlag = 1;
                bestGroup = group;
                bestGroupCount[0] = groupCount[0];
                bestGroupCount[1] = groupCount[1];
            }
            // bucketList[0].print();
            // cout << endl;
            // bucketList[1].print();
            // cout << endl;
            // cout << "Group Count: " << groupCount[0] << ' ' << groupCount[1] << endl;
            // cout << endl;
        }

        // Check if all gains are non-positive
        if (cumuGainFlag == 0) {
            break;
        }
        group = bestGroup;
        groupCount[0] = bestGroupCount[0];
        groupCount[1] = bestGroupCount[1];
    }

    // Write result
    ofstream fout("output.txt");
    for (auto i : bestGroup) {
        fout << i << '\n';
    }
    fout.close();

}