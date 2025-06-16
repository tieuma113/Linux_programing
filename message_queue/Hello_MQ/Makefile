CXX := g++
CXXFLAGS := -Wall -std=c++17 -g
BUILDDIR:= build
TARGETS += main.cpp

all: $(TARGETS)

SYS_V: main.cpp $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -D$@ $< -o $(BUILDDIR)/$@

POSIX: main.cpp $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -D$@ $< -o $(BUILDDIR)/$@

$(BUILDDIR):
	mkdir -p build

clean:
	rm -rf build