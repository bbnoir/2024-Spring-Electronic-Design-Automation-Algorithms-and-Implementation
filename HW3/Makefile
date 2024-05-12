CXXFLAGS += -std=c++11 -O3

all: lab3

lab3: main.cpp router.o
	$(CXX) $(CXXFLAGS) -o $@ $^

router.o: router.cpp router.hpp
	$(CXX) $(CXXFLAGS) -c -o $@ $<

clean:
	rm -f lab3
	rm -f *.o
