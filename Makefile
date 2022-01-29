CXX := g++
CXX_FLAGS := -Wall -std=c++11
EXAMPLE := example
SELECTORS := selectors

.PHONY: all clean

all: $(EXAMPLE) $(SELECTORS)

$(EXAMPLE): html.o example.o
	$(CXX) $(CXX_FLAGS) -o $@ $^

$(SELECTORS): html.o selectors.o
	$(CXX) $(CXX_FLAGS) -o $@ $^

html.o: html.cpp html.hpp
	$(CXX) $(CXX_FLAGS) -c -o $@ $<

example.o: test/example.cpp html.hpp
	$(CXX) $(CXX_FLAGS) -I. -c -o $@ $<

selectors.o: test/selectors.cpp html.hpp
	$(CXX) $(CXX_FLAGS) -I. -c -o $@ $<

clean:
	-rm -f *.o $(EXAMPLE) $(SELECTORS)