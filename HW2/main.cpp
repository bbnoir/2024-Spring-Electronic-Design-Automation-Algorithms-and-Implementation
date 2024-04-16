#include "include.hpp"
#include "partition.hpp"

int main(int argv, char* argc[]) {

    // Parse input file
    ifstream fin(argc[1]);
    Partition partition(fin);
    fin.close();

    // Partitioning
    partition.partitioning();

    // Write result
    ofstream fout("output.txt");
    partition.writeResult(fout);
    fout.close();

}