#include "include.hpp"
#include "router.hpp"

int main(int argv, char* argc[]) {
    Router router(argc[1]);
    router.route();
    router.writeResults(argc[2]);
}