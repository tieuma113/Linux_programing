CXX := g++
CXXFLAGS := -Wall -std=c++17 -g -pthread

BUILDDIR := build

$(BUILDDIR)/%: %.cpp $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BUILDDIR):
	mkdir -p $@

clean:
	rm -rf $(BUILDDIR)