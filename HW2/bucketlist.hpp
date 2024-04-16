#pragma once
#include "include.hpp"

class Node {
public:
    Node(int cell_id) : cell_id(cell_id), prev(nullptr), next(nullptr) {}
    ~Node() {}

public:
    int cell_id;
    Node* prev;
    Node* next;
};

class BucketList {
public:
    BucketList(int maxPin) : maxPin(maxPin), maxGain(-maxPin) {
        for (int i = -maxPin; i <= maxPin; i++)
            bucketMap[i] = new Node(-1);
    }

    void insert(int gain, Node* node) {
        assert(gain <= maxPin && gain >= -maxPin);
        if (gain > maxGain)
            maxGain = gain;
        Node* firstNode = bucketMap[gain]->next;
        node->prev = bucketMap[gain];
        node->next = firstNode;
        bucketMap[gain]->next = node;
        if (firstNode)
            firstNode->prev = node;
    }

    void erase(Node* node) {
        Node* prevNode = node->prev;
        Node* nextNode = node->next;
        prevNode->next = nextNode;
        if (nextNode)
            nextNode->prev = prevNode;
        node->prev = nullptr;
        node->next = nullptr;
        while (maxGain != -maxPin && bucketMap[maxGain]->next == nullptr) maxGain--;
    }

    void reset() {
        for (int i = -maxPin; i <= maxPin; i++)
            bucketMap[i]->next = nullptr;
    }

    Node* getMaxGainNode() {
        return bucketMap[maxGain]->next;
    }

public:
    int maxPin, maxGain;
    unordered_map<int, Node*> bucketMap;
};