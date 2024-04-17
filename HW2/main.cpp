#include "include.hpp"
#include "partition.hpp"

int main(int argv, char* argc[]) {
    Partition partition(argc[1]);
    partition.partitioning();
    partition.writeResult("output.txt");
}