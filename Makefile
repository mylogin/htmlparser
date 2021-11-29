CXX := g++
CXX_FLAGS := -Wall -std=c++11 -g
EXECUTABLE := example

.PHONY: all clean

all: $(EXECUTABLE)

$(EXECUTABLE): html.o example.o
	$(CXX) $(CXX_FLAGS) -o $@ $^

html.o: html.cpp html.hpp
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

example.o: test/example.cpp html.hpp
	$(CXX) $(CXX_FLAGS) -I. -c -o $@ $<

clean:
	-rm -f *.o $(EXECUTABLE)