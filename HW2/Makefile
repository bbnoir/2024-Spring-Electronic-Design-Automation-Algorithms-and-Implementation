CXXFLAGS += -std=c++11 -O3

all: Lab2

Lab2: main.cpp partition.o
	$(CXX) $(CXXFLAGS) -o $@ $^

partition.o: partition.cpp partition.hpp cell.hpp net.hpp bucketlist.hpp include.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f Lab2