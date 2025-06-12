CXX := g++
CXXFLAGS := -Wall -g -pthread -std=c++17
BUILDDIR := build

$(BUILDDIR)/%: %.cpp $(BUILDDIR)
	$(CXX) $(CXXFLAGS) $< -o $@

$(BUILDDIR):
	mkdir -p $(BUILDDIR)

clean:
	rm -rf $(BUILDDIR)

