CXX := g++
CXXFLAGS := -Wall -std=c++17 -g
BUILDDIR := build
TARGETS += main

all: $(TARGETS)

main: main.cpp $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $< -o $(BUILDDIR)/$@

$(BUILDDIR): 
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)